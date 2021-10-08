/////////////////////////////////////////////////////////////////
// 電池長期間駆動Arduinoロガー
// deslemLogger (deep sleep EEPROM logger)
// 使用状況等で変更する設定、複数のソースファイルで共通の設定
// 
// 設定内容は
// 1. ロガーID設定
// 2. 使用するセンサ等設定
// 3. EEPROMへの保存設定（範囲，桁数制限して節約するかどうか）
// 4. 積算温度計として使用の設定
// 5. ロギング条件設定（ロギング間隔等）
// 6. EEPROM書込み保存設定
// 7. ハードウェア等設定
// 8. 設定内容保存EEPROM（内部）アドレス設定
// 9. 起動時の時刻設定他
//10. 積算温度関係設定（ここではなくsekisan.ino内）
// 
// 状況によって設定変更するのは一般的に２と５である．他は通常は設定変更不要
// ２を設定変更したらEEPROMに保存されたデータを正しく読み出すことができなくなる
// ので必要なら変更前に読み出しておく．
// ５は変更してもEEPROMに保存されたデータを正しく読み出すことが可能
//
//
/////////////////////////////////////////////////////////////////

#ifndef _deslemLoggerConfig_h
#define _deslemLoggerConfig_h

/////////////////////////////////////////////////////////////////
//                    ロガーID設定
/////////////////////////////////////////////////////////////////
// 複数ロガーを使用する場合にロガーを識別するためのIDをArduinoのEEPROMに書き込む。
// 設定しておけば起動時にはロガーIDが表示される。また，microSDへのデータ書き出し時に
// ファイルの最初にロガーIDが書き込まれる．
// 
// サーミスタ分圧抵抗の正確な値がわかっていれば，THERMISTOR_DVR1，
// THERMISTOR_DVR2を設定することで，SHthermistorライブラリの
// コンストラクタでの設定に優先してその設定値を使って計測を行なう．
// このため，複数のロガーを使っていても各ロガーに書き込まれた最適設定値を
// 使って計測が可能となる．
// ただし，ここで設定できるほど正確に抵抗値を測定するのは容易ではない｡
/////////////////////////////////////////////////////////////////
//#define SET_ID "Logger08"      // ロガーID（英数13文字以内）。設定するとき以外はコメントアウトする。
//#define THERMISTOR_DVR1  10003 // サーミスタ分圧抵抗1の正確な抵抗値
//#define THERMISTOR_DVR2  10003 // サーミスタ分圧抵抗2の正確な抵抗値

/////////////////////////////////////////////////////////////////
//                       使用するセンサ等の設定
//  基本的には1つだけ有効にするが，センサによっては2つ同時に使用可能
/////////////////////////////////////////////////////////////////
/////////////////////////////////
//サーミスタを使う場合
// 2個同時に使用可能
/////////////////////////////////
//#define SENSOR_NTC   // サーミスタを使う場合。サーミスタの特性はsensorNTC.cpp内のコンストラクタで設定
//#define DUAL_SENSORS // サーミスタを２つ使う場合

/////////////////////////////////
// BoschのBME280を使う場合
// 2個同時に設定，使用可能
/////////////////////////////////
#define BME280_x76  // BoschのBME280をI2Cアドレスx76で使う場合
//#define BME280_x77  // BoschのBME280をI2Cアドレスx77で使う場合

/////////////////////////////////
// SHT-31/35を使う場合 秋月電子通商のライブラリ使用。定義名がAE_SHT31だとクラス名と同じためコンパイルエラーとなる。
// 2個同時に設定，使用可能
/////////////////////////////////
//#define SHT3X_A1      // I2Cアドレスは0x44. 秋月電子通商のAE-SHT31では4番ピンをGNDに接続  GY-SHT31-Dのデフォルト。SHT-85も使用可能
//#define SHT3X_A2      // I2Cアドレスは0x45. 秋月電子通商のAE-SHT31のデフォルト（4番ピンオープン）。GY-SHT31-DはAD端子をVddにつなぐ。

/////////////////////////////////
// SHT-31/35を使う場合 Sensirionのライブラリ使用。純正だがフラッシュメモリ使用量が大きい。
// 2個同時に設定，使用可能
/////////////////////////////////
//#define SHT3X_S1     // I2Cアドレスは0x44. 秋月電子通商のAE-SHT31では4番ピンをGNDに接続  GY-SHT31-Dのデフォルト。SHT-85も使用可能
//#define SHT3X_S2     // I2Cアドレスは0x45. 秋月電子通商のAE-SHT31のデフォルト（4番ピンオープン）。GY-SHT31-DはAD端子をVddにつなぐ。

