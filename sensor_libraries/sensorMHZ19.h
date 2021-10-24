//////////////////////////////////////////////////////
// 電池長期間駆動Arduinoロガー
// deslemLogger (deep sleep EEPROM logger)用補助スケッチ
// CO2センサー MHZ19 を使う場合のセンサー処理ライブラリ
//////////////////////////////////////////////////////
#ifndef _sensorMHZ19_h
#define _sensorMHZ19_h
#include <Arduino.h>
#include "deslemLoggerConfig.h"


////////////////////////////////////////////////////////////////
// 　型宣言 + 付属広域変数宣言　センサー依存部
/////////////////////////////////////////////////////////////////
#define NULLDATA_MARK           -9999  // センサーエラー時等に返す値　必要に応じて変える
#define ERR_DATA                -9999                

// センサから読み出すデータの型宣言
// 必須ではないが処理の関係上変数名はなるべく変えない。１センサの出力が増える場合はdt1a, dt1bとアルファベットを進める。
// センサの本数が増える場合は dt2a, dt3a, dt4a と続ける。
typedef struct {
  int dt1a;
} data_t;


// EEPROMへ書込むデータの型宣言
// センサのデータを適切に変換し、メモリ節約
// 処理の関係上データ型はbyteのみとし、増やす場合はdata1a, data1b と続ける（少なくとも最初はdata1a）。
// センサの本数が増える場合は data2a, data2bと続ける。いずれ配列に変更したい。
typedef struct {
  byte data1a;
  byte data1b;
} emData_t;

const emData_t nullEmData = {EM_NULLDATA_MARK, EM_NULLDATA_MARK};
const data_t nullData = {NULLDATA_MARK};

void initSensor();
data_t getData();
data_t avgData();
emData_t setEmData(data_t tData);
data_t restoreEmData(emData_t tEmData);
void getDivR();

#endif
