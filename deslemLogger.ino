// 電池長期間駆動Arduinoロガーに使っているスケッチ
// deslemLogger (deep sleep EEPROM logger)
// https://github.com/citriena/deslemLogger
// Copyright (C) 2021 by citriena
//
// センサーの種類、ロガーの動作等の設定変更は deslemLoggerConfig.h 内で行う。

#define SdFatLite               // 標準のSDライブラリでは無く、メモリ使用量が少ないSdFatLiteライブラリを使う。
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#ifdef SdFatLite                // 設定によっては標準のSDライブラリではスケッチが容量オーバーとなる。
#include <SdFat.h>              // https://github.com/greiman/SdFat/releases/tag/1.1.4
#else                           // SdFatLite (1.1.4)ではスケッチ容量が大幅に小さくなる(13%以上)
#include <SD.h>
#endif
#include <EEPROM.h>
#include <TimeLib.h>            // https://github.com/PaulStoffregen/Time
#include <ST7032.h>             // https://github.com/tomozh/arduino_ST7032
#include <RX8900RTC.h>          // https://github.com/citriena/RX8900RTC
#include <EEPROM_24xx1025.h>    // https://github.com/citriena/EEPROM_24xx1025
#include "timeLibSub.h"
#include "deslemLoggerConfig.h"

#ifdef NTC
#include "sensorNTC.h"  // 他のセンサライブラリは同じフォルダに置かない（コンパイルエラーとなる）。
#endif                  // 入れ替えたらArduino IDE再起動必要

#ifdef SHT
#include "sensorSHT.h"  // 他のセンサライブラリは同じフォルダに置かない（コンパイルエラーとなる）。
#endif                  // 入れ替えたらArduino IDE再起動必要

#ifdef SEKISAN
#define MENU_NO          5 // LCDでメニューを表示する場合のgDispModeの番号。0は基本画面表示
#else                      // 積算の場合は０－４が積算の切替で5が処理メニュー
#define MENU_NO          1 // 積算以外では基本画面(0)と処理メニュー画面(1)の２つ
#endif                     // 必要があれば、複数の別画面表示とすることも可能
// その場合はMENU_NOを2以降にずらし、空いたgDispModeに対応する表示をlcdTime(), lcdData()等で定義する。

/////////////////////////////////////////////////////////////////
//           ライブラリ初期化設定（24xx1025の数等により変更）
/////////////////////////////////////////////////////////////////

// set external EEPROM
// set I2C addresses of external 24x1025 EEPROM connected; see EEPROM_24xx1025.h
// EEPROMの数、I2Cアドレスに従って設定する。
//EEPROM_24xx1025 exEeprom(EPR_ADDR0); // 24xx1025 1個
//EEPROM_24xx1025 exEeprom(EPR_ADDR0, EPR_ADDR1); // 24xx1025 2個
//EEPROM_24xx1025 exEeprom(EPR_ADDR2, EPR_ADDR3); // 24xx1025 2個 実装のアドレスに従う。
//EEPROM_24xx1025 exEeprom(EPR_ADDR0, EPR_ADDR1, EPR_ADDR2); // 24xx1025 3個
EEPROM_24xx1025 exEeprom(EPR_ADDR0, EPR_ADDR1, EPR_ADDR2, EPR_ADDR3); // 24xx1025 4個

//************** USER SETTING END ***********************************
//////////////////////////////////////////////////////////////////////

RX8900RTC RTC;

ST7032 lcd;

#ifdef SdFatLite
SdFat SD;
#endif

////////////////////////////////////////////////////////////////
// 　型宣言 + 付属広域変数宣言　データロガー部　共通部
/////////////////////////////////////////////////////////////////

///////////////////////////////////////////
// 外付EEPROM内ログに定期的に書き込むヘッダ
typedef struct {
  byte startMark;
  byte Year;
  byte Month;
  byte Day;
  byte Hour;
  byte Minute;
  byte Second;
  byte LogInterval; // 上位2ビット：単位, 下位6ビット：記録間隔
} emLogHeader_t;    //ログデータヘッダ型


typedef struct {
  byte startMark;
  byte Year;
  byte Month;
  byte Day;
  byte Hour;
  byte Minute;
  byte Second;
  byte LogInterval; // 上位2ビット：単位, 下位6ビット：記録間隔
  byte endMark;     // 0xFF
} emLogHeaderWriter_t;  //ログデータヘッダ型（書き込み用に最後にendMark追加）
///////////////////////////////////////////


///////////////////////////////////////////
// 外付EEPROMデータ書込み用バッファ
typedef struct {
  emData_t data[EM_DATA_PER_BUFF];
  byte endMark;  // 0xFF
} emDataBuff_t;  // EEPROMに書き込むデータバッファ型

emDataBuff_t gEmDataBuff;

byte gEmDataNo = 0;      // バッファ内のデータ書込数（正確にはバッファ内の場所情報）
byte gEmBuffWriteNo = 0; // ヘッダ以降のバッファ書込回数
///////////////////////////////////////////


// ログ等の時間間隔単位
typedef enum {
  SEC_INTERVAL  = 0x00,
  MIN_INTERVAL  = 0x01,
  HOUR_INTERVAL = 0x02,
  //  DAY_INTERVAL  = 0x03
} intervalUnit_t;


typedef enum {  // EEPROM書込アドレスを進めるかどうか。データをmicroSDに転送時に事前にバッファをmicroSDに書き込み、転送漏れを防いでいる。
  INC_ADDRESS,  // ただし通常のデータ処理に影響を与えないように、書き込みアドレス等は変更しない。その機能のための定義。DEC_ADDRESSは定義しているが使っていない。
  STAY_ADDRESS,
  DEC_ADDRESS
} addressChange_t;


