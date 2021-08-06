#ifndef SEKISAN
#ifdef NTC

//////////////////////////////////////////////////////
//          display time to lcd                     //
//////////////////////////////////////////////////////

#define DECIMAL_PLACE 1

void lcdTime(tmElements_t tm, lcdTimeMode_t mode, char cursorColumn) { // LCD first line

  if ((gDispMode == MENU_NO) && (mode == DISP_TIME_MODE)) return;
  lcd.setCursor(0, 0);
  if (mode == SET_TIME_MODE) {  // 設定モード時は年も表示
    lcd.print(tmYearToCalendar(tm.Year));
    lcd.print(F("/"));
  }
  if (tm.Month < 10) lcd.print(F("0"));
  lcd.print(tm.Month);
  lcd.print(F("/"));
  if (tm.Day < 10) lcd.print(F("0"));
  lcd.print(tm.Day);
  if (mode == DISP_TIME_MODE) { // 時分は
    lcd.setCursor(0, 1);        // 表示モード時は２行目
  } else {
    lcd.print(F(" "));          // 設定モード時はスペースを空けて続ける。
  }
  if (tm.Hour < 10) lcd.print(F("0"));
  lcd.print(tm.Hour, DEC);
  lcd.print(F(":"));
  if (tm.Minute < 10) lcd.print(F("0"));
  lcd.print(tm.Minute, DEC);
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
void lcdData(data_t tData, byte atNo) {
  if (gDispMode == MENU_NO) return;
  lcd.setCursor(6, 0);
//  lcd.print(F("A"));
  if (tData.dt1a == NULLDATA_MARK) {
    lcd.print(F(":ERR  "));
  } else {
    if ((tData.dt1a < 10) && (tData.dt1a >= 0)) lcd.print(F(" "));
    lcd.print(tData.dt1a, DECIMAL_PLACE);
    lcd.print(F("C "));
  }
#ifdef DUAL_SENSORS
  lcd.setCursor(6, 1);
//  lcd.print(F("B"));
  if (tData.dt2a == NULLDATA_MARK) {
    lcd.print(F(":ERR  "));
  } else {
    if ((tData.dt2a < 10)  && (tData.dt2a >= 0)) lcd.print(F(" "));
    lcd.print(tData.dt2a, DECIMAL_PLACE);
    lcd.print(F("C "));
  }
#endif
}
  // 0123456789abcdef
  // 04/22 25.0C 100%
  // 12:00 30.0C  80%


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
  if (tLogIntervalUnit == SEC_INTERVAL) {
    logfile.print(F(":"));
    logfile.print(tm.Second, DEC);
  }
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
#endif
