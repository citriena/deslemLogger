#ifndef _deslemLoggerConfig_h
#define _deslemLoggerConfig_h


// 使用状況等で変更する設定、複数のソースファイルで共通の設定

//#define SET_ID "Logger08"      // ロガーID（英数13文字以内） ロガーのIDを設定、変更するとき以外はコメントアウトする。
//#define THERMISTOR_DVR1  10003 // サーミスタ分圧抵抗1の正確な抵抗値
//#define THERMISTOR_DVR2  10003 // サーミスタ分圧抵抗2の正確な抵抗値


/////////////////////////////////////////////////////////////////
//                       使用するセンサ等の設定
// 　　　　　　　　　　　　　いずれか一つだけ有効にする
/////////////////////////////////////////////////////////////////
//
//#define NTC  // サーミスタを使う場合。サーミスタの特性はsensorNTC.cpp内のコンストラクタで設定

// SHT-31/35, SHT-85等を使う場合 秋月電子通商のライブラリ使用。AE_SHT31だとクラス名と同じためコンパイルエラーとなる。
#define SHT3X_A1      // 秋月電子通商のAE-SHT31のデフォルト（4番ピンオープン）。GY-SHT31-DはAD端子をVddにつなぐ。
//#define SHT3X_A2      // 秋月電子通商のAE-SHT31では4番ピンをGNDに接続  GY-SHT31-Dのデフォルト。SHT-85はこちらのみ

// SHT-31/35, SHT-85等を使う場合　Sensirionのライブラリ使用。純正だがフラッシュメモリ使用量が大きい。
//#define SHT3X_S1     // 秋月電子通商のAE-SHT31では4番ピンをGNDに接続  GY-SHT31-Dのデフォルト。SHT-85はこちらのみ
//#define SHT3X_S2     // 秋月電子通商のAE-SHT31のデフォルト（4番ピンオープン）。GY-SHT31-DはAD端子をVddにつなぐ。

//#define SHT2X_R      // SHT-21を使う場合。 SHT21だとクラス名と同じのためコンパイルエラーとなる。
//#define HTU21        // HTU21を使う場合。実はSHT-21と同じライブラリが使えるようだ。

/////////////////////////////////////////////////////////////////
#if defined(SHT3X_A1) || defined(SHT3X_A2) || defined(SHT3X_S1) || defined(SHT3X_S2) || defined(SHT2X_R) || defined(HTU21)
#define SHT  // Sensirion SHT-21, SHT-31, SHT-85 やHTU21を使う場合の共通定義
#endif
/////////////////////////////////////////////////////////////////
//                       使用するセンサ等の設定
// 　　　　　　　　　　　　　　必要に応じて有効にする
/////////////////////////////////////////////////////////////////

//#define SEKISAN  // 積算温度計として使う場合。一番目のセンサ出力を温度として処理
//#define DUAL_SENSORS // サーミスタ、もしくはSHT-31等でセンサを２つ使う場合。SHT-31はモジュールのI2Cアドレスを別にする必要あり。
                       // SHT-21はアドレス固定なので、同一バスで複数センサの使用は不可（方法はあるが面倒）

// #define ECO_DATA   // SHTセンサでEEPROM用に変換するデータを温湿度で２バイト／１センサにする。指定しないと３バイト; 違いは
// バイト数が少ないので、以下の制限がある。
// 温度：-9.9～52.7℃（0.1℃単位）
// 湿度：1～100%（1%単位）
// 測定エラー時は-10℃、0%を返す。
// ECO_DATAを設定しない場合は、
// 温度：-40.0℃～125.0℃（0.1℃単位）
// 湿度：0.1%～100.0%（0.1%単位）
// 測定エラー時は150℃、0%を返す。

/////////////////////////////////////////////////////////////////
//                       ロギング条件の設定
/////////////////////////////////////////////////////////////////
// TIMER_INTERVAL, MEASURE_INTERVAL, LOG_INTERVALの設定について
// 設定できるのは　INTERVAL_UNIT が SEC_INTERVAL、MIN_INTERVALの時は60の約数、
// HOUR_INTERVALの時は24の約数
// それ以外の値を設定すると一定間隔ではなくなる（現在時刻が割り切れるかで判定しているため）。
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
//                   動作設定（状況に応じ設定変更）
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
// メモリを一周したらヘッダ単位で前のデータが順に読み出せなくなる。
// この設定でロギング間隔が10分の場合、ヘッダの書き込み間隔は 10x12x12=1440分=24時間=1日
// メモリ使用量はヘッダあたり（=1日あたり）2x12x12+8=296bytes。EEPROM 512Kbytesだと1771日

// ヘッダ以降の経過時間（gIntervalUnitの単位）をintで計算しているので、
// EM_DATA_PER_BUFF x EM_BUFF_WRITE_PER_HEADER x gLogIntervalがintの制限を超えないようにする。
// そんなに大きくすることはないだろうが。

#ifdef NTC
#ifdef DUAL_SENSORS
#define EM_DATA_PER_BUFF          6  // emData_t が4バイトなので、gEmDataBuffは4x6+1=25バイト
#define EM_BUFF_WRITE_PER_HEADER 24  // 
#else
#define EM_DATA_PER_BUFF         12  // emData_t が2バイトなので、gEmDataBuffは2x12+1=25バイト
#define EM_BUFF_WRITE_PER_HEADER 12  // 
#endif
#endif

#ifdef SHT
#ifdef DUAL_SENSORS
#ifdef ECO_DATA
#define EM_DATA_PER_BUFF          6  // emData_t が4バイトなので、gEmDataBuffは4x6+1=25バイト
#define EM_BUFF_WRITE_PER_HEADER 24  // 
#else
#define EM_DATA_PER_BUFF          4  // emData_t が6バイトなので、gEmDataBuffは6x4+1=25バイト
#define EM_BUFF_WRITE_PER_HEADER 36  // 
#endif
#else
#ifdef ECO_DATA
#define EM_DATA_PER_BUFF         12  // emData_t が2バイトなので、gEmDataBuffは2x12+1=25バイト
#define EM_BUFF_WRITE_PER_HEADER 12  // 
#else
#define EM_DATA_PER_BUFF          9  // emData_t が3バイトなので、gEmDataBuffは3x9+1=28バイト
#define EM_BUFF_WRITE_PER_HEADER 16  // 
#endif
#endif
#endif


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
//                 設定内容保存アドレス
/////////////////////////////////////////////////////////////////
// set EEPROM address  AVR内蔵EEPROMのアドレス
#define BACKUP_DATA_ADDRESS    0 // 積算温度、積算開始日等のバックアップデータ保存用。実際に有効な定義はsekisan.h内
#define LOGGER_ID_ADDRESS   0x80 // ロガーIDの保存用
#define DV_R_ADDRESS        0x90 // サーミス分圧抵抗値の保存用


/////////////////////////////////////////////////////////////////
//                 その他設定（基本的に変更はしない）
/////////////////////////////////////////////////////////////////
#define EM_FORMAT_MARK           0xFF  // 外部EEPROM内の未使用領域マーク
#define EM_HEADER_MARK           0xFE  // 外部EEPROM内のデータヘッダ識別マーク
#define EM_NULLDATA_MARK         0xFD  // 外部EEPROM内のデータ初期化マーク；これでデータ記録済みか判断

#endif
