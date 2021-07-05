#ifdef SEKISAN

////////////////////////////////////////////////////////////////
//                  積算関係
////////////////////////////////////////////////////////////////

#define SEKISAN_LEVERAGE    10  // 積算温度を0.1℃単位で整数計算するため、10倍する。
                                // unsigned intなので最大積算温度は6502℃日。足りない場合は精度は少し落ちるが5倍値等にする。
#define BACKUP_DATA_ADDRESS  0  // 積算温度、積算開始日等のバックアップデータを保存するArduino EEPROMのアドレス

#define REF_TEMP    (16.0 * SEKISAN_LEVERAGE) // 積算温度の基準温度。16℃は熱帯作物用


////////////////////////////////////////////////////////////////
// 　　　　　　型宣言 + 付属広域変数宣言　積算温度部
/////////////////////////////////////////////////////////////////

typedef struct {
  byte Year;    // 年
  byte Month;   // 月
  byte Day;     // 日
} YMD_t;


typedef struct { //1日の簡易データバックアップ（トラブル復旧用）
  byte dataStatus;      // 0x00は最終日データ記録有りを示す。
  YMD_t startYMD[5];    // 積算開始日（5つ）
  YMD_t lastYMD;        // 最後の積算日
  unsigned int lastDayAvgTemp;   // 最後の日平均温度（１０倍値）
  unsigned int accumTemp[5];     // 最後の積算温度　（１０倍値）（16℃基準）（5つ）
  unsigned int accumTemp0[5];    // 最後の積算温度　（１０倍値）（ 0℃基準）（5つ）
  byte at[5];                    // 積算中のフラグ（5つ）
} lastSekisanData_t;


typedef struct {        // 1日の簡易データバックアップのうちstartYMDアクセス用（lastSekisanData_tの前方一部。lcdTime()で使う。）
  byte dataStatus;      // 0x00は最終日データ記録有りを示す。
  YMD_t startYMD[5];    // 積算開始日（5つ）
} startYMDdata_t;


//YMD_t gStartYMD[5] = {{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}};  // メモリ節約のため、EEPROMの情報を直接使うことにした。


// 積算温度　10倍値のunsigned intなので、最大積算温度は6502℃日。足りない場合は精度は少し落ちるが5倍値等にする。
unsigned int gAccumTemp[5] = {0, 0, 0, 0, 0};  // 積算温度（１０倍値）
unsigned int gAccumTemp0[5] = {0, 0, 0, 0, 0}; // 積算温度（１０倍値） 0℃基準
byte gAt[5] = {0, 0, 0, 0, 0};                 // 5個の積算温度のどれを積算するか


//////////////////////////////////////////////////////
//                    温度積算処理
//////////////////////////////////////////////////////
// 1. ログ時の温度を加算
// 2. 日付が変わったら加算した温度を加算回数で割って日平均温度算出
// 3. 算出した平均温度を積算
// 4. 新しい積算値等を前日の日付でバックアップ
// 5. 加算温度、回数をリセット
//////////////////////////////////////////////////////
void dataSekisan(data_t tData, tmElements_t tm) {
//  static tmElements_t lastTm = {0, 0, 0, 0, 1, 1, 0}; // 1970/1/1 00:00:00 // 前回加算時の時刻
  static tmElements_t lastTm = tm;  // 前回加算時の日付
  static long kasanData = 0; // 10倍値なのでintだと条件によってはオーバーフローする。25x10x6x24=36,000, 30x10x6x24=43,200
  static int dataCount = 0;
  lastSekisanData_t lastSekisanData;
  byte i;

  kasanData += (tData.dt1a * SEKISAN_LEVERAGE);       // 10倍値とする。
//  Serial.println(kasanData);//OK
  dataCount++;
  if (tm.Day != lastTm.Day) {                            // 日付が変わったら日平均温度算出して積算＆バックアップ処理
    kasanData /= dataCount;                           // 日平均温度算出。単純に1日の回数（6*24）で割ると初日は正確になるが、いろいろ面倒
    EEPROM.get(BACKUP_DATA_ADDRESS, lastSekisanData); // バックアップしていた値を読み出す。
    if (lastSekisanData.dataStatus == 0x00) {         // バックアップデータ有の場合＝積算中は情報更新
      lastSekisanData.lastYMD = {lastTm.Year, lastTm.Month, lastTm.Day}; // 日付を更新（前日＝平均気温を算出した日）
      lastSekisanData.lastDayAvgTemp = kasanData;     // 日平均温度を更新（10倍値）
      for (i = 0; i < 5; i++) {                       // 5種類について積算処理＆バックアップ処理
        gAccumTemp[i] += ((effectiveTemp(kasanData, REF_TEMP) * gAt[i]));
        gAccumTemp0[i] += ((effectiveTemp((int)kasanData, 0) * gAt[i]));
        lastSekisanData.accumTemp[i] = gAccumTemp[i];
        lastSekisanData.accumTemp0[i] = gAccumTemp0[i];
        lastSekisanData.at[i] = gAt[i];
      }
      EEPROM.put(BACKUP_DATA_ADDRESS, lastSekisanData); // バックアップを EEPROMに書き込む。
    }  // バックアップデータ無しの場合はまだ積算開始していないのでなにもしない。
    kasanData = 0;
    dataCount = 0;
    lastTm = tm;
  }
}


