//////////////////////////////////////////////////////
// 電池長期間駆動Arduinoロガー
// deslemLogger (deep sleep EEPROM logger)用補助スケッチ
// LCD表示関係の共通処理
//////////////////////////////////////////////////////


void lcdTime(tmElements_t tm) { // LCD first line
  if (gDispMode < CONFIG_NO) {
    lcd.setCursor(0, 0); // データ画面では時計は1行目
    lcdTime(tm, DATA_TIME_MODE, -1);
  } else if (gDispMode == MENU_NO) {
    lcd.setCursor(0, 1); // メニュー導入画面では時計は2行目
    lcdTime(tm, MENU_TIME_MODE, -1);
  } // CONFIG_MODEでは時計は表示しない。
}


void lcdPrintZero(byte num) {
  if (num < 10) {
    lcd.print(F("0"));
  }
  lcd.print(num);
}


void logIcon(bool showIcon) {
  if (showIcon) {
    lcd.setIcon(0x06, 0b10000); // show recording icon
  } else {
    lcd.setIcon(0x06, 0b00000); // hide recording icon
  }
}


void clearIcon() {
  for (byte i = 0; i < 10; i++) {
    lcd.setIcon(i, 0);
  }
}


/////////// error print and indicate ////////////////
void errorDetect(const char *str) {
  lcd.clear();
  lcd.blink();
  lcd.setCursor(0, 0);
  lcd.print(F("error: "));
  lcd.setCursor(0, 1);
  lcd.println(str);
  lcd.noBlink();
  lcd.clear();
}


/////////////////////////////////////////////////////////////////
// 　　　　　消費電流節約のためにLCD表示を止める時間帯の識別
/////////////////////////////////////////////////////////////////

void lcdControl(byte hr) {
  static byte lastHr = 0;
  byte lcdOn = 0;

  if (lastHr == hr) return;
  lastHr = hr;
  if (hr >= LCD_ON_TIME) ++lcdOn;
  if (hr < LCD_OFF_TIME) ++lcdOn;
  if (LCD_ON_TIME >= LCD_OFF_TIME) ++lcdOn;
  if (lcdOn > 1) {
    lcd.display();
    gLCDon = true;
  } else {
    lcd.noDisplay();
    gLCDon = false;
  }
}