/////////////////////////////////
// SHT-85を使う場合．秋月電子通商のAE-SHT31用ライブラリ使用
/////////////////////////////////
//#define SHT8X_A
/////////////////////////////////

/////////////////////////////////
// SHT-85を使う場合．Sensirionのライブラリ使用
/////////////////////////////////
//#define SHT8X_S

/////////////////////////////////
// SHT-21を使う場合。
// 定義名がSHT21だとクラス名と同じのためコンパイルエラーとなる。
/////////////////////////////////
// #define SHT2X_R

/////////////////////////////////
// HTU21を使う場合。実はSHT-21と同じライブラリが使えるようだ。
/////////////////////////////////
//#define HTU21

/////////////////////////////////

/////////////////////////////////////////////////////////////////
#ifdef SENSOR_NTC  // サーミスタを使う場合の共通定義
#define SENSOR_NAME "Thermistor"  // この定義がmicroSDに書き出されるログファイルの最初に記録される．
#endif

#if defined(BME280_x76) || defined(BME280_x77)  // BME280を使う場合の共通定義
#define SENSOR_BME280
#define SENSOR_NAME "BME280"  // この定義がmicroSDに書き出されるログファイルの最初に記録される．
#endif

#if defined(BME280_x76) && defined(BME280_x77)  // BME280をアドレス違いで2つ使う場合の条件コンパイルのための設定
#define DUAL_SENSORS
#endif

#if defined(SHT3X_A1) || defined(SHT3X_A2) || defined(SHT3X_S1) || defined(SHT3X_S2)
#define SENSOR_SHT  // Sensirion SHT-31/35を使う場合の共通定義
#define SENSOR_NAME "SHT-31/35"  // この定義がmicroSDに書き出されるログファイルの最初に記録される．
#endif

#if defined(SHT3X_A1) && defined(SHT3X_A2) // SHT3Xをアドレス違いで2つ使う場合の条件コンパイルのための設定 
#define DUAL_SENSORS
#endif

#if defined(SHT3X_S1) && defined(SHT3X_S2) // SHT3Xをアドレス違いで2つ使う場合の条件コンパイルのための設定
#define DUAL_SENSORS
#endif

#if defined(SHT8X_A)
#define SENSOR_SHT  // Sensirion SHT-21, SHT-31, SHT-85 やHTU21を使う場合の共通定義
#define SHT3X_A1
#define SENSOR_NAME "SHT-85"  // この定義がmicroSDに書き出されるログファイルの最初に記録される．
#endif

#if defined(SHT8X_S)
#define SENSOR_SHT  // Sensirion SHT-21, SHT-31, SHT-85 やHTU21を使う場合の共通定義
#define SHT3X_S1
#define SENSOR_NAME "SHT-85"  // この定義がmicroSDに書き出されるログファイルの最初に記録される．
#endif

#if defined(SHT2X_R)
#define SENSOR_SHT  // Sensirion SHT-21, SHT-31, SHT-85 やHTU21を使う場合の共通定義
#define SENSOR_NAME "SHT-21"  // この定義がmicroSDに書き出されるログファイルの最初に記録される．
#endif

#if defined(HTU21)
#define SENSOR_SHT  // Sensirion SHT-21, SHT-31, SHT-85 やHTU21を使う場合の共通定義
#define SENSOR_NAME "HTU-21"  // この定義がmicroSDに書き出されるログファイルの最初に記録される．
#endif


/////////////////////////////////////////////////////////////////
//                       EEPROMへの保存設定
// センサデータをEEPROMに記録する際，データの範囲，桁数を制限することでEEPROMの使用量を調整する．
//　長期ロギングでなければHQ_DATAで問題ないが，長期ロギングの場合は必要に応じて変更する．
//　実装は各センサ用ライブラリ内．一部実装してない場合もあるが，HQ_DATAはどのセンサでも使えるようにしている．
/////////////////////////////////////////////////////////////////
//#define LQ_DATA   // 記録するセンサデ－タを範囲制限，桁数制限し，EEPROM使用量を節約する．
//#define SQ_DATA   // 記録するセンサデ－タを実用上十分な範囲，桁数にする．
#define HQ_DATA   // 記録するセンサデ－タをスペック範囲実質的制限無，十分な桁数とする．その分EEPROM使用量が多い．