typedef enum {  // 時計表示のモード
  DISP_TIME_MODE, // 時刻表示時
  SET_TIME_MODE   // 時刻設定時
} lcdTimeMode_t;


typedef enum {
  ENDLESS_MODE,
  WRITE_ONCE_MODE
} logMode_t;

/////////////////////////////////////////////////////////////////
// 　　　　　　広域変数宣言  ユーザー設定（ロギング条件設定）。具体的数値はdeslemLoggerConfig.h内のマクロ定義で設定
// いずれメニューで設定できるようにしたい（このため広域変数）。
// LCDの時刻更新はタイマー起動時なので、LCDに時刻を表示する場合はタイマー割り込み間隔を1分間隔以下としないと時刻表示更新が遅れる。
/////////////////////////////////////////////////////////////////
logMode_t gLogMode = LOG_MODE;                 // endless mode か write once modeか；　LOG_MODはdeslemLoggerConfig.hで定義
boolean gIsLogging = true;                     // ロギング実施フラグ cycleJob()実行時にこのフラグに従って処理をする（現在は使っていない。将来実装予定）。
byte gTimerInterval =   TIMER_INTERVAL;        // タイマー割り込み間隔。有効なのはgIntervalUnit = SEC_INTERVALの時だけで、それ以外の時は1分
byte gMeasureInterval = MEASURE_INTERVAL;      // 測定間隔
byte gLogInterval =     LOG_INTERVAL;          // 記録間隔。前回記録以降の測定データの平均値を記録する
intervalUnit_t gIntervalUnit = INTERVAL_UNIT;  // 上記の間隔の単位（時、分、秒）


//Global variables
tmElements_t gFileTm;      // ファイル日時を設定するマクロで用い、ファイル日時を記録開始日時とするために使用する。
long gWriteEmAddress = 0;  // 外部EEPROMのログ書込アドレス
byte gDispMode = 0;        // ボタンを押したときに変える画面の種類。0は基本モード、#define で設定するMENU_NOはメニューの番号（gDispModeの最大値）

volatile boolean rtcint = false;   // 割り込み処理中のフラグ（RTCタイマー） 割り込みサービスルーチンで設定するので volatile必要
volatile boolean lvlint = false;   // 割り込み処理中のフラグ（手動割り込み） 割り込みサービスルーチンで設定するので volatile必要
//volatile boolean rtcintFirst = false;   // 定周期タイマー起動のための割り込み処理中のフラグ（RTCタイマー） 割り込みサービスルーチンで設定するので volatile必要

// the logging file
#ifdef SdFatLite
SdFile logfile;
#else
File logfile;
#endif

/////////////////////////////////////////////////////////////////
// 　　　　　　　　　　　　　　基本処理
/////////////////////////////////////////////////////////////////

#ifndef SET_ID
void setup() {
  char loggerId[13] = "";

  //  Serial.begin(9600);
  Wire.begin();
  lcd.begin(16, 2, LCD_5x8DOTS);
  lcd.setContrast(40);
  clearIcon();                // clear icon
  lowVdetect();               // 電圧低下していたらアイコン表示

  EEPROM.get(LOGGER_ID_ADDRESS, loggerId);
  lcd.setCursor(0, 1);
  lcd.print(F("ID:"));
  lcd.print(loggerId);

  RTC.init();
  setTimeButton();
#ifdef SEKISAN
  readBackupData(RTC.read());
#endif
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("SEARCH DATA AREA"));
  if (!searchEmDataEnd()) {          // ログデータ記憶領域設定のため、最初のデータ未使用域を探す。
    lcd.setCursor(0, 1);             // ライトワンスで最後まで書き込んで、次に起動したらこれが実行されるか。
    lcd.print(F("DATA FULL;PRESS")); 
    gIsLogging = false;              // 見つからなかったらとりあえずログを停止する。
     while (digitalRead(MANUAL_INT_PIN) == HIGH); // キーが押されるまで待つ。
  }
  lcd.clear();
  initSensor();              // initialize sensor
  lcdTime(RTC.read());       // display time on the LCD
  lcdData(getData(), 0);     // display temperatures on the LCD
  setManualInt();            // set manual interrupt
  setAlarmInt();             // set periodical alarm interrupt
  resetEmDataBuff();         //
  lvlint = false;            // これが無いとボタンを押した表示になることがある。
}

void loop() {

  if (lvlint) { // interrupt call by manual button
    manualJob();
    lvlint = false;
  }
  if (rtcint) { // interrupt call by timer
    timerJob();
    rtcint = false;
  }
  //  if (rtcintFirst) { // interrupt call by timer
  //    setFixedCycleTimer();
  //    rtcint = false;
  //  }
  enterSleep();  //go to power save mode
  //  ADCSRA |= (1 << ADEN);  // ADC ON
}


/////////////////////////////////////////////////////////////////
// 　　　　　　　　　　　　ボタン操作処理関係
/////////////////////////////////////////////////////////////////