/*
void dataSekisanHour(data_t tData, tmElements_t tm) {
//  static tmElements_t lastTm = {0, 0, 0, 0, 1, 1, 0}; // 1970/1/1 00:00:00 // 前回加算時の時刻
  static tmElements_t lastTm = tm;  // 前回加算時の日付
  static long kasanData = 0; // 10倍値なのでintだと条件によってはオーバーフローする。
  static int dataCount = 0;
  static long kasanDataHour = 0;
  static int dataCoutHour = 0;
  lastSekisanData_t lastSekisanData;
  byte i;

  kasanData += (tData.dt1a * SEKISAN_LEVERAGE);       // 10倍値とする。
//  Serial.println(kasanData);//OK
  dataCount++;
  if (tm.Hour != lastTm.Hour) {                            // 時が変わったら平均温度算出して積算処理
    kasanData /= (dataCount);                              // 平均温度算出
    kasanDataHour += kasanData;
    for (i = 0; i < 5; i++) {                       // 5種類について積算処理＆バックアップ処理
      gAccumTemp[i] += (((effectiveTemp(kasanData, REF_TEMP) / 24) * gAt[i]));
      gAccumTemp0[i] += (((effectiveTemp((int)kasanData, 0) / 24) * gAt[i]));
    }
    if (tm.Day != lastTm.Day) {                            // 日付が変わったら日平均温度算出して積算＆バックアップ処理
    EEPROM.get(BACKUP_DATA_ADDRESS, lastSekisanData); // バックアップしていた値を読み出す。
    if (lastSekisanData.dataStatus == 0x00) {         // バックアップデータ有の場合＝積算中は情報更新
      lastSekisanData.lastYMD = {lastTm.Year, lastTm.Month, lastTm.Day}; // 日付を更新（前日＝平均気温を算出した日）
      lastSekisanData.lastDayAvgTemp = kasanData / ;     // 日平均温度を更新（10倍値）
      for (i = 0; i < 5; i++) {                       // 5種類について積算処理＆バックアップ処理
        lastSekisanData.accumTemp[i] = gAccumTemp[i];
        lastSekisanData.accumTemp0[i] = gAccumTemp0[i];
        lastSekisanData.at[i] = gAt[i];
      }
      EEPROM.put(BACKUP_DATA_ADDRESS, lastSekisanData); // バックアップを EEPROMに書き込む。
    }  // バックアップデータ無しの場合はまだ積算開始していないのでなにもしない。
    kasanData = 0;
    dataCount = 0;
    lastTm = tm;
  }
}

*/

int effectiveTemp(int temp, int refTemp) {  // 有効温度算出。intなのは整数計算してメモリ節約するため。実際の温度の10倍値を使って精度確保（0.1℃単位）
  temp -= refTemp;
  if (temp < 0) temp = 0;
  return temp;
}


void sekisanStart(tmElements_t tm, byte atNo) {   // 最初の積算開始時に、開始する積算番号だけでなく全ての開始日時をリセット（1970/1/1）する。
  lastSekisanData_t lastSekisanData;

  EEPROM.get(BACKUP_DATA_ADDRESS, lastSekisanData);
  if (lastSekisanData.dataStatus != 0x00)  { // バックアップデータ無しの場合
    resetBackupData();
    EEPROM.get(BACKUP_DATA_ADDRESS, lastSekisanData);
  }
  lastSekisanData.dataStatus = 0x00;
  if (gAt[atNo] == 0) {
    gAt[atNo] = 1;
    if (lastSekisanData.startYMD[atNo].Year == 0) {  // リセットしたときの00のままなら、最初の積算開始。一旦積算終了して再開した場合は開始日は元のまま
      lastSekisanData.startYMD[atNo].Year = tm.Year;
      lastSekisanData.startYMD[atNo].Month = tm.Month;
      lastSekisanData.startYMD[atNo].Day = tm.Day;
    }
    logIcon(true);
  } else {
    gAt[atNo] = 0;
    logIcon(false);
  }
  lastSekisanData.at[atNo] = gAt[atNo];
  EEPROM.put(BACKUP_DATA_ADDRESS, lastSekisanData);
}