/////////////////////////////////////////////////////////////////
//               積算温度計として使用する場合に設定
/////////////////////////////////////////////////////////////////
//#define SEKISAN  // 積算温度計として使う場合。一番目のセンサ出力を温度として処理

/////////////////////////////////////////////////////////////////
#ifdef SEKISAN
#define MENU_NO          5 // LCDでメニューを表示する場合のgDispModeの番号。0は基本画面表示
#else                      // 積算の場合は0-4が積算の切替で5が処理メニュー
#define MENU_NO          1 // 積算以外では基本画面(0)と処理メニュー画面(1)の２つ
#endif                     // 必要があれば、複数の別画面表示とすることも可能
                           // その場合はMENU_NOを2以降にずらし、
                           // 空いたgDispModeに対応する表示をlcdTime(), lcdData()等で定義する。


/////////////////////////////////////////////////////////////////
//                       ロギング条件の設定
/////////////////////////////////////////////////////////////////
// TIMER_INTERVAL, MEASURE_INTERVAL, LOG_INTERVALの設定について
// 設定できるのは　INTERVAL_UNIT が SEC_INTERVAL、MIN_INTERVALの時は60の約数、
// HOUR_INTERVALの時は24の約数
// それ以外の値を設定すると一定間隔ではなくなる（現在時刻が割り切れるかどうかで判定しているため）。
// 当然だが、TIMER_INTERVAL <= MEASURE_INTERVAL <= LOG_INTERVAL　の必要あり
// TIMER_INTERVALの設定が有効なのはINTERVAL_UNIT が SEC_INTERVALの時だけで、それ以外の時は設定にかかわらず1となる。
//
#define LOG_MODE ENDLESS_MODE            // 記録モード（ENDLESS_MODEかWRITE_ONCE_MODE）
//#define WRITE_ONCE_MODE                  // 記録モード（ENDLESS_MODEかWRITE_ONCE_MODE）
#define TIMER_INTERVAL    1              // タイマー割り込み間隔。有効なのはgIntervalUnit = SEC_INTERVALの時だけで、それ以外の時は1分
#define MEASURE_INTERVAL  1              // 測定間隔; 単位はINTERVAL_UNITで設定
#define LOG_INTERVAL     10              // 記録間隔; 前回記録以降の測定データの平均値を記録する。単位はINTERVAL_UNITで設定
//#define INTERVAL_UNIT    SEC_INTERVAL  // 上記の間隔の単位（秒）
#define INTERVAL_UNIT    MIN_INTERVAL    // 上記の間隔の単位（分）
//#define INTERVAL_UNIT    HOUR_INTERVAL // 上記の間隔の単位（時）

// other setting
#define LCD_ON_TIME      6 // turn off the LCD at LCD_OFF_TIME and turn on at LCD_ON_TIME to reduce electricity consumption
#define LCD_OFF_TIME    20 // you can turn on the LCD by pressing button even if the LCD is off


/////////////////////////////////////////////////////////////////
//           EEPROM書込み設定（既存の設定は基本的には変更不要）
//    新たなセンサライブラリを作った場合は設定の追加必要
/////////////////////////////////////////////////////////////////
// EM_DATA_PER_BUFF  //
///////////////////////
// バッファ内に一時保存するデータ数（EEPROM記憶用に変換したもの）
// 小さくするとEEPROMに書き込む回数が増えるため、EEPROMの書込回数制限上あまり小さい値は好ましくない。
// しかし、大きくするとその分ArduinoのRAMを消費する。Arduino wireライブラリを使う場合、書き込み時に
// 使えるバッファが30byteなので、基本的にはgEmDataBuffはこの30バイト以下とする。
// gEmDataBuffのサイズはemData_tのbyte数 x EM_DATA_PER_BUFF + 1