void manualJob() { // ボタンを押したら時の処理
  lcd.display();  // re-desplay during LCD off time
  lcd.noBlink();  // 積算中等をブリンクで表示しているので、一旦ブリンクを停止する。
  if (keyLongPressed()) {
    switch (gDispMode) {
      case MENU_NO:
        shoriMenu();
        gDispMode = 0; // 処理メニューの次は表示を最初に戻す（メニューが最後）。
        break;
      default:
#ifdef SEKISAN
        sekisanStart(RTC.read(), gDispMode);  // 積算の場合は、ボタン長押しで積算開始、停止の切替
        while (digitalRead(MANUAL_INT_PIN)  == LOW); // キーが離されるまで待つ。
#endif
        break;
    }
    lcdTime(RTC.read());
    lcdData(getData(), gDispMode);
    return;  // 設定に入ったらgDispModeは元のまま
  }
  gDispMode++;
  if (gDispMode > MENU_NO) gDispMode = 0;
  if (gDispMode == MENU_NO) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("SHORI"));
  } else {
    lcdTime(RTC.read());
    lcdData(getData(), gDispMode);
  }
  while (digitalRead(MANUAL_INT_PIN) == LOW); // キーが離されるまで待つ。
}


void shoriMenu() {
  byte setteiNo = 0;
  int count = 0;
  bool selected = false;
  bool keyPressed = true;
  do {
    if (keyPressed) {
      keyPressed = false;
      lcd.setCursor(0, 1);
      switch (setteiNo) {
        case 0: // 日時設定
          if (selected) {
            setTimeButton();
          } else {
            lcd.print(F("DATE&TIME    "));
          }
          break;
        case 1: // データ回収
          if (selected) {
            log2SD();
          } else {
            lcd.print(F("DATA COPY    "));
          }
          break;
        case 2: // リセット
          if (selected) {
            lcd.setCursor(0, 1);
            lcd.print(F("Press to RESET  "));
            while (digitalRead(MANUAL_INT_PIN) == LOW); // キーが離されるまで待つ。
            delay(50);
            int ct = 0;
            selected = false;
            do {
              if (digitalRead(MANUAL_INT_PIN) == LOW) {
                resetAllData();
                delay(500);
                lcd.clear();
                ct = 500;
                selected = true;
              }
              delay(10);
              ct++;
            } while (ct < 500);   // ボタンが押されるか5秒待つ。delay(10)×500で5秒
          } else {
            lcd.print(F("DATA RESET   "));
          }
          break;
        /*
                case 3:
                  if (selected) {
                    setLogging();
                  } else {
                    lcd.print(F("LOGGING SETTINGS"));
                  }
                  break;
        */
        case 3: // 戻る
          if (!selected) {
            lcd.print(F("BACK         "));
          }
          break;
      }
      if (selected) return;
      while (digitalRead(MANUAL_INT_PIN) == LOW); // キーが離されるまで待つ。
    }
    if (digitalRead(MANUAL_INT_PIN) == LOW) {
      keyPressed = true;
      count = 0;
      if (keyLongPressed()) {          // キーが長押しされたら
        selected = true;               // 選択されたと判定し、次回のswitch文で処理実施
      } else {
        setteiNo++;
        if (setteiNo > 3) setteiNo = 0;  // 4を超えたら元に戻る。
      }
    }
    delay(10);
    count++;
  } while (count < 500); // delay(10)なので、約5秒押さなかったら設定完了
}

/*
  void setLogging() {
  lcd.clear();

  }

*/


bool keyLongPressed() {
  byte j = 0;
  for (byte i = 0; i < 200; i++) {         // さらに長押しされるか待って設定に移行
    delay(10);
    if (digitalRead(MANUAL_INT_PIN)  == HIGH) j++; // チャタリング対応のため合計で判定
    if (j > 30) return false; // 10だと低品質のタクトスイッチで判定できないことがある。
  }
  return true;
}


/////////////////////////////////////////////////////////////////
// 　　　　　　　　　　　　　　タイマー割り込み関係
/////////////////////////////////////////////////////////////////

void timerJob() {
  tmElements_t tm;

  tm = RTC.read();
  lcdTime(tm);
  lcdControl(tm.Hour); // blink止まるのでcycleJob()の前に実行する必要あり
  if (isCycleTime(tm, gMeasureInterval)) {
    cycleJob(tm);
  }
}


/////////////////////////////////////////////////////////////////
//                    タイマー割り込み処理設定
//                    現在は1秒～1分間隔まで
/////////////////////////////////////////////////////////////////
void setAlarmInt() {  //set alarm to fire every interval time
  attachInterrupt(digitalPinToInterrupt(ALARM_INT_PIN), alcall, FALLING);
  if (gIntervalUnit == SEC_INTERVAL) {
    while (RTC.read().Second % gTimerInterval) {}; // 呼び出しタイミングまで待つ
    RTC.setFixedCycleTimer(gTimerInterval, SECOND_UPDATE); // 定周期タイマー起動
    RTC.fixedCycleTimerInterrupt(ENABLE);
  } else {
    RTC.setTimeUpdateTimer(UPDATE_MINUTE_INT); // set update interrupt timing to every minute
    RTC.timeUpdateTimerInterrupt(ENABLE);
  }
}


void setManualInt() {   // attach Interrupt for key pressed trigger
  attachInterrupt(digitalPinToInterrupt(MANUAL_INT_PIN), lvcall, FALLING);
}


void alcall() {
  //interrupt call by RTC trigger
  rtcint = true;
}

void lvcall() {
  //interrupt call by key pressed trigger
  lvlint = true;
}


