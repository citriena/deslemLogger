#include <Arduino.h>
#include <Wire.h>
#include <forcedClimate.h>   // https://github.com/JVKran/Forced-BME280
#include "sensorBME280.h"

#ifdef BME280_x76
ForcedClimate BME280_1 = ForcedClimate(Wire, 0x76);
#endif

#ifdef BME280_x77
#ifdef BME280_x76
ForcedClimate BME280_2 = ForcedClimate(Wire, 0x77);
#else
ForcedClimate BME280_1 = ForcedClimate(Wire, 0x77);
#endif
#endif

uint8_t _dataCount1a = 0;
uint8_t _dataCount1b = 0;
uint8_t _dataCount1c = 0;
#ifdef DUAL_SENSORS
uint8_t _dataCount2a = 0;
uint8_t _dataCount2b = 0;
uint8_t _dataCount2c = 0;
data_t _sumData = {0, 0, 0, 0, 0, 0};  // 平均値計算用
#else
data_t _sumData = {0, 0, 0};           // 平均値計算用
#endif

/////////////////////////////////////////////////////////////////
//                  センサーデータ処理関係
/////////////////////////////////////////////////////////////////

void initSensor() {
  BME280_1.begin();
#ifdef DUAL_SENSORS
  BME280_2.begin();
#endif
}


//read data from Sensor
data_t getData() {

  data_t tData;

  BME280_1.takeForcedMeasurement();
  tData.dt1a = BME280_1.getTemperatureCelcius();
  if (tData.dt1a != NULLDATA_MARK) {
    _sumData.dt1a += tData.dt1a;  // 平均用の処理も行う。
    _dataCount1a++;
  }
  tData.dt1b = BME280_1.getRelativeHumidity();
  if (tData.dt1b != NULLDATA_MARK) {
    _sumData.dt1b += tData.dt1b;  // 平均用の処理も行う。
    _dataCount1b++;
  }
  tData.dt1c = BME280_1.getPressure();
  if (tData.dt1c != NULLDATA_MARK) {
    _sumData.dt1c += tData.dt1c;  // 平均用の処理も行う。
    _dataCount1c++;
  }
#ifdef DUAL_SENSORS
  BME280_2.takeForcedMeasurement();
  tData.dt2a = BME280_2.getTemperatureCelcius();
  if (tData.dt2a != NULLDATA_MARK) {
    _sumData.dt2a += tData.dt2a;  // 平均用の処理も行う。
    _dataCount2a++;
  }
  tData.dt2b = BME280_2.getRelativeHumidity();
  if (tData.dt2b != NULLDATA_MARK) {
    _sumData.dt2b += tData.dt2b;  // 平均用の処理も行う。
    _dataCount2b++;
  }
  tData.dt2c = BME280_2.getPressure();
  if (tData.dt2c != NULLDATA_MARK) {
    _sumData.dt2c += tData.dt2c;  // 平均用の処理も行う。
    _dataCount2c++;
  }
#endif
  return tData;
}


