#include "deslemLoggerConfig.h"
#ifdef SENSOR_NTC
//////////////////////////////////////////////////////
// 電池長期間駆動Arduinoロガー
// deslemLogger (deep sleep EEPROM logger)用補助スケッチ
// サーミスタを使う場合のセンサー処理ライブラリ
//////////////////////////////////////////////////////
#ifndef _sensorNTC_h
#define _sensorNTC_h
#include <Arduino.h>


////////////////////////////////////////////////////////////////
// 　型宣言 + 付属広域変数宣言　センサー依存部
/////////////////////////////////////////////////////////////////
#define NULLDATA_MARK           -9999  // センサーエラー時等に返す値　必要に応じて変える


// センサから読み出すデータの型宣言
// 必須ではないが処理の関係上変数名はなるべく変えない。１センサの出力が増える場合はdt1a, dt1bとアルファベットを進める。
// センサの本数が増える場合は dt2a, dt3a, dt4a と続ける。
typedef struct {
  float dt1a;
#ifdef DUAL_SENSORS
  float dt2a;
#endif
} data_t;


// EEPROMへ書込むデータの型宣言
// センサのデータを適切に変換し、メモリ節約
// 処理の関係上データ型はbyteのみとし、増やす場合はdata1a, data1b と続ける（少なくとも最初はdata1a）。
// センサの本数が増える場合は data2a, data2bと続ける
typedef struct {
  byte data1a;
  byte data1b;
#ifdef DUAL_SENSORS
  byte data2a;
  byte data2b;
#endif
} emData_t;

#ifndef DUAL_SENSORS
const emData_t nullEmData = {EM_NULLDATA_MARK, EM_NULLDATA_MARK};  // EEPROMに書き込むバッファの初期化はEM_NULLDATA_MARK;これでデータ書込済かどうか識別する
const data_t nullData = {NULLDATA_MARK};
#else
const emData_t nullEmData = {EM_NULLDATA_MARK, EM_NULLDATA_MARK, EM_NULLDATA_MARK, EM_NULLDATA_MARK};
const data_t nullData = {NULLDATA_MARK, NULLDATA_MARK};
#endif

void initSensor();
data_t getData();
data_t avgData();
emData_t setEmData(data_t tData);
data_t restoreEmData(emData_t tEmData);
void getDivR();

#endif

#endif // SENSOR_NTC