/*
  // 2分以上とする場合は、 setlarmInt()で次の割り込み時刻を設定し、attachInterrupでalcallFirstを起動するよう設定する。
  そこで呼び出された時に定周期タイマーを起動する。

  void setFixedCycleTimerInt() {  //set alarm to fire every interval time
  attachInterrupt(digitalPinToInterrupt(ALARM_INT_PIN), alcall, FALLING);
  RTC.setFixedCycleTimer(interval, MINUTE_UPDATE); // set fixed cycle timer
  RTC.fixedCycleTimerInterrupt(ENABLE);
  }


  void setlarmInt() {  //set alarm to fire every interval time
  attachInterrupt(digitalPinToInterrupt(ALARM_INT_PIN), alcallFirst, FALLING);
  RTC.setAlarm(minute, second); // set alarm interrupt 定周期タイマーを起動する時刻を設定
  RTC.alarmInterrupt(ENABLE);
  }


  void alcallFirst() {
  //interrupt call by RTC trigger
  rtcintFirst = true;
  }
*/

//////////////////////////////////////////////////////////////////////////////
//                      定周期測定処理（計測＆ロギング）
//////////////////////////////////////////////////////////////////////////////

void cycleJob(tmElements_t tm) {

  data_t tData;  // 計測データ
  data_t aData;  // 計測データの平均値（前回ログ以降）
  static tmElements_t headerTm;  // ヘッダの時刻
  int timeDiff;                  // ヘッダからの経過時間
  byte buffPoint;                // バッファ内の書き込み位置
  byte buffCount;                // ヘッダ以降バッファ書き込み番号
  static byte prevBuffCount = 0;       // 前回のバッファ書き込み番号。異なっていればバッファをEEPROMに書き込み後バッファリセット
  static boolean buffEmpty = true;     // 不要か
  static boolean isLogging = false;    // ロギング処理実施中
  static boolean updateHeader = true;  // 最初はヘッダが必要
  static boolean updateBuffer = false;
  bool isNewLog;                       // 新しいログファイルにするかどうか。この情報をヘッダに書き込んでおき、読み出すときにこれに従って処理。

  tData = getData();                   // センサ等からデータ取得
  lcdData(tData, gDispMode);

  // ロギングを停止できるようにする時用のコード
  //いずれメニューで、ロギング開始、停止、ロギング間隔を設定できるようにしたい。

  if (!gIsLogging) {
    if (isLogging) {
      if (buffEmpty == false) {  // もしバッファにデータが残っていたら。最後のバッファ位置がメニュー処理等で書き込めなかった場合に対応
        buff2em(INC_ADDRESS);    // バッファをEEPROMに書き込み
        buffEmpty = true;        // バッファ空
      }
      isLogging = false;
    }
    return;
  } else {
    if (!isLogging) {
      updateHeader = true;
      headerTm = tm;
      prevBuffCount = 0;
      isLogging = true;
      isNewLog = true;
    } else {
      isNewLog = false;
    }
  }

  if (isCycleTime(tm, gLogInterval)) {  // ログ時刻ならログ記録処理
    aData = avgData();
#ifdef SEKISAN
    dataSekisan(aData, tm);
#endif
    // ヘッダ時刻からの経過時間算出
    if (gIntervalUnit == MIN_INTERVAL) {         // 分単位の場合
      timeDiff = minDiff(headerTm, tm);
    } else if (gIntervalUnit == SEC_INTERVAL) {  // 秒単位の場合
      timeDiff = secDiff(headerTm, tm);
    } else {
      timeDiff = hourDiff(headerTm, tm);            // 時単位の場合
    }
    // ヘッダ時刻からの経過時間を基にデータを書き込むバッファ内の位置等を算出
    // 書込み位置をtmのincrementで算出しないのは、メニュー処理等で計測が飛んだ場合に対応するため
    buffPoint = timeDiff / gLogInterval;      // ヘッダ時刻からのログ回数＝を算出
    buffCount = buffPoint / EM_DATA_PER_BUFF; // バッファの番号を算出
    buffPoint %= EM_DATA_PER_BUFF;            // バッファ内の位置を算出

    // ここからヘッダ更新が必要かどうかの判断処理（ヘッダ更新時はバッファ更新も必要）
    // バッファ番号が最大値を超えているか、バッファ番号が２つ以上進んでいたらヘッダ更新
    if ((buffCount >= EM_BUFF_WRITE_PER_HEADER) || (buffCount > (prevBuffCount + 1))) {
      updateHeader = true;
      updateBuffer = true;
      if (buffCount > (prevBuffCount + 1)) isNewLog = true; // BuffCountが２つ以上進んでいたらファイルを替える
      // ここからバッファ更新のみ必要がどうかの判断
    } else if (buffCount != prevBuffCount) { // バッファ番号が前回と異なっていたらバッファ更新
      updateBuffer = true;
    }
    // ここから上記の判断に基づいた処理（新たなデータをバッファに書き込む前に必要）
    if (updateBuffer) {              // バッファ更新が必要な場合（以前のデータなのでヘッダ更新前に行う）
      buff2em(INC_ADDRESS);          // すでにバッファにあるデータを EEPROMに書き込み
      updateBuffer = false;
      buffEmpty = true;              // バッファ空
    }
    if (updateHeader) {          // ヘッダ更新が必要な場合（新しいデータ用なのでバッファ更新後に行う）
      headerTm = tm;             // headerTime更新
      setHeader(tm, isNewLog);   // ヘッダ更新、書込み；ヘッダ時刻は最初のデータの時刻
      buffCount = 0;             // ヘッダを更新したのでバッファカウントは０
      buffPoint = 0;             // ヘッダを更新したのでバッファ内の位置は最初
//      prevBuffCount = 0;       // 前回書き込みバッファカウントはリセット; 次のprevBuffCount = buffCount; で問題ないので削除
      updateHeader = false;
    }
    // ここから新たなデータをバッファに書き込む処理
    gEmDataBuff.data[buffPoint] = setEmData(tData); // バッファにデータ書き込み
    prevBuffCount = buffCount;                      // 前回バッファカウントを更新
    buffEmpty = false;                              // 書き込んだのでバッファは空ではない
  }
}


