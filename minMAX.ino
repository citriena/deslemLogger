#ifdef MIN_MAX
////////////////////////////////////////////////////////////////
// 電池長期間駆動Arduinoロガー
// deslemLogger (deep sleep EEPROM logger)用補助スケッチ
// 最低最高気温を表示する場合用のデータ処理、LCD表示処理等
//
////////////////////////////////////////////////////////////////
//                  最低最高関係定義
////////////////////////////////////////////////////////////////
#ifdef DUAL_SENSORS
#define SENSOR_NO 2
#else
#define SENSOR_NO 1
#endif

////////////////////////////////////////////////////////////////
// 　　　　　　型宣言 + 付属広域変数宣言　積算温度部
/////////////////////////////////////////////////////////////////

typedef struct {
  byte Month;       // 月
  byte Day;         // 日
} minMaxDate_t;

typedef struct {
  byte minHour;     // 最低気温時
  byte minMinute;   // 最低気温分
  int  minTemp;     // 最低気温 x 10
  byte maxHour;     // 最高気温時
  byte maxMinute;   // 最高気温分
  int  maxTemp;     // 最高気温 x 10
} minMaxData_t;

byte gMinMaxDataNo[MIN_MAX];                    // 当日、前日、前々日...のデータ番号、99はデータ無を示す。
minMaxDate_t gMinMaxDate[MIN_MAX];              // 最低最高気温日付データ。インデックスは上のデータ番号
minMaxData_t gMinMaxData[SENSOR_NO][MIN_MAX];   // 最低最高気温データ。2番目のインデックスは上のデータ番号

//////////////////////////////////////////////////////
//                    最低最高温度処理
//////////////////////////////////////////////////////
// 1. 最低最高が更新されたか確認し、更新されたらデータ更新
// 2. 日付が変わったらdataNoを更新
//////////////////////////////////////////////////////
void minMaxCheck(data_t tData, tmElements_t tm) {
  static byte currentDay = 99;
  int tempData[SENSOR_NO];
  float cData[SENSOR_NO]; // 設計が良くない。

  tempData[0] = (int)((tData.dt1a + 0.05) * 10);  // 最低最高温度はx10のintで処理。整数変換時に小数点以下は切り捨てられるので、四捨五入するように0.05加える。
  cData[0] = tData.dt1a;
#ifdef DUAL_SENSORS
  tempData[1] = (int)((tData.dt2a + 0.05) * 10);  // 設計が良くない。同じセンサーのデータは配列で処理すべきだった。
  cData[1] = tData.dt2a;
#endif

  if (tm.Day != currentDay) {
    shiftMinMax();
    currentDay = tm.Day;
    gMinMaxDate[gMinMaxDataNo[0]].Month = tm.Month;
    gMinMaxDate[gMinMaxDataNo[0]].Day = tm.Day;
  }
  for (byte i = 0; i < SENSOR_NO; i++) {
    if (cData[i] != NULLDATA_MARK) {
      if (tempData[i] < gMinMaxData[i][gMinMaxDataNo[0]].minTemp) {
        gMinMaxData[i][gMinMaxDataNo[0]].minTemp = tempData[i];
        gMinMaxData[i][gMinMaxDataNo[0]].minHour = tm.Hour;
        gMinMaxData[i][gMinMaxDataNo[0]].minMinute = tm.Minute;
      }
      if (tempData[i] > gMinMaxData[i][gMinMaxDataNo[0]].maxTemp) {  // 最初は最高最低ともに更新する必要があるので、elseを使わず別々に処理する。
        gMinMaxData[i][gMinMaxDataNo[0]].maxTemp = tempData[i];
        gMinMaxData[i][gMinMaxDataNo[0]].maxHour = tm.Hour;
        gMinMaxData[i][gMinMaxDataNo[0]].maxMinute = tm.Minute;
      }
    }
  }
}


void shiftMinMax() {  // ５日前のデータ領域を今日のデータ領域にし、その他は１日ずらす。
  byte newMinMaxDataNo = gMinMaxDataNo[MIN_MAX - 1];  // 最後のデータ領域を記録しておき、新しいデータ用に使う。
  byte i;

  for (i = MIN_MAX - 1; i > 0; i--) {
    gMinMaxDataNo[i] = gMinMaxDataNo[i - 1];          // データを1日ずらす。
  }
  gMinMaxDataNo[0] = newMinMaxDataNo;                 // 最後日のデータだった領域を今日のデータ領域にする。
  for (i = 0; i < SENSOR_NO; i++) {
    gMinMaxData[i][newMinMaxDataNo].minHour = 0;
    gMinMaxData[i][newMinMaxDataNo].minMinute = 0;
    gMinMaxData[i][newMinMaxDataNo].maxHour = 0;
    gMinMaxData[i][newMinMaxDataNo].maxMinute = 0;
    gMinMaxData[i][newMinMaxDataNo].minTemp =  999;    // x10なので 99.9°
    gMinMaxData[i][newMinMaxDataNo].maxTemp = -999;    // x10なので-99.9°
  }
}


void initMinMax() {
  for (byte i = 0; i < MIN_MAX; i++) {
    gMinMaxDataNo[i] = i;     // 最初は番号通り
    gMinMaxDate[i].Day = 0;   // 0はデータ無しを示す。
  }
  shiftMinMax();              // データリセット（手抜き）
}


