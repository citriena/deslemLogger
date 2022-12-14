#include "deslemLoggerConfig.h"
#ifdef SENSOR_BME280
////////////////////////////////////////////////////////////////
// delemLogger.ino 用のBosch BME280センサライブラリ
// sensorBME280.h ver. 1.00 by citriena
// delemLogger.inoと同じフォルダに置く。
// 複数のセンサライブラリがある場合は使わないライブラリは別のフォルダに移動しておく。
// 同じフォルダに置いておくとコンパイルエラーとなる。
// センサからデータを読み出すライブラリは別途必要．sensorBME280.cpp参照
////////////////////////////////////////////////////////////////

#ifndef _sensorBME280_h
#define _sensorBME280_h

#include <Arduino.h>
#include "deslemLoggerConfig.h"


////////////////////////////////////////////////////////////////
// 　型宣言 + 付属広域変数宣言　センサー依存部
/////////////////////////////////////////////////////////////////
#define NULLDATA_MARK           -9999  // センサーエラー時等に返す値　必要に応じて変える

// センサから読み出すデータの型宣言
// 必須ではないが処理の関係上変数名はなるべく変えない。１センサの出力が増える場合はdt1a, dt1bとアルファベットを進める。
// センサの本数が増える場合は dt2a, dt3a, dt4a と続ける。複数センサを配列で処理すればプログラムが簡単になるが、同一データの
// センサしか複数扱えなくなる。異なるセンサも一緒に扱えるようにするため、あえて配列は使っていない。
typedef struct {
  float dt1a;
  float dt1b;
  float dt1c;
#ifdef DUAL_SENSORS
  float dt2a;
  float dt2b;
  float dt2c;
#endif
} data_t;

// EEPROMへ書込むデータの型宣言
// センサのデータを適切に変換し、データを記憶するEEPROMメモリを節約
// 処理の関係上データ型はbyteのみとする。メンバ名の規則も上と同様だが、最後のアルファベットはセンサの出力と対応する必要は無い。

#ifdef LQ_DATA
typedef struct {
  byte data1a;
  byte data1b;
  byte data1c;
#ifdef DUAL_SENSORS
  byte data2a;
  byte data2b;
  byte data2c;
#endif  // DUAL_SENSORS
} emData_t;
#endif // LQ_DATA

#ifdef SQ_DATA // 
typedef struct {
  byte data1a;
  byte data1b;
  byte data1c;
  byte data1d;
#ifdef DUAL_SENSORS
  byte data2a;
  byte data2b;
  byte data2c;
  byte data2d;
#endif // DUAL_SENSORS
} emData_t;
#endif  // SQ_DATA

#ifdef HQ_DATA //
typedef struct {
  byte data1a;
  byte data1b;
  byte data1c;
  byte data1d;
  byte data1e;
#ifdef DUAL_SENSORS
  byte data2a;
  byte data2b;
  byte data2c;
  byte data2d;
  byte data2e;
#endif // DUAL_SENSORS
} emData_t;
#endif // HQ_DATA


#ifndef DUAL_SENSORS
const data_t nullData = {NULLDATA_MARK, NULLDATA_MARK, NULLDATA_MARK};
#else  // DUAL_SENSORS
const data_t nullData = {NULLDATA_MARK, NULLDATA_MARK, NULLDATA_MARK, NULLDATA_MARK, NULLDATA_MARK, NULLDATA_MARK};
#endif // DUAL_SENSORS


#ifdef LQ_DATA
#ifndef DUAL_SENSORS
const emData_t nullEmData = {EM_NULLDATA_MARK, EM_NULLDATA_MARK, EM_NULLDATA_MARK};  // EEPROMに書き込むバッファの初期化はEM_NULLDATA_MARK;これでデータ書込済かどうか識別する
#else  // DUAL_SENSORS
const emData_t nullEmData = {EM_NULLDATA_MARK, EM_NULLDATA_MARK, EM_NULLDATA_MARK, EM_NULLDATA_MARK, EM_NULLDATA_MARK, EM_NULLDATA_MARK};
#endif // DUAL_SENSORS
#endif // LQ_DATA


#ifdef SQ_DATA // LQ_DATA
#ifndef DUAL_SENSORS
const emData_t nullEmData = {EM_NULLDATA_MARK, EM_NULLDATA_MARK, EM_NULLDATA_MARK, EM_NULLDATA_MARK};  // EEPROMに書き込むバッファの初期化はEM_NULLDATA_MARK;これでデータ書込済かどうか識別する
#else  // DUAL_SENSORS
const emData_t nullEmData = {EM_NULLDATA_MARK, EM_NULLDATA_MARK, EM_NULLDATA_MARK, EM_NULLDATA_MARK, EM_NULLDATA_MARK, EM_NULLDATA_MARK, EM_NULLDATA_MARK, EM_NULLDATA_MARK};
#endif // DUAL_SENSORS
#endif // SQ_DATA


#ifdef HQ_DATA
#ifndef DUAL_SENSORS
const emData_t nullEmData = {EM_NULLDATA_MARK, EM_NULLDATA_MARK, EM_NULLDATA_MARK, EM_NULLDATA_MARK, EM_NULLDATA_MARK};
#else  // DUAL_SENSORS
const emData_t nullEmData = {EM_NULLDATA_MARK, EM_NULLDATA_MARK, EM_NULLDATA_MARK, EM_NULLDATA_MARK, EM_NULLDATA_MARK, EM_NULLDATA_MARK, EM_NULLDATA_MARK, EM_NULLDATA_MARK, EM_NULLDATA_MARK, EM_NULLDATA_MARK};
#endif // DUAL_SENSORS
#endif // HQ_DATA


void initSensor();
data_t getData();
data_t avgData();
emData_t setEmData(data_t tData);
data_t restoreEmData(emData_t tEmData);

#endif

#endif //SENSOR_BME280