//////////////////////////////////////////////////////////////////////////////
//                      タイマー割り込み、計測、ログ記録時刻等かどうか判定
//////////////////////////////////////////////////////////////////////////////
boolean isCycleTime(tmElements_t tm, byte interval) {

  byte cTime;

  switch (gIntervalUnit) {
    case SEC_INTERVAL: {
      cTime = tm.Second;
      break;
    }
    case MIN_INTERVAL: {
      cTime = tm.Minute;
      break;
    }
    case HOUR_INTERVAL: {
      if (tm.Minute != 0) return false;  // 0分以外は
      cTime = tm.Hour;
      break;
    }
  }
  if ((cTime % interval) != 0) return false; // ロギング等は割り切れる時間
  return true;
}


//////////////////////////////////////////////////////////////////////////////
//                      ログデータをデータバッファに書き込み
//////////////////////////////////////////////////////////////////////////////
//void data2buff(data_t tData, byte buffPoint) {
//  gEmDataBuff.data[buffPoint] = setEmData(tData);
//}  // 1行で済むのと呼び出すのは1カ所だけなので、プログラムメモリ節約のために直接記述にした。


//////////////////////////////////////////////////////////////////////////////
//                   データバッファをEEPROMに書き込み
//////////////////////////////////////////////////////////////////////////////
void buff2em(addressChange_t addressChange) {
  if (gLogMode == WRITE_ONCE_MODE) {          // ライトワンスモードでは
    long nWriteEmAddress = exEeprom.incLongAddress(gWriteEmAddress, sizeof(gEmDataBuff));
    if (nWriteEmAddress < gWriteEmAddress) {  // 次の書込みアドレスが小さくなる場合は
      gIsLogging = false;                     // 今回の書込みが以前のログを上書きするのでロギング停止
      return;
    }
  }
  exEeprom.writeBlock(gWriteEmAddress, gEmDataBuff);
  if (addressChange == INC_ADDRESS) {    // アドレスを進める場合は
    gWriteEmAddress = exEeprom.incLongAddress(gWriteEmAddress, sizeof(gEmDataBuff) - 1); // 最後のendData = 0xFFは上書きするので -1。
    resetEmDataBuff();                   // gEmDataBuffをクリア
  }
}


//////////////////////////////////////////////////////////////////////////////
//                      データバッファをリセット
//////////////////////////////////////////////////////////////////////////////
void resetEmDataBuff() {
  byte i;

  for (i = 0; i < EM_DATA_PER_BUFF; i++) {
    gEmDataBuff.data[i] = nullEmData;
  }
  gEmDataBuff.endMark = EM_FORMAT_MARK;
}


//////////////////////////////////////////////////////////////////////////////
//                      データヘッダをセット＆EEPROMに書き込み
//////////////////////////////////////////////////////////////////////////////
void setHeader(tmElements_t tm, bool isNewLog) {
  emLogHeaderWriter_t emLogHeader;
  byte newLogFlag = 0;

  if (isNewLog) newLogFlag = 0b10000000;  // 月の最上位ビットを新ログファイルかどうかのフラグとする。
  emLogHeader.startMark = EM_HEADER_MARK;
  emLogHeader.Year = tm.Year;
  emLogHeader.Month = tm.Month | newLogFlag;
  emLogHeader.Day = tm.Day;
  emLogHeader.Hour = tm.Hour;
  emLogHeader.Minute = tm.Minute;
  emLogHeader.Second = tm.Second;
  emLogHeader.LogInterval = gLogInterval | ((byte)gIntervalUnit << 6); // 上位2ビット：単位, 下位6ビット：記録間隔
  emLogHeader.endMark = EM_FORMAT_MARK;
  exEeprom.writeBlock(gWriteEmAddress, emLogHeader);                    // ヘッダをEEPROMに書込み
  gWriteEmAddress = exEeprom.incLongAddress(gWriteEmAddress, sizeof(emLogHeader) - 1);  //ヘッダの分 EEPROMの書込みアドレスを進める。ただしendMarkは上書きするので-1
}


/////////////////////////////////////////////////////////////////
// 　　　　　　　　　　　外部EEPROM関係
/////////////////////////////////////////////////////////////////

//*****************************************************
//    identify xEEPROM address to start writing data
//    find first 0xFF which indicates no data
//*****************************************************
boolean searchEmDataEnd() {
  gWriteEmAddress = findEmChar(0, EM_FORMAT_MARK);
  if (gWriteEmAddress == -1) {
    gWriteEmAddress = 0;  // 見つからない場合はアドレス0から書き込む。
    return false;
  }
  return true;
}


/////////////////////////////////////////////
//     transfer xEEPROM log data to SD      //
/////////////////////////////////////////////
void log2SD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Write data to SD"));
  buff2em(STAY_ADDRESS);// バッファデータをEEPROMに書き込む。バッファ満杯ではないのでEEPROM内のポインタは進めない。
  if (em2SD()) {
    lcd.setCursor(0, 0);
    lcd.print(F("Write finished  "));
  } else {
    lcd.setCursor(0, 0);
    lcd.print(F("Write failed  "));
  }
  delay(2000);
  lcd.clear();
}


