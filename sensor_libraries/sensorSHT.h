////////////////////////////////////////////////////////////////
// delemLogger.ino 用のSensirion SHT2x/3x センサライブラリ
// sensorSHT.h ver.1.01 by citriena
// delemLogger.inoと同じフォルダに置く。
// 複数のセンサライブラリがある場合，使わないライブラリは別のフォルダに移動しておく。
// 同じフォルダに置いておくとコンパイルエラーとなる。
// センサからデータを読み出すライブラリは別途必要．sensorSHT.cpp参照
////////////////////////////////////////////////////////////////

#ifndef _sensorSHT_h
#define _sensorSHT_h
#include <Arduino.h>
#include "deslemLoggerConfig.h"

////////////////////////////////////////////////////////////////
// 　型宣言 + 付属広域変数宣言　センサー依存部
/////////////////////////////////////////////////////////////////

#define NULLDATA_MARK           -9999  // センサーエラー時等に返す値　必要に応じて変える

// センサから読み出すデータの型宣言
// 処理の関係上、メンバ名は変えない。センサの出力が複数の場合はdt1a, dt1b, dt1cとアルファベットを進める
// 複数のセンサを使う場合はdt2a, dt2b と番号を進める。
typedef struct {
  float dt1a;  // 温度
  float dt1b;  // 湿度
#ifdef DUAL_SENSORS
  float dt2a;  // 温度
  float dt2b;  // 湿度
#endif
} data_t;


// EEPROMへ書込むデータの型宣言
// センサのデータを適切に変換し、データを記憶するEEPROMメモリを節約
// 処理の関係上データ型はbyteのみとする。メンバ名の規則も上と同様だが、最後のアルファベットはセンサの出力と対応する必要は無い。

#ifdef SQ_DATA

typedef struct {
  byte data1a;
  byte data1b;
#ifdef DUAL_SENSORS
  byte data2a;
  byte data2b;
#endif
} emData_t;

#else

typedef struct {
  byte data1a;
  byte data1b;
  byte data1c;
#ifdef DUAL_SENSORS
  byte data2a;
  byte data2b;
  byte data2c;
#endif
} emData_t;

#endif

#ifndef DUAL_SENSORS
const data_t nullData = {NULLDATA_MARK, NULLDATA_MARK};
#else
const data_t nullData = {NULLDATA_MARK, NULLDATA_MARK, NULLDATA_MARK, NULLDATA_MARK};
#endif

#ifdef SQ_DATA
#ifndef DUAL_SENSORS
const emData_t nullEmData = {EM_NULLDATA_MARK, EM_NULLDATA_MARK};  // EEPROMに書き込むバッファの初期化はEM_NULLDATA_MARK;これでデータ書込済かどうか識別する
#else
const emData_t nullEmData = {EM_NULLDATA_MARK, EM_NULLDATA_MARK, EM_NULLDATA_MARK, EM_NULLDATA_MARK};
#endif
#else
#ifndef DUAL_SENSORS
const emData_t nullEmData = {EM_NULLDATA_MARK, EM_NULLDATA_MARK, EM_NULLDATA_MARK};  // EEPROMに書き込むバッファの初期化はEM_NULLDATA_MARK;これでデータ書込済かどうか識別する
#else
const emData_t nullEmData = {EM_NULLDATA_MARK, EM_NULLDATA_MARK, EM_NULLDATA_MARK, EM_NULLDATA_MARK, EM_NULLDATA_MARK, EM_NULLDATA_MARK};
#endif
#endif


void initSensor();
data_t getData();
data_t avgData();
emData_t setEmData(data_t tData);
data_t restoreEmData(emData_t tEmData);

#endif