///////////// write backup data to EEPROM ////////////////////////
// store last day data for recovery in case of trouble
//////////////////////////////////////////////////////////////////

//void writeBackupData(lastSekisanData_t lastSekisanData) {
//  lastSekisanData.dataStatus = 0x00;  // 0x00は最終日データ記録有りを示す。
//  EEPROM.put(BACKUP_DATA_ADDRESS, lastSekisanData);
//}


bool readBackupData(tmElements_t tm) { // 電源断後の再起動時にAVR内蔵EEPROMから保存データ読み込み。
  lastSekisanData_t lastSekisanData;
  tmElements_t lastTm;
  int i, j;

  EEPROM.get(BACKUP_DATA_ADDRESS, lastSekisanData);
  if (lastSekisanData.dataStatus != 0x00)  { // バックアップデータ無しの場合
    //    resetBackupData(); // 初期化はしない方が良いか。
    return false;
  } else {   // 最終日データがある場合は、
    lastTm = {0, 0, 0, 0, lastSekisanData.lastYMD.Day, lastSekisanData.lastYMD.Month, lastSekisanData.lastYMD.Year};
    int dDays = daysDiff(lastTm, tm);     // 最終データ日から今日までの日数
//    if ((dDays < 0) || (dDays > 30)) {    // 日付が戻った場合や３０日以上空いた場合 // 起算開始日は最終日データがないので、この処理はしない。
//      resetBackupData();                  // 初期化
//      return false;
//    } else {                              // 最終日データが有効な場合
      for (i = 0; i < 5; i++) {           // 最終日の平均気温で空いた日の積算概算
        gAccumTemp[i] = lastSekisanData.accumTemp[i];
        gAccumTemp0[i] = lastSekisanData.accumTemp0[i];
        gAt[i] = lastSekisanData.at[i];
        for (j = 1; j < dDays; j++) { // 日数分不足データを推計 1日差（前日まで積算あり）の場合は積算しない
          gAccumTemp[i] += (effectiveTemp(lastSekisanData.lastDayAvgTemp, REF_TEMP) * gAt[i]);
          gAccumTemp0[i] += (effectiveTemp(lastSekisanData.lastDayAvgTemp, 0) * gAt[i]);
        }
      }
//    }
  }
  return true;
}


void resetBackupData() { // EEPROMバックアップデータを初期化＆バックアップ無とする。
  lastSekisanData_t lastSekisanData;

  lastSekisanData.dataStatus = EM_FORMAT_MARK;
  lastSekisanData.lastYMD = {0, 0, 0};
  lastSekisanData.lastDayAvgTemp = 0;
  for (byte i = 0; i < 5; i++) {
    lastSekisanData.startYMD[i] = {0, 0, 0};
    lastSekisanData.accumTemp[i] = 0;
    lastSekisanData.accumTemp0[i] = 0;
    lastSekisanData.startYMD[i] = {0, 0, 0};
    gAccumTemp[i]  = 0;  // 積算温度
    gAccumTemp0[i] = 0;  // 積算温度 0℃基準
    gAt[i]         = 0;  // 5個の積算温度のどれを積算するか
  }
  EEPROM.put(BACKUP_DATA_ADDRESS, lastSekisanData);
}