//////////////////////////////////////////////////////////////////////////////
//                      EEPROMのデータをmicroSDカードに書き込み
//////////////////////////////////////////////////////////////////////////////
boolean em2SD() {
  tmElements_t tm;
  //  tmElements_t headerTm;
  long readEmAddress = 0;
  //  long prevReadEmAddress = 0;
  emLogHeader_t emLogHeader;
  emDataBuff_t emDataBuff;
  byte tLogInterval;
  intervalUnit_t tLogIntervalUnit;
  boolean needNewFile = true;
  unsigned long dataCount = 0;  // SDにデータ書出時の処理データ数
  byte i, j;

  if (!initSD()) return false;
  readEmAddress = findEmChar(readEmAddress, EM_HEADER_MARK);  // 上書きされていないヘッダを探す。２周目も問題ない。ただしファイル番号は逆転することもある。
  while (readEmAddress >= 0) { // ヘッダが見つかっていれば
    exEeprom.readBlock(readEmAddress, emLogHeader);  // ヘッダを読み込む
    tm = {emLogHeader.Second, emLogHeader.Minute, emLogHeader.Hour, 0, emLogHeader.Day, (byte)(emLogHeader.Month & 0b01111111), emLogHeader.Year};
    tLogInterval = emLogHeader.LogInterval & 0b00111111;
    tLogIntervalUnit = (intervalUnit_t)(emLogHeader.LogInterval >> 6);
    readEmAddress = exEeprom.incLongAddress(readEmAddress, sizeof(emLogHeader));
    needNewFile = needNewFile || ((emLogHeader.Month & 0b10000000) != 0) ;  // ヘッダに新ファイルフラグが立っていたら
    if (needNewFile) {
      initFile(tm);    // ファイル作成、更新日時を最初のヘッダ時刻にするため、tm設定後に処理する
      needNewFile = false;
    }
    for (j = 0; j < EM_BUFF_WRITE_PER_HEADER; j++) {
      exEeprom.readBlock(readEmAddress, emDataBuff);
      for (i = 0; i < EM_DATA_PER_BUFF; i++) {
        if ((emDataBuff.data[i].data1a == EM_FORMAT_MARK) || (emDataBuff.data[i].data1a == EM_HEADER_MARK)) { // データ終了、またはヘッダだったら
          // ヘッダ更新後最初のバッファを書き込む前に電池が切れたりリセットするとヘッダだけのデータが発生する。ヘッダの後にEM_FORMAT_MARKを書き込むようにしたので、後半は不要のはず。
          logfile.close();
          needNewFile = true;
          j = EM_BUFF_WRITE_PER_HEADER;  // データ読み込みループを抜けて次のヘッダを探す。
          break;
        }
        //        if (needNewFile) { // ヘッダだけのデータファイルを作成しないようにここが良いか。ただわかりにくい。めったにないのでこの案は採用しない。
        //          initFile(tm);    // ファイル作成日時をヘッダ時刻にするため、tm設定後に処理する
        //          needNewFile = false;
        //        }
        writeLog2SD(tm, restoreEmData(emDataBuff.data[i]), tLogIntervalUnit);
        readEmAddress = exEeprom.incLongAddress(readEmAddress, sizeof(emDataBuff.data[0])); // readEmAddressを進める。
        dataCount++;
        if (tLogIntervalUnit == MIN_INTERVAL) {
          tm = incTimeMinutes(tm, tLogInterval);
        } else if (tLogIntervalUnit == SEC_INTERVAL) {
          tm = incTimeSeconds(tm, tLogInterval);
        } else {
          tm = incTimeHours(tm, tLogInterval);
        }
      }
    }
    lcd.setCursor(0, 1);    // 毎回だと表示に時間がかかるので、ループの外に置いて表示回数を減らす
    lcd.print(F("DATA:"));
    lcd.print(dataCount);
    readEmAddress = findEmChar(readEmAddress, EM_HEADER_MARK);  // ヘッダを探す
    //    if (readEmAddress <= prevReadEmAddress) { // exEeprom.incLongAddress()では一周回ると元に戻るのでこの処理が必要
    //      readEmAddress = -1;
    //    } else {
    //      prevReadEmAddress = readEmAddress;
    //    }
  }
  return true;
}


//////////////////////////////////////////////////////////////////////////////
//            EEPROM内でaddrアドレスから最後までの間で markを探す。
//            見つからなかったら -1 を返す。
//////////////////////////////////////////////////////////////////////////////
long findEmChar(long addr, byte mark) {
  byte eGroupData[32];
  long i;
  byte j;
  long maxAddress = exEeprom.maxLongAddress();

  for (i = addr; i <= maxAddress; i += 32) {
    exEeprom.read(i, eGroupData, 32);
    for (j = 0; j < 32; j++) {
      if (eGroupData[j] == mark) {
        if ((i + j) > maxAddress) {  // メモリの最大アドレスを超えると最初に戻って読み込むのでこの処理が必要
          return -1;
        }
        return i + j;
      }
    }
  }
  return  -1; // return -1 if not found
}


////////////////////////////////////////////////////
//   erase ExEEPROM with EM_FORMAT_MARK (0xFF)    //
////////////////////////////////////////////////////

void eraseExEEPROM() {
  long address;
  byte fData[30];
  byte rData[30];
  long sDivider = exEeprom.maxLongAddress() / 100;
  long cDivider = 1;
  byte multiplier = 0;

  for (address = 0; address < 30; address ++) fData[address] = EM_FORMAT_MARK;
  for (address = 0; address < exEeprom.maxLongAddress(); address += 30) {
    long thres = address / cDivider;
    if (thres > 0) {
      lcd.setCursor(0, 1);
      lcd.print(multiplier);
      lcd.print(F("%"));
      multiplier++;
      cDivider = sDivider * multiplier;
    }
    exEeprom.read(address, rData, 30);  // 消去データと同じなら書き込まない。
    if (rData != fData) {
      exEeprom.write(address, fData, 30);
    }
  }
}