void lcdTime(tmElements_t tm, lcdTimeMode_t mode, char cursorColumn) { // LCD first line
  if ((gDispMode > 0) && (gDispMode < CONFIG_NO)) return;     // 現在温度表示以外では実行しない
  if (mode != DATA_TIME_MODE) {  // メニュー導入画面，時計設定モード時は年も表示
    lcd.print(tmYearToCalendar(tm.Year));
    lcd.print(F("/"));
  }
  if (tm.Month < 10) lcd.print(F("0"));
  lcd.print(tm.Month);
  lcd.print(F("/"));
  if (tm.Day < 10) lcd.print(F("0"));
  lcd.print(tm.Day);
  if (mode == DATA_TIME_MODE) { // 時分は
    lcd.setCursor(0, 1);        // 表示モード時は２行目
  } else {
    lcd.print(F(" "));          // 設定モード時はスペースを空けて続ける。
  }
  if (tm.Hour < 10) lcd.print(F("0"));
  lcd.print(tm.Hour, DEC);
  lcd.print(F(":"));
  if (tm.Minute < 10) lcd.print(F("0"));
  lcd.print(tm.Minute, DEC);
  lcd.print(F("  "));
  if (cursorColumn >= 0) {
    lcd.setCursor(cursorColumn, 0);
    lcd.blink();
  }
}


//////////////////////////////////////////////////////
//          display data to lcd                     //
//////////////////////////////////////////////////////

void lcdData(data_t tData) {
  float tempData[SENSOR_NO];

  if (gDispMode >= CONFIG_NO) return; // RTC割り込みでデータ処理後に通常はデータを表示するが、設定表示、メニュー時は表示しない。
  tempData[0] = tData.dt1a;
#ifdef DUAL_SENSORS      // データ設計が良くない。
  tempData[1] = tData.dt2a;
#endif
  if (gDispMode == 0) {    // 通常の温度表示
    for (byte i = 0; i < SENSOR_NO; i++) {
      lcd.setCursor(8, i);
      lcd.print(i + 1);
      lcd.print(F(":"));
      if (tempData[i] == NULLDATA_MARK) {
        lcd.print(F(":ERR  "));
      } else {
        printTemp(tempData[i]);
        lcd.print(F("C ")); // 異常値表示時に桁数あふれが出た場合に消すため。
      }
    }
  } else {   // 最低最高温度表示
    byte sensorNo = (gDispMode - 1) % SENSOR_NO;
    byte minMaxNo = (gDispMode - 1) / SENSOR_NO;
    minMaxNo = gMinMaxDataNo[minMaxNo];
    lcd.clear();
    lcd.setCursor(0, 0);
    if (gMinMaxDate[minMaxNo].Day == 0) {         // データ無し
      gDispMode = CONFIG_NO - 1;
      lcd.print(F("NO DATA"));
      return;
    }
    if (gMinMaxDate[minMaxNo].Month < 10) lcd.print(F("0"));
    lcd.print(gMinMaxDate[minMaxNo].Month);
    lcd.print(F("/"));
    if (gMinMaxDate[minMaxNo].Day < 10) lcd.print(F("0"));
    lcd.print(gMinMaxDate[minMaxNo].Day);
    lcd.setCursor(0, 1);
    lcd.print(sensorNo + 1);
    lcd.print(F(":"));

    lcd.setCursor(6, 0);
    if (gMinMaxData[sensorNo][minMaxNo].maxHour < 10) lcd.print(F(" "));
    lcd.print(gMinMaxData[sensorNo][minMaxNo].maxHour, DEC);
    lcd.print(F(":"));
    if (gMinMaxData[sensorNo][minMaxNo].maxMinute < 10) lcd.print(F("0"));
    lcd.print(gMinMaxData[sensorNo][minMaxNo].maxMinute, DEC);
 
    lcd.setCursor(6, 1);
    if (gMinMaxData[sensorNo][minMaxNo].minHour < 10) lcd.print(F(" "));
    lcd.print(gMinMaxData[sensorNo][minMaxNo].minHour, DEC);
    lcd.print(F(":"));
    if (gMinMaxData[sensorNo][minMaxNo].minMinute < 10) lcd.print(F("0"));
    lcd.print(gMinMaxData[sensorNo][minMaxNo].minMinute, DEC);

    lcd.setCursor(12, 0);
    printTemp((float)gMinMaxData[sensorNo][minMaxNo].maxTemp / 10);
    lcd.setCursor(12, 1);
    printTemp((float)gMinMaxData[sensorNo][minMaxNo].minTemp / 10);
  }
}


// 0123456789abcdef
// 12/23   1: 15.2C
// 06:00   2: 20.3C

// 0123456789abcdef
// 12/23 13:00 12.9
// 1:    06:00 -2.0

void printTemp(float temp) {
  if (temp != NULLDATA_MARK) {
    if (temp < 0) {
      lcd.print(F("-"));
      temp = -temp;
    } else if (temp < 10) {
      lcd.print(F(" "));
    }
    lcd.print(temp, 1);
  } else {
    lcd.print(F("ERR  "));
  }
}

#endif // MIN_MAX