void lcdTime(tmElements_t tm, lcdTimeMode_t mode, char cursorColumn) { // LCD first line
  if ((gDispMode == MENU_NO) && (mode == DISP_TIME_MODE)) return;
  //  lastSekisanData_t lastSekisanData;
  startYMDdata_t emBackup;
  EEPROM.get(BACKUP_DATA_ADDRESS, emBackup);

  lcd.setCursor(0, 0);
  if (mode == SET_TIME_MODE) {
    lcd.print(tmYearToCalendar(tm.Year));
    lcd.print(F("/"));
  }
  if (tm.Month < 10) lcd.print(F("0"));
  lcd.print(tm.Month);
  lcd.print(F("/"));
  if (tm.Day < 10) lcd.print(F("0"));
  lcd.print(tm.Day);
  lcd.print(F(" "));
  if (tm.Hour < 10) lcd.print(F("0"));
  lcd.print(tm.Hour, DEC);
  if (mode == SET_TIME_MODE) {
    lcd.print(F(":"));
  }
  if (tm.Minute < 10) lcd.print(F("0"));
  lcd.print(tm.Minute, DEC);
  lcd.print(F(" "));
  if (mode == DISP_TIME_MODE) {
    if (emBackup.dataStatus == 0) {     // 積算情報がある場合
      if (emBackup.startYMD[gDispMode].Month < 10) lcd.print(F("0"));
      lcd.print(emBackup.startYMD[gDispMode].Month);
      lcd.print(F("/"));
      if (emBackup.startYMD[gDispMode].Day < 10) lcd.print(F("0"));
      lcd.print(emBackup.startYMD[gDispMode].Day);
    }  
  }
  if (cursorColumn >= 0) {
    lcd.setCursor(cursorColumn, 0);
    lcd.blink();
  }
}


void lcdTime(tmElements_t tm) { // LCD first line
  lcdTime(tm, DISP_TIME_MODE, -1);
}


//////////////////////////////////////////////////////
//          display data to lcd                     //
//////////////////////////////////////////////////////

void lcdData(data_t tData, byte atNo) { // LCD second line
  if (gDispMode == MENU_NO) return;
  logIcon(gAt[atNo] == 1);
  //  printAccumIcon(gAt[atNo] == 1);
  lcd.setCursor(0, 1);
  lcd.print(atNo + 1);
  lcd.print(F(" "));
  if (tData.dt1a != NULLDATA_MARK) {
    if (tData.dt1a < 10) {
      lcd.print(F(" "));
    }
    lcd.print(tData.dt1a, 0);
    lcd.print(F("C ")); // 異常値表示時に桁数あふれが出た場合に消すため。
  } else {
    lcd.print(F("ERR "));
  }
  lcd.setCursor(6, 1);
  lcd.print(F("A"));

  printAT(gAccumTemp0[atNo]);
  lcd.print(F(" "));
  printAT(gAccumTemp[atNo]);
  if (gAt[atNo] == 1) {
    lcd.setCursor(6, 1);
    lcd.blink();
  }
}
// 0123456789abcdef
// 06/25 1200 04/20
// 5 30C A0900 0800
//以下は以前の表示
//0123456789ABCDEF
//1:01/23 A00:3456
//12.3C   A16:3456



void printAccumIcon(bool isDisp) {
  logIcon(isDisp);
}


void printAT(int AT) {
  AT /= SEKISAN_LEVERAGE; // 10倍値なので元に戻す。
  if (AT < 1000) lcd.print(F("0"));
  if (AT <  100) lcd.print(F("0"));
  if (AT <   10) lcd.print(F("0"));
  lcd.print(AT);
}

/*
  void lcdDate(startYMD_t startYMD, bool dispSlash) {
  if (startYMD.Month < 10) {
    lcd.print(F("0"));
  }
  lcd.print(startYMD.Month);
  if (dispSlash) {
    lcd.print(F("/"));
  }
  if (startYMD.Day < 10) {
    lcd.print(F("0"));
  }
  lcd.print(startYMD.Day);
  }

*/


////////////////////////////////////////////////////
//   write xEEPROM log data to SD with date       //
////////////////////////////////////////////////////
void writeLog2SD(tmElements_t tm, data_t tData, intervalUnit_t tLogIntervalUnit) {

  logfile.print(tmYearToCalendar(tm.Year), DEC);
  logfile.print(F("/"));
  logfile.print(tm.Month, DEC);
  logfile.print(F("/"));
  logfile.print(tm.Day, DEC);
  logfile.print(F(","));
  logfile.print(tm.Hour, DEC);
  logfile.print(F(":"));
  logfile.print(tm.Minute, DEC);
  logfile.print(F(","));
  if (tData.dt1a != NULLDATA_MARK) {
    logfile.print(tData.dt1a, 1);
  }
#ifdef DUAL_SENSORS
  logfile.print(F(","));
  if (tData.dt2a != NULLDATA_MARK) {
    logfile.print(tData.dt2a, 1);
  }
#endif
  logfile.println();
}
#endif