void resetAllData() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("RESET DATA"));
#ifdef SEKISAN
  resetBackupData();
#endif
  eraseExEEPROM();
  lcd.setCursor(0, 1);
  lcd.print(F("RESET COMPLETED"));
  delay(1000);
  asm volatile ("  jmp 0");  // 再起動。現在は再起動しないとEEPROM書込み位置がおかしくなる。対策必要
}


//////////////////////////////////////////////////////////////////////////////
// 　　　　　　　　　　　SDカード関係
//////////////////////////////////////////////////////////////////////////////

//////////////// initialize SD card  ////////////////
boolean initSD() {
  // initialize the SD card
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(CHIP_SELECT, OUTPUT); // T.Ogata
  // see if the card is present and can be initialized:
  if (!SD.begin(CHIP_SELECT)) {
    errorDetect("SD card");
    return false;
  } else {
    return true;
  }
}


/////////////////////////////////////////////////////////////////
//                  ファイル作成＆ＩＤ等書き込み
/////////////////////////////////////////////////////////////////
boolean initFile(tmElements_t tm) { // 最初のヘッダ時刻をファイル作成時刻とする
  char loggerId[13] = "";

  if (!createFile(tm)) return false;
  EEPROM.get(LOGGER_ID_ADDRESS, loggerId);
  logfile.print(F("Logger ID,"));
  logfile.println(loggerId);
  return true;
}


