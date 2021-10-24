//////////////////////////////////////////////////////
// 電池長期間駆動Arduinoロガー
// deslemLogger (deep sleep EEPROM logger)用補助スケッチ
// 使用状況等で変更する設定、複数のソースファイルで共通の設定
// Sensirion SCD40等温湿度、CO2のセンサのLCD表示，microSDへのデータ書き出しフォーマット設定
// 同じ測定項目のセンサーで共用利用できるようにソースファイルを分離
//////////////////////////////////////////////////////

#ifdef SENSOR_SCD40
#ifndef SEKISAN

//////////////////////////////////////////////////////
//          display time to lcd                     //
//////////////////////////////////////////////////////

void lcdTime(tmElements_t tm, lcdTimeMode_t mode, char cursorColumn) {
  if (mode != DATA_TIME_MODE){  // メニュー導入画面，時計設定モード時は年も表示
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

  // 0123456789abcdef
  // 04/22 25.0C 100%
  // 12:00 1000ppm


//////////////////////////////////////////////////////
//          display data to lcd                     //
//////////////////////////////////////////////////////
void lcdData(data_t tData) {
  if (gDispMode >= CONFIG_NO) return; // RTC割り込みでデータ処理後に通常はデータを表示するが、設定表示、メニュー時は表示しない。
  lcd.setCursor(6, 0);
  if (tData.dt1a == NULLDATA_MARK) {
    lcd.print(F(":ERR  "));
  } else {
    if ((tData.dt1a < 10) && (tData.dt1a >= 0)) lcd.print(F(" "));
    lcd.print(tData.dt1a, 1);
    lcd.print(F("C "));
    lcd.setCursor(12, 0);
    if (tData.dt1b < 100) lcd.print(F(" "));
    if (tData.dt1b <  10) lcd.print(F(" "));
    lcd.print(tData.dt1b, 0);
    lcd.print(F("%"));
    lcd.setCursor(6, 1); // 行目
    if (tData.dt1c < 1000) lcd.print(F(" "));
    if (tData.dt1c < 100)  lcd.print(F(" "));
    if (tData.dt1c < 10)   lcd.print(F(" "));
    lcd.print(tData.dt1c, 0);
    lcd.print(F("ppm"));
  }
}
#endif // SEKISAN

////////////////////////////////////////////////////
//   write xEEPROM log data to SD with date       //
////////////////////////////////////////////////////

void writeFieldName() {
  logfile.println(F("Date,Time,Temp(C),RH(%),CO2(ppm)"));
}

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
  if (tLogIntervalUnit == SEC_INTERVAL) {
    logfile.print(F(":"));
    logfile.print(tm.Second, DEC);
  }
  logfile.print(F(","));
  if (tData.dt1a != NULLDATA_MARK) {
    logfile.print(tData.dt1a, 1);
  }
  logfile.print(F(","));
  if (tData.dt1b != NULLDATA_MARK) {
    logfile.print(tData.dt1b, 1);
  }
  logfile.print(F(","));
  if (tData.dt1c != NULLDATA_MARK) {
    logfile.print(tData.dt1c, 0);
  }
/////////////////////////////////////////////////////////////////////////////////////////////////////
  logfile.println();
}
#endif // SENSOR_SCD40