data_t avgData() {

  data_t avgData;

  if (_dataCount1a > 0) {
    avgData.dt1a = _sumData.dt1a / _dataCount1a;
  }
  if (_dataCount1b > 0) {
    avgData.dt1b = _sumData.dt1b / _dataCount1b;
  }
  if (_dataCount1c > 0) {
    avgData.dt1c = _sumData.dt1c / _dataCount1c;
  }
  _dataCount1a = 0;           // 平均を返したら平均処理用の変数をリセット
  _dataCount1b = 0;           // 平均を返したら平均処理用の変数をリセット
  _dataCount1c = 0;           // 平均を返したら平均処理用の変数をリセット
#ifdef DUAL_SENSORS
  if (_dataCount2a > 0) {
    avgData.dt2a = _sumData.dt2a / _dataCount2a;
  }
  if (_dataCount2b > 0) {
    avgData.dt2b = _sumData.dt2b / _dataCount2b;
  }
  if (_dataCount2c > 0) {
    avgData.dt2c = _sumData.dt2c / _dataCount2c;
  }
  _dataCount2a = 0;           // 平均を返したら平均処理用の変数をリセット
  _dataCount2b = 0;           // 平均を返したら平均処理用の変数をリセット
  _dataCount2c = 0;           // 平均を返したら平均処理用の変数をリセット
  _sumData = {0, 0, 0, 0, 0, 0};
#else
  _sumData = {0, 0, 0};
#endif
  return avgData;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef LQ_DATA  // センサ一つあたり
//////////////////////////////////////////////////////////////////////////////////////////////////////////
// LQ_DATAが設定されている場合
// EEPROM使用量節約のために温度、湿度，気圧を3バイトで保存する。
// 
// LQ_DATAはバイト数が少ないので、以下の制限がある。
// 温度：-10.0～68.0℃（0.1℃単位；エラー含めて782カウント）
// 湿度：5～100%（1%単位; エラー含めて97カウント）
// 気圧：850～1050hPa (1hPa単位；エラー含めて202カウント)
// 測定エラー時は68.1℃、101%, 1051hPaを返す。

// 温度:tp, 湿度rh, ap とする。
// 温度変換値 ct = (tp - 10) * 10
// で-10.0~68.0℃を0~780の整数に変換（781カウント）
// 湿度変換値 ch = rh - 5
// で5-100%を0-95の整数に変換（96カウント）
// 気圧変換値 cp = ap - 850
// で850-1050hPaを0-200の整数に変換（201カウント）
// tDt = (ct * 101 * 202) + (ch * 202) + cp
// とすると上記の温度，湿度，気圧は
// 0 <= cDt <= 15,332,508 < 15,813,251（0-251の3バイト最大値）
// で表現できる。
////////////////////////////////////////////////
emData_t setEmData(data_t tData) {   // センサデータをEEPROM記憶用に変換

  emData_t tEmData;
  unsigned long tDt;                   // tDtは0-16,003,007とする必要がある。
  unsigned long tDta;                  // tDtaは0-63503とする必要がある。
  unsigned long tDtb;                  // tDtaは0-63503とする必要がある。

  if (tData.dt1a == NULLDATA_MARK) { // 複数データがある場合は個別に処理
    tData.dt1a = 68.1;                 // エラー時は68.1℃
  } else if (tData.dt1a > 68.0) {     // emData変換最大値
    tData.dt1a = 68.0;
  } else if (tData.dt1a < -10) {
    tData.dt1a = -10;
  }
  if (tData.dt1b == NULLDATA_MARK) { // 複数データがある場合は個別に処理
    tData.dt1b = 101;                   // 湿度エラー時は101%
  } else if (tData.dt1b > 100) {
    tData.dt1b = 100;
  } else if (tData.dt1b < 0) {
    tData.dt1b = 0;
  }
  if (tData.dt1c == NULLDATA_MARK) { // 複数データがある場合は個別に処理
    tData.dt1c = 1051;                 // エラー時は1050hPa
  } else if (tData.dt1c > 1050) {     // emData変換最大値
    tData.dt1c = 1050;
  } else if (tData.dt1c < 850) {
    tData.dt1c = 850;
  }
  // 整数変換時に生じた誤差は切り捨てられるので、正しく整数変換＋四捨五入されるように補正するため0.5，0.05加算している。
  tDt = (unsigned long)((tData.dt1a  + 10.05) * 10) * (97 * 202) + (unsigned long)(tData.dt1b - 5.0 + 0.5) * 202 + (unsigned long)(tData.dt1c - 850 + 0.5);
  tDta = tDt / 252;
  tDtb = tDt % 252;
  tEmData.data1a = tDta / 252; // tEmData.data1a, data1b, data1cは0-251まで
  tEmData.data1b = tDta % 252;
  tEmData.data1c = tDtb;

#ifdef DUAL_SENSORS

  if (tData.dt2a == NULLDATA_MARK) { // 複数データがある場合は個別に処理
    tData.dt2a = 68.1;                 // エラー時は-10℃
  } else if (tData.dt2a > 68.0) {     // emData変換最大値
    tData.dt2a = 68.0;
  } else if (tData.dt2a < -10.0) {
    tData.dt2a = -10.0;
  }
  if (tData.dt2b == NULLDATA_MARK) { // 複数データがある場合は個別に処理
    tData.dt2b = 100.1;                   // 湿度エラー時は100.1%
  } else if (tData.dt2b > 100) {
    tData.dt2b = 100;
  } else if (tData.dt2b < 0) {
    tData.dt2b = 0;
  }
  if (tData.dt2c == NULLDATA_MARK) { // 複数データがある場合は個別に処理
    tData.dt2c = 1051;                 // エラー時は1050hPa
  } else if (tData.dt2c > 1050) {     // emData変換最大値
    tData.dt1a = 1050;
  } else if (tData.dt2c < 850) {
    tData.dt2c = 850;
  }
  // 整数変換時に生じた誤差は切り捨てられるので、正しく整数変換＋四捨五入されるように補正するため0.5，0.05加算している。
  tDt = (unsigned long)((tData.dt2a  + 10.05) * 10) * (97 * 202) + (unsigned long)(tData.dt2b - 5.0 + 0.5) * 202 + (unsigned long)(tData.dt2c - 850 + 0.5);
  tDta = tDt / 252;
  tDtb = tDt % 252;
  tEmData.data2a = tDta / 252; // tEmData.data2a, data2b, data2cは0-251まで
  tEmData.data2b = tDta % 252;
  tEmData.data2c = tDtb;
#endif
  return tEmData;
}


data_t restoreEmData(emData_t tEmData) { // EEPROM内の変換データを元に戻す。
  data_t tData;
  unsigned int tDta;
  unsigned int tDtb;
  
  if (tEmData.data1a == EM_NULLDATA_MARK) {
    tData = nullData;
  } else {
    tDta = (unsigned long)tEmData.data1a * 252 * 252 + (unsigned long)tEmData.data1b * 252 + (unsigned long)tEmData.data1c;
    tDtb = tDta / 202;
    tDta = tDta % 202;
    tData.dt1c = tDta + 850;
    if (tData.dt1c == 1051) tData.dt1c = NULLDATA_MARK;
    tDta = tDtb / 97;
    tDtb = tDtb % 97;
    tData.dt1b = tDtb + 5.0;
    if (tData.dt1b == 101) tData.dt1b = NULLDATA_MARK;
    tData.dt1a = (float)tDta / 10 - 10.0;
    if (tData.dt1a == 68.1) tData.dt1a = NULLDATA_MARK;
  }

#ifdef DUAL_SENSORS
  if (tEmData.data2a == EM_NULLDATA_MARK) {
    tData = nullData;
  } else {
    tDta= (unsigned long)tEmData.data2a * 252 * 252 + (unsigned long)tEmData.data2b * 252 + (unsigned long)tEmData.data2c;
    tDtb = tDta / 202;
    tDta = tDta % 202;
    tData.dt2c = tDta + 850;
    if (tData.dt2c == 1051) tData.dt2c = NULLDATA_MARK;
    tDta = tDtb / 97;
    tDtb = tDtb % 97;
    tData.dt2b = tDtb + 5.0;
    if (tData.dt2b == 101) tData.dt2b = NULLDATA_MARK;
    tData.dt2a = (float)tDta / 10 - 10.0;
    if (tData.dt2a == 68.1) tData.dt2a = NULLDATA_MARK;
  }
#endif
  return tData;
}
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef SQ_DATA
//////////////////////////////////////////////////////////////////////////////////////////////////////////
// 以下のデータを4バイトで保存する。以下のカウントはエラーも含めての値
// 温度：-40.0℃～83.0℃ (1232カウント)
// 湿度：0.0%～100.0% (1002カウント)
// 気圧：760.0～1080.0hPa (3202カウント)
// と仕様上のセンサの出力を大きな制限無く保存できる。すなわち
// 0 <= cDt <= 3,952,753,728 < 3,969,126,001（0-251の4バイト最大値）
// で表現できる。
////////////////////////////////////////////////
emData_t setEmData(data_t tData) {   // センサデータをEEPROM記憶用に変換

  emData_t tEmData;                    // EEPROM内のバイトデータには処理上の都合により 0xFC, 0xFD, 0xFE, 0xFF が使えないので、
  unsigned long tDta;                  //
  unsigned long tDtb;                  //

  if (tData.dt1a == NULLDATA_MARK) {
    tData.dt1a = 83.1;                 // エラー時は 83.1℃
  } else if (tData.dt1a > 83.0) {
    tData.dt1a = 83.0;
  } else if (tData.dt1a < -40.0) {
    tData.dt1a = -40.0;
  }
  if (tData.dt1b == NULLDATA_MARK) {  // 湿度データの制限処理
    tData.dt1b = 100.1;               // 湿度エラー時は100.1%
  } else if (tData.dt1b > 100) {
    tData.dt1b = 100;
  } else if (tData.dt1b < 0) {
    tData.dt1b = 0;
  }
  if (tData.dt1c == NULLDATA_MARK) {
    tData.dt1c = 1080.1;              // エラー時は1080.1hPa
  } else if (tData.dt1c > 1080) {
    tData.dt1c = 1080;
  } else if (tData.dt1c < 760) {
    tData.dt1c = 760;
  }
  // 温度，湿度，気圧を合わせて、0-3,969,126,001の整数値(tDta) に変換
  // 整数変換時に生じた誤差は切り捨てられるので、正しく整数変換＋四捨五入されるように補正するため0.05加算している。
  tDta = (unsigned long)((tData.dt1a  + 40.05) * 10)  * 1002 * 3202 + (unsigned long)((tData.dt1b + 0.05) * 10) * 3202 + (unsigned long)((tData.dt1c - 760 + 0.05) * 10);
  // 変換した整数値(tDta)を0-251の範囲の4バイトに分解
  tDtb = tDta / 252;
  tEmData.data1d = tDta % 252;
  tDta = tDtb / 252;
  tEmData.data1c = tDtb % 252;
  tEmData.data1a = tDta / 252;
  tEmData.data1b = tDta % 252;

#ifdef DUAL_SENSORS
  if (tData.dt2a == NULLDATA_MARK) {
    tData.dt2a = 83.1;                // エラー時は 83.1℃
  } else if (tData.dt2a > 83.0) {
    tData.dt2a = 83.0;
  } else if (tData.dt2a < -40.0) {
    tData.dt2a = -40.0;
  }
  if (tData.dt2b == NULLDATA_MARK) {  // 湿度データの制限処理
    tData.dt2b = 100.1;               // 湿度エラー時は100.1%
  } else if (tData.dt2b > 100) {
    tData.dt2b = 100;
  } else if (tData.dt2b < 0) {
    tData.dt2b = 0;
  }
  if (tData.dt2c == NULLDATA_MARK) {
    tData.dt2c = 1080.1;              // エラー時は1080.1hPa
  } else if (tData.dt2c > 1080) {     // データの制限処理
    tData.dt2c = 1080;
  } else if (tData.dt2c < 760) {       // データの制限処理
    tData.dt2c = 760;
  }
  // 温度，湿度，気圧を合わせて、0-3,969,126,001の整数値(tDt) に変換
  // 整数変換時に生じた誤差は切り捨てられるので、正しく整数変換＋四捨五入されるように補正するため0.05加算している。
  tDta = (unsigned long)((tData.dt1a  + 40.05) * 10)  * 1002 * 3202 + (unsigned long)((tData.dt1b + 0.05) * 10) * 3202 + (unsigned long)((tData.dt1c - 760 + 0.05) * 10);
  // 変換した整数値(tDta)を0-251の範囲の4バイトに分解
  tDtb = tDta / 252;
  tEmData.data2d = tDta % 252;
  tDta = tDtb / 252;
  tEmData.data2c = tDtb % 252;
  tEmData.data2a = tDta / 252;
  tEmData.data2b = tDta % 252;
#endif
  return tEmData;
}


data_t restoreEmData(emData_t tEmData) { // EEPROM内の変換データを元に戻す。
  data_t tData;
  unsigned long tDta;
  unsigned long tDtb;
  
  if (tEmData.data1a == EM_NULLDATA_MARK) {
    tData = nullData;
  } else {
    tDta = (unsigned long)(tEmData.data1a) * 252 * 252 * 252 + (unsigned long)(tEmData.data1b) * 252 * 252 + (unsigned long)(tEmData.data1c) * 252 + (unsigned long)(tEmData.data1d);
    tDtb = tDta / 3202;
    tDta = tDta % 3202;
    tData.dt1c = (float)(tDta) / 10 + 760;
    if (tData.dt1c == 1080.1) tData.dt1c = NULLDATA_MARK;
    tDta = tDtb / 1002;
    tDtb = tDtb % 1002;
    tData.dt1b = (float)(tDtb) / 10;
    if (tData.dt1b == 100.1) tData.dt1b = NULLDATA_MARK;
    tData.dt1a = (float)(tDta) / 10 - 40.0;
    if (tData.dt1a == 83.1) tData.dt1a = NULLDATA_MARK;
  }
#ifdef DUAL_SENSORS
  if (tEmData.data2a == EM_NULLDATA_MARK) {
    tData = nullData;
  } else {
    tDta = (unsigned long)(tEmData.data2a) * 252 * 252 * 252 + (unsigned long)(tEmData.data1b) * 252 * 252 + (unsigned long)(tEmData.data2b) * 252 + (unsigned long)(tEmData.data2c);
    tDtb = tDta / 3202;
    tDta = tDta % 3202;
    tData.dt2c = (float)(tDta) / 10 + 760;
    if (tData.dt2c == 1080.1) tData.dt2c = NULLDATA_MARK;
    tDta = tDtb / 1002;
    tDtb = tDtb % 1002;
    tData.dt2b = (float)(tDtb) / 10;
    if (tData.dt2b == 100.1) tData.dt2b = NULLDATA_MARK;
    tData.dt2a = (float)(tDta) / 10 - 40.0;
    if (tData.dt2a == 83.1) tData.dt2a = NULLDATA_MARK;
  }
#endif
  return tData;
}
#endif // SQ_DATA


//////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef HQ_DATA
//////////////////////////////////////////////////////////////////////////////////////////////////////////
// 以下のデータを5バイトで保存する。カウント数はエラーも含めた値
// 温度：-40.0℃～85.0℃ (1252カウント)
// 湿度：0.0%～100.0% (1002カウント)
// 気圧：300.0～1100.0hPa (8002カウント)
// と仕様上のセンサの出力を制限無く保存できる。
////////////////////////////////////////////////
emData_t setEmData(data_t tData) {     // センサデータをEEPROM記憶用に変換

  emData_t tEmData;                    // EEPROM内のバイトデータには処理上の都合により 0xFC, 0xFD, 0xFE, 0xFF が使えないので、
  unsigned long tDta;                  //
  unsigned long tDtb;                  //

  if (tData.dt1a == NULLDATA_MARK) {   // 温度データの処理
    tData.dt1a = 85.1;                 // エラー時は 85.1℃
  } else if (tData.dt1a > 85.0) {
    tData.dt1a = 85.0;
  } else if (tData.dt1a < -40.0) {
    tData.dt1a = -40.0;
  }
  if (tData.dt1b == NULLDATA_MARK) {  // 湿度データの処理
    tData.dt1b = 100.1;               // 湿度エラー時は100.1%
  } else if (tData.dt1b > 100) {
    tData.dt1b = 100;
  } else if (tData.dt1b < 0) {
    tData.dt1b = 0;
  }
  if (tData.dt1c == NULLDATA_MARK) {  // 気圧データの処理
    tData.dt1c = 1100.1;              // エラー時は1100.1hPa
  } else if (tData.dt1c > 1100) {
    tData.dt1c = 1100;
  } else if (tData.dt1c < 300) {
    tData.dt1c = 300;
  }
// 温度と湿度を合わせて、0-16,003,007の整数値(tDta) に変換(小数点1桁まで) 
// 整数変換時に生じた誤差は切り捨てられるので、正しく整数変換＋四捨五入されるように補正するため0.05を加算している。
  tDta = (unsigned long)((tData.dt1a  + 40.05) * 10)  * 1002 + (unsigned long)((tData.dt1b + 0.05) * 10);
// 変換した整数値(tDta)を0-251の範囲の3バイトに分解
  tDtb = tDta / 252;
  tEmData.data1c = tDta % 252; // tEmData.data1a, data1b, data1cは0-251まで
  tEmData.data1a = tDtb / 252;
  tEmData.data1b = tDtb % 252;
  tDta = (tData.dt1c - 300 + 0.05) * 10; // Arduinoはunsigned longの4バイトが限度なので，3+2バイトで処理
  tEmData.data1d = tDta / 252;           // 変換した整数値(tDt)を0-251の範囲の2バイトに分解
  tEmData.data1e = tDta % 252;

#ifdef DUAL_SENSORS
  if (tData.dt2a == NULLDATA_MARK) {
    tData.dt2a = 85.1;                  // エラー時は 85.1℃
  } else if (tData.dt2a > 85.0) {
    tData.dt2a = 85.0;
  } else if (tData.dt2a < -40.0) {
    tData.dt2a = -40.0;
  }
  if (tData.dt2b == NULLDATA_MARK) {  // 湿度データの制限処理
    tData.dt2b = 100.1;                   // 湿度エラー時は100.1%
  } else if (tData.dt2b > 100) {
    tData.dt2b = 100;
  } else if (tData.dt2b < 0) {
    tData.dt2b = 0;
  }
  if (tData.dt2c == NULLDATA_MARK) {
    tData.dt2c = 1100.1;                 // エラー時は1081hPa
  } else if (tData.dt2c > 1100) {     // データの制限処理
    tData.dt2c = 1100;
  } else if (tData.dt2c < 300) {       // データの制限処理
    tData.dt2c = 300;
  }

// 温度と湿度を合わせて、0-16,003,007の整数値(tDta) に変換(小数点1桁まで) 
// 整数変換時に生じた誤差は切り捨てられるので、正しく整数変換＋四捨五入されるように補正するため0.05を加算している。
  tDta = (unsigned long)((tData.dt2a  + 40.05) * 10)  * 1002 + (unsigned long)((tData.dt2b + 0.05) * 10);
// 変換した整数値(tDta)を0-251の範囲の3バイトに分解
  tDtb = tDta / 252;
  tEmData.data2c = tDta % 252; // tEmData.data1a, data1b, data1cは0-251まで
  tEmData.data2a = tDtb / 252;
  tEmData.data2b = tDtb % 252;
  tDta = (tData.dt2c - 300 + 0.05) * 10; // Arduinoはunsigned longの4バイトが限度なので，3+2バイトで処理
  tEmData.data2d = tDta / 252;           // 変換した整数値(tDt)を0-251の範囲の2バイトに分解
  tEmData.data2e = tDta % 252;
#endif
  return tEmData;
}


data_t restoreEmData(emData_t tEmData) { // EEPROM内の変換データを元に戻す。
  data_t tData;
  unsigned long tDt;
  unsigned long tDta;
  unsigned long tDtb;

  if (tEmData.data1a == EM_NULLDATA_MARK) {
    tData = nullData;
  } else {
    tDt = (unsigned long)(tEmData.data1a) * 252 * 252 + (unsigned long)(tEmData.data1b) * 252 + (unsigned long)(tEmData.data1c);
    tDta = tDt / 1002;
    tDtb = tDt % 1002;
    tData.dt1a = (float)(tDta) / 10 - 40.0;
    if (tData.dt1a == 85.1) tData.dt1a = NULLDATA_MARK;

    tData.dt1b = (float)(tDtb) / 10;
    if (tData.dt1b == 100.1) tData.dt1b = NULLDATA_MARK;

    tDt = (unsigned long)(tEmData.data1d) * 252 + (unsigned long)(tEmData.data1e);
    tData.dt1c = (float)(tDt) / 10 + 300.0;
    if (tData.dt1c == 1100.1) tData.dt1c = NULLDATA_MARK;

#ifdef DUAL_SENSORS
    tDt = (unsigned long)(tEmData.data2a) * 252 * 252 + (unsigned long)(tEmData.data2b) * 252 + (unsigned long)(tEmData.data2c);
    tDta = tDt / 1002;
    tDtb = tDt % 1002;
    tData.dt2a = (float)(tDta) / 10 - 40.0;
    if (tData.dt2a == 85.1) tData.dt2a = NULLDATA_MARK;

    tData.dt2b = (float)(tDtb) / 10;
    if (tData.dt2b == 100.1) tData.dt2b = NULLDATA_MARK;

    tDt = (unsigned long)(tEmData.data2d) * 252 + (unsigned long)(tEmData.data2e);
    tData.dt2c = (float)(tDt) / 10 + 300.0;
    if (tData.dt2c == 1100.1) tData.dt2c = NULLDATA_MARK;
#endif
  }
  return tData;
}
#endif  // HQ_DATA