/////////////////////////////////////////////////////////////////
/////////////// create a new file //////////////////////
/////////////////////////////////////////////////////////////////
boolean createFile(tmElements_t tm) {
  char filename[] = "LOGGER00.CSV";
  bool isFileOpen;

  gFileTm = tm;  // dateTime はこの時刻を返すように設定している。すなわちtmがファイル作成時刻になる。
  // set date and time to the time stamp
  SdFile::dateTimeCallback( &dateTime );
  for (byte i = 0; i < 100; i++) {
    filename[6] = i / 10 + '0';
    filename[7] = i % 10 + '0';
    if (! SD.exists(filename)) {
      // only open a new file if it doesn't exist
#ifdef SdFatLite
      isFileOpen = logfile.open(filename, O_WRONLY | O_CREAT | O_EXCL);
#else
      logfile = SD.open(filename, FILE_WRITE);
#endif
      break;  // leave the loop!
    }
  }
#ifdef SdFatLite
  if (!isFileOpen) {
#else
  if (!logfile) {
#endif
    errorDetect("fail create file");
    return false;
  }
  return true;
}


/////////////////////////////////////////////////////////////////
// SDカードのファイル作成日付設定用
// 色んな所で使われているコードで、どれがオリジナルかは不明
/////////////////////////////////////////////////////////////////
void dateTime(uint16_t* date, uint16_t* time) {

  //setSyncProvider(RTC.get); //sync time with RTC
  //  tmElements_t tm = RTC.read();

  // FAT_DATEマクロでフィールドを埋めて日付を返す
  // fill date to FAT_DATE macro
  //  *date = FAT_DATE(tmYearToCalendar(tm.Year), tm.Month, tm.Day);
  *date = FAT_DATE(tmYearToCalendar(gFileTm.Year), gFileTm.Month, gFileTm.Day);

  // FAT_TIMEマクロでフィールドを埋めて時間を返す
  // fill time to FAT_TIME macro
  //  *time = FAT_TIME(tm.Hour, tm.Minute, tm.Second);
  *time = FAT_TIME(gFileTm.Hour, gFileTm.Minute, gFileTm.Second);
}


//////////////////// RTC SET/////////////////////////
// set RTC clock using only one push button switch //
/////////////////////////////////////////////////////
void setTimeButton() {
  int i;
  byte tStart[5] = {CalendarYrToTm(2020), 1, 1, 0, 0}; // Year member of tmElements_t is the offset from 1970.
  byte tValue[5];
  byte tLimit[5] = {CalendarYrToTm(2099), 12, 31, 23, 59};
  byte item = 0; // year=0, month=1, day=2, hour=3, minute=4
  bool isChanged = false;    // キーを全く押さずにいたら最後の０秒設定は行なわない。

  tmElements_t tm = RTC.read();
  tValue[0] = tm.Year;
  tValue[1] = tm.Month;
  tValue[2] = tm.Day;
  tValue[3] = tm.Hour;
  tValue[4] = tm.Minute;

  byte cursorColumn[5] = {0, 5, 8, 0x0b, 0x0e}; // 設定部を示すためにブリンクさせるカラム
  // |    |  |  |  |
  // 0123456789abcdef
  // 2019/07/01 16:15

  //  lcd.clear();
  while (item < 5) { // YearからMinuteまで順番に設定
    lcdTime(tm, SET_TIME_MODE, cursorColumn[item]);
    delay(500); // wait time for next switch detect
    for (i = 0; i < 500; i++) {
      if (digitalRead(MANUAL_INT_PIN)  == LOW) break;
      delay(10); //大きすぎるとボタンを押してから反応が遅れる。
    }
    if (i == 500) { // button not pressed for 10mS x 500 = 5s  約５秒ボタンが押されなければ修正なし、もしくは設定終了と判断
      item++; // move to next item
      if (item == 2) tLimit[2] = daysInMonth(tm); // last day number in the month
    } else {
      tValue[item]++;  // if button is pressed increase the value
      if (tValue[item] > tLimit[item]) tValue[item] = tStart[item]; // value will roll over from the start point if reaches over the limit
      isChanged = true;
    }
    tm = {0, tValue[4], tValue[3], 0, tValue[2], tValue[1], tValue[0]}; //設定値をtmに代入したら whileに戻って表示
  }
  if (isChanged) {  // set RTC if date and time changed  変更があったらRTCの設定を行なう。
    lcd.setCursor(0, 1);
    lcd.print(F("Press to set"));
    while (digitalRead(MANUAL_INT_PIN)  == HIGH) {}; // ボタンが押されるのを待つ。
    RTC.write(tm);                                   // ボタンが押されたら設定値をRTCに設定。ゼロ秒で押せば秒まで正確に設定できる。
  } // do not set RTC if date and time are not changed  変更がなければRTCの設定はしない。
  lcd.noBlink();
  while (digitalRead(MANUAL_INT_PIN)  == LOW) {};    // 次の処理に影響しないようにボタンが離されるのを待つ。
}


/////////////////////////////////////////////////////////////////
// 　　　　　　　　　　　AVR & LCD ハードウェア処理関係
/////////////////////////////////////////////////////////////////


////////////// deep sleep ////////////////////
//  http://tetsuakibaba.jp/index.php?page=workshop/ArduinoBasis/sleep

void enterSleep(void) {

  // 待ち時間
  // すぐにsleepするとSerialの表示が間に合わないことがある。
  // need delay to finish Serial print
  delay(100);

  // set power-down mode
  SMCR |= (1 << SM1);
  SMCR |= 1;

  // disable ADC
  ADCSRA &= ~(1 << ADEN);

  // disable BOD
  MCUCR |= (1 << BODSE) | (1 << BODS);
  MCUCR = (MCUCR & ~(1 << BODSE)) | (1 << BODS);
  asm("sleep");

  // enable ADC
  //ADCSRA |= (1 << 7);
  ADCSRA |= (1 << ADEN);
}

//////////////////////////////////////////////////////////
/////// Detect Voltage Low and display icon  /////////////
//////////////////////////////////////////////////////////
float lowVdetect() {

  float vcc = cpuVcc();

  if (vcc < 2.90) {
    lcd.setIcon(0x0d, 0b00010); // show battery icon (empty)
  } else if (vcc < 3.05) {
    lcd.setIcon(0x0d, 0b00110); // show battery icon (low)
  } else if (vcc < 3.20) {
    lcd.setIcon(0x0d, 0b01110); // show battery icon (middle)
  } else {
    lcd.setIcon(0x0d, 0b00000); // hide battery icon
  }
  return vcc;
}

// http://radiopench.blog96.fc2.com/blog-entry-490.html //
float cpuVcc() {                     // 電源電圧(AVCC)測定関数
  long sum = 0;
  adcSetup(0x4E);                    // Vref=AVcc, input=internal1.1V
  for (byte n = 0; n < 10; n++) {
    sum = sum + adc();               // adcの値を読んで積分
  }
  return (1.1 * 10240.0) / sum;      // 電圧を計算して戻り値にする
}

void adcSetup(byte data) {           // ADコンバーターの設定
  ADMUX = data;                      // ADC Multiplexer Select Reg.
  ADCSRA |= ( 1 << ADEN);            // ADC イネーブル
  ADCSRA |= 0x07;                    // AD変換クロック　CK/128
  delay(10);                         // 安定するまで待つ
}

unsigned int adc() {                 // ADCの値を読む
  unsigned int dL, dH;
  ADCSRA |= ( 1 << ADSC);            // AD変換開始
  while (ADCSRA & ( 1 << ADSC) ) {   // 変換完了待ち
  }
  dL = ADCL;                         // LSB側読み出し
  dH = ADCH;                         // MSB側
  return dL | (dH << 8);             // 10ビットに合成した値を返す
}

#else //SET_ID

//char gLoggerId[13] = "Logger14";
char hLoggerId[13] = SET_ID;


void setup() {
  // put your setup code here, to run once:

  Serial.begin(9600);
  setId();
//  setDV_R();
}


void setId() {
  EEPROM.put(LOGGER_ID_ADDRESS, gLoggerId);
  Serial.print("Logger ID is set to ");
  EEPROM.get(LOGGER_ID_ADDRESS, gLoggerId);
  Serial.println(gLoggerId);
}


void setDV_R() {
  long dv_r;
  const unsigned int setMark = 0xfdfe;

// store the specific TM_R resistance in the AVR EEPROM
#ifdef THERMISTOR_DVR1
  EEPROM.put(DV_R_ADDRESS, setMark);
  EEPROM.put(DV_R_ADDRESS + sizeof(setMark), (long)THERMISTOR_DVR1);
  EEPROM.get(DV_R_ADDRESS + sizeof(setMark), dv_r);
  Serial.print(F("DV_R1 is set to "));
  Serial.println(dv_r);
#endif
#ifdef THERMISTOR_DVR2
  EEPROM.put(DV_R_ADDRESS + sizeof(setMark) + sizeof((long)THERMISTOR_DVR1), (long)THERMISTOR_DVR2);
  EEPROM.get(DV_R_ADDRESS + sizeof(setMark) + sizeof((long)THERMISTOR_DVR1), dv_r);
  Serial.print(F("DV_R2 is set to "));
  Serial.println(dv_r);
#endif
}

void loop() {
  
}

#endif