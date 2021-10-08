//////////////////////////////////////////////////////
// 電池長期間駆動Arduinoロガー
// deslemLogger (deep sleep EEPROM logger)用補助スケッチ
// 使用状況等で変更する設定、複数のソースファイルで共通の設定
// Bosch BME280等温湿度、気圧のセンサのLCD表示，microSDへのデータ書き出しフォーマット設定
// 同じ測定項目のセンサーで共用利用できるようにソースファイルを分離
//////////////////////////////////////////////////////

#ifdef SENSOR_BME280
#ifndef SEKISAN

//////////////////////////////////////////////////////
//          display time to lcd                     //
//////////////////////////////////////////////////////

void lcdTime(tmElements_t tm, lcdTimeMode_t mode, char cursorColumn) {
  if ((gDispMode == MENU_NO) && (mode == DATA_TIME_MODE)) { // 設定画面＆非時計設定モード時
    return;
  } else if (mode == MENU_TIME_MODE) {
    lcd.setCursor(0, 1); // メニュー導入画面では時計は2行目
  } else {
    lcd.setCursor(0, 0); // 時刻設定画面では時計は1行目
  }
  if (mode != DATA_TIME_MODE){  // メニュー導入画面，時計設定モード時は年も表示
    lcd.print(tmYearToCalendar(tm.Year));
    lcd.print(F("/"));
  }
#ifdef DUAL_SENSORS // 2センサー時はLCDに余裕がないのでデータ表示モードでは時刻は表示しない．
  if (mode == DATA_TIME_MODE) return;
#endif
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


void lcdTime(tmElements_t tm, lcdTimeMode_t mode) { // LCD first line
  lcdTime(tm, mode, -1);
}


void lcdTime(tmElements_t tm) { // LCD first line
  lcdTime(tm, DATA_TIME_MODE, -1);
}

//////////////////////////////////////////////////////
//          display data to lcd                     //
//////////////////////////////////////////////////////
void lcdData(data_t tData) {
  if (gDispMode == MENU_NO) return; // RTC割り込みでデータ処理後に通常はデータを表示するが、メニュー時は表示しない。
#ifndef DUAL_SENSORS
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
    lcd.setCursor(7, 1); // 行目
    if (tData.dt1c < 1000) lcd.print(F(" "));
    lcd.print(tData.dt1c, 1);
    lcd.print(F("hPa"));
  }
#else // DUAL_SENSORS
  lcd.setCursor(0, 0);
  if (tData.dt1a == NULLDATA_MARK) {
    lcd.print(F(":ERR  "));
  } else {
    if ((tData.dt1a < 10) && (tData.dt1a >= 0)) lcd.print(F(" "));
    lcd.print(tData.dt1a, 1);
    lcd.print(F("C "));
    if (tData.dt1b < 100) lcd.print(F(" "));
    if (tData.dt1b <  10) lcd.print(F(" "));
    lcd.print(tData.dt1b, 0);
    lcd.print(F("% "));
    if (tData.dt1c < 1000) lcd.print(F(" "));
    if (tData.dt1c == 0) lcd.print(F("  "));
    lcd.print(tData.dt1c, 0);
    lcd.print(F("h"));
  }
  lcd.setCursor(0, 1);
  if (tData.dt2a == NULLDATA_MARK) {
    lcd.print(F(":ERR  "));
  } else {
    if (tData.dt2a < 10) lcd.print(F(" "));
    lcd.print(tData.dt2a, 1);
    lcd.print(F("C "));
    if (tData.dt2b < 100) lcd.print(F(" "));
    if (tData.dt2b <  10) lcd.print(F(" "));
    lcd.print(tData.dt2b, 0);
    lcd.print(F("% "));
    if (tData.dt2c < 1000) lcd.print(F(" "));
    if (tData.dt2c == 0) lcd.print(F("  "));
    lcd.print(tData.dt2c, 0);
    lcd.print(F("h"));
  }
#endif // DUAL_SENSORS
}
  // 0123456789abcdef
  // 04/22 25.0C 100%
  // 12:00  1000.0hPa

  // 0123456789abcdef
  // 25.0C 100% 1000h
  // 25.0C 100% 1000h


#endif // SEKISAN

////////////////////////////////////////////////////
//   write xEEPROM log data to SD with date       //
////////////////////////////////////////////////////

void writeFieldName() {
  logfile.println(F("Date,Time,Temp(C),RH(%),Presssure(hPa)"));
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
    logfile.print(tData.dt1c, 1);
  }
#ifdef DUAL_SENSORS
  logfile.print(F(","));
  if (tData.dt2a != NULLDATA_MARK) {
    logfile.print(tData.dt2a, 1);
  }
  logfile.print(F(","));
  if (tData.dt2b != NULLDATA_MARK) {
    logfile.print(tData.dt2b, 1);
  }
  logfile.print(F(","));
  if (tData.dt2b != NULLDATA_MARK) {
    logfile.print(tData.dt2c, 1);
  }
#endif // DUAL_SENSORS
/////////////////////////////////////////////////////////////////////////////////////////////////////
  logfile.println();
}
#endif // SENSOR_BME280