///////////////////////////////
// EM_BUFF_WRITE_PER_HEADER  //
///////////////////////////////
// バッファ数／ヘッダ書込
// ヘッダには8byte使うので、EM_BUFF_WRITE_PER_HEADERが大きい方がEEPROMに記憶できるデータが多くなるが、
// EEPROMメモリを一周したらヘッダ単位で前のデータが順に読み出せなくなる。
//
// たとえばEM_DATA_PER_BUFF = 12, EM_BUFF_WRITE_PER_HEADER = 12, ロギング間隔 = 10分の場合
// ヘッダの書き込み間隔は 12 x 12 x 10 = 1440分 = 24時間 = 1日
//
// 最大記録データ例1：上記の設定でemData_tが2バイト = 1データあたり2バイトの場合
// メモリ使用量はヘッダあたり（=1日あたり）2 x 12 x 12 + 8 = 296bytes. EEPROM 512Kbytesだと1771日
//
// 最大記録データ例2：emData_tが5バイト = 1データあたり5バイトで
// EM_DATA_PER_BUFF = 5, EM_BUFF_WRITE_PER_HEADER = 28, ロギング間隔 = 10分の場合
// メモリ使用量はヘッダあたり（=1日あたり）5 x 5 x 28 + 8 = 708bytes. EEPROM 512Kbytesだと740日
//
// ヘッダ以降の経過時間（gIntervalUnitの単位）をintで計算しているので、
// EM_DATA_PER_BUFF x EM_BUFF_WRITE_PER_HEADER x gLogIntervalがintの制限を超えないようにする。
// そんなに大きくすることはないだろうが。

///////////////////////////////////////
#ifdef SENSOR_NTC                    // サーミスタを使う場合
///////////////////////////////////////
#ifdef DUAL_SENSORS                  // サーミスタ2本の場合
#define EM_DATA_PER_BUFF          6  // emData_t が4バイトなので、gEmDataBuffは4 x 6 + 1 = 25バイト
#define EM_BUFF_WRITE_PER_HEADER 24  // ロギング間隔が10分では 10 x 6 x 24 = 1440分 = 1日
#else                                // サーミスタ1本の場合
#define EM_DATA_PER_BUFF         12  // emData_t が2バイトなので、gEmDataBuffは2 x 12 + 1 = 25バイト
#define EM_BUFF_WRITE_PER_HEADER 12  // ロギング間隔が10分では 10 x 12 x 12 = 1440分 = 1日
#endif // DUAL_SENSORS
#endif // SENSOR_NTC
///////////////////////////////////////
#ifdef SENSOR_BME280                 // BME280を使う場合
///////////////////////////////////////
#ifdef LQ_DATA                      // センサデータを制限してEEPROMを節約する場合
#ifdef DUAL_SENSORS                  // BME280を2個同時に使う場合
#define EM_DATA_PER_BUFF          4  // emData_t が6バイトなので、gEmDataBuffは6 x 4 + 1 = 25バイト
#define EM_BUFF_WRITE_PER_HEADER 36  // ロギング間隔が10分では 10 x 2 x 72 = 1440分 = 1日
#else                                // BME280を1個だけ使う場合
#define EM_DATA_PER_BUFF          9  // emData_t が3バイトなので、gEmDataBuffは3 x 9 + 1 = 28バイト
#define EM_BUFF_WRITE_PER_HEADER 16  // ロギング間隔が10分では 10 x 5 x 28 = 1400分 = 1日弱
#endif // DUAL_SENSORS
#endif // LQ_DATA

#ifdef SQ_DATA                      // センサデータを実用範囲でEEPROMに記録する場合
#ifdef DUAL_SENSORS                  // BME280を2個同時に使う場合
#define EM_DATA_PER_BUFF          3  // emData_t が8バイトなので、gEmDataBuffは8 x 3 + 1 = 25バイト
#define EM_BUFF_WRITE_PER_HEADER 48  // ロギング間隔が10分では 10 x 3 x 48 = 1440分 = 1日
#else                                // BME280を1個だけ使う場合
#define EM_DATA_PER_BUFF          7  // emData_t が4バイトなので、gEmDataBuffは4 x 7 + 1 = 29バイト
#define EM_BUFF_WRITE_PER_HEADER 20  // ロギング間隔が10分では 10 x 7 x 20 = 1400分 = 1日弱
#endif // DUAL_SENSORS
#endif // SQ_DATA

#ifdef HQ_DATA                      // センサデータを事実上制限無しに保存する場合
#ifdef DUAL_SENSORS                  // BME280を2個同時に使う場合
#define EM_DATA_PER_BUFF          2  // emData_t が10バイトなので、gEmDataBuffは10 x 2 + 1 = 21バイト
#define EM_BUFF_WRITE_PER_HEADER 72  // ロギング間隔が10分では 10 x 2 x 72 = 1440分 = 1日
#else                                // BME280を1個だけ使う場合
#define EM_DATA_PER_BUFF          5  // emData_t が5バイトなので、gEmDataBuffは5 x 5 + 1 = 26バイト
#define EM_BUFF_WRITE_PER_HEADER 28  // ロギング間隔が10分では 10 x 5 x 28 = 1400分 = 1日弱
#endif // DUAL_SENSORS
#endif // HQ_DATA
#endif // SENSOR_BME280
///////////////////////////////////////
#ifdef SENSOR_SHT
///////////////////////////////////////
#ifdef LQ_DATA                      // センサデータの範囲，桁数を制限して保存し，EEPROM使用量を節約する場合
#ifdef DUAL_SENSORS                  // SHT2x/3xを2個同時に使う場合
#define EM_DATA_PER_BUFF          6  // emData_t が4バイトなので、gEmDataBuffは4 x 6 + 1 = 25バイト
#define EM_BUFF_WRITE_PER_HEADER 24  // ロギング間隔が10分では 10 x 6 x 24 = 1440分 = 1日
#else                                // SHT2x/3xを1個だけ使う場合
#define EM_DATA_PER_BUFF         12  // emData_t が2バイトなので、gEmDataBuffは2 x 12 + 1 = 25バイト
#define EM_BUFF_WRITE_PER_HEADER 12  // ロギング間隔が10分では 10 x 12 x 12 = 1440分 = 1日
#endif // DUAL_SENSORS
#endif // LQ_DATA

#ifdef HQ_DATA                      // センサデータを事実上制限無しに保存する場合
#ifdef DUAL_SENSORS                  // SHT2x/3xを2個同時に使う場合
#define EM_DATA_PER_BUFF          4  // emData_t が6バイトなので、gEmDataBuffは6 x 4 + 1 = 25バイト
#define EM_BUFF_WRITE_PER_HEADER 36  //  ロギング間隔が10分では 10 x 4 x 36 = 1440分 = 1日
#else                                // SHT2x/3xを1個だけ使う場合
#define EM_DATA_PER_BUFF          9  // emData_t が3バイトなので、gEmDataBuffは3 x 9 + 1 = 28バイト
#define EM_BUFF_WRITE_PER_HEADER 16  //  ロギング間隔が10分では 10 x 9 x 16 = 1440分 = 1日
#endif // DUAL_SENSORS
#endif // HQ_DATA
#endif // SENSOR_SHT


/////////////////////////////////////////////////////////////////
//           ハードウェア等設定（ハードウェアに応じて設定）
/////////////////////////////////////////////////////////////////
// set hardware pin asignments
#define ALARM_INT_PIN       2 // alarm interrupt input pin for periodical measurement
#define MANUAL_INT_PIN      3 // manual interrupt input pin to call menu etc.
#define CHIP_SELECT        10 // SDカードアクセス用(SPI)
// connect push button between GND and MANUAL_INT_PIN with pull-up resistor
#define THERMISTOR_EXC      9 // サーミスタ加電圧ピン（省電力のため、測定時のみ加電圧）
#define THERMISTOR_ADC1     3 // サーミスタ測定ピン１
#define THERMISTOR_ADC2     2 // サーミスタ測定ピン１
#define THERMISTOR_DVR1 10000 // サーミスタ分圧抵抗1の正確な抵抗値（Ω）
#define THERMISTOR_DVR2 10000 // サーミスタ分圧抵抗2の正確な抵抗値（Ω）


/////////////////////////////////////////////////////////////////
//           8. 設定内容保存EEPROM（内部）アドレス設定
/////////////////////////////////////////////////////////////////
// set EEPROM address  AVR内蔵EEPROMのアドレス
#define BACKUP_DATA_ADDRESS    0 // 積算温度、積算開始日等のバックアップデータ保存用
#define LOGGER_ID_ADDRESS   0x80 // ロガーIDの保存用
#define DV_R_ADDRESS        0x90 // サーミス分圧抵抗値の保存用


/////////////////////////////////////////////////////////////////
//                 起動時の時刻設定
/////////////////////////////////////////////////////////////////
//#define REBOOT_TIME_SET          // 起動時に時刻の設定を行なう


/////////////////////////////////////////////////////////////////
//                 その他設定（基本的に変更はしない）
/////////////////////////////////////////////////////////////////
#define EM_FORMAT_MARK           0xFF  // 外部EEPROM内の未使用領域マーク
#define EM_HEADER_MARK           0xFE  // 外部EEPROM内のデータヘッダ識別マーク
#define EM_NULLDATA_MARK         0xFD  // 外部EEPROM内のデータ初期化マーク；これでデータ記録済みか判断

#endif
