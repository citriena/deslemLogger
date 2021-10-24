#include "sensorSCD40.h"
#include <SensirionI2CScd4x.h>   // https://github.com/Sensirion/arduino-i2c-scd4x
// SensirionI2CScd4x.h depends on SensirionCore.h at https://github.com/Sensirion/arduino-core

uint8_t _dataCount1a = 0;
uint8_t _dataCount1b = 0;
uint8_t _dataCount1c = 0;
data_t _sumData = {0, 0, 0};           // 平均値計算用

/////////////////////////////////////////////////////////////////
//                       ライブラリ初期化
/////////////////////////////////////////////////////////////////

SensirionI2CScd4x scd4x;

/////////////////////////////////////////////////////////////////
// 　　　　　　　　　　　　センサーデータ処理関係
/////////////////////////////////////////////////////////////////

void initSensor() {

  Wire.begin();
  scd4x.begin(Wire);
}


//read data from Sensor
data_t getData() {

  data_t tData;
  uint16_t error;
  uint16_t co2;
//  float temperature;
//  float humidity;

  error = scd4x.readMeasurement(co2, tData.dt1a, tData.dt1b);  // CO2, temperature, humidity
  tData.dt1c = (int32_t)co2;
  if ((error != 0) || (tData.dt1c > 0)) {
    _sumData.dt1a += tData.dt1a;  // 平均用の処理も行う。
    _dataCount1a++;
    _sumData.dt1b += tData.dt1b;  // 平均用の処理も行う。
    _dataCount1b++;
    _sumData.dt1c += tData.dt1c;  // 平均用の処理も行う。
    _dataCount1c++;
  }
  return tData;
}


data_t avgData() {

  data_t avgData = nullData;

  if (_dataCount1a > 0) {
    avgData.dt1a = _sumData.dt1a / _dataCount1a;
  }
  _dataCount1a = 0;           // 平均を返したら平均処理用の変数をリセット
  _sumData = {0};
  return avgData;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////
//#ifdef SQ_DATA
//////////////////////////////////////////////////////////////////////////////////////////////////////////
// 以下のデータを4バイトで保存する。カウント数はエラーも含めた値
// 温度：-10.0℃～60.0℃ (702カウント)
// 湿度：0.0%～100.0% (1002カウント)
// CO2 ：000～5000ppm (5002カウント)
// と仕様上のセンサの出力を制限無く保存できる。
////////////////////////////////////////////////
emData_t setEmData(data_t tData) {     // センサデータをEEPROM記憶用に変換

  emData_t tEmData;                    // EEPROM内のバイトデータには処理上の都合により 0xFC, 0xFD, 0xFE, 0xFF が使えないので、
  unsigned long tDta;                  //
  unsigned long tDtb;                  //

  if (tData.dt1a == NULLDATA_MARK) {   // 温度データの処理
    tData.dt1a = 60.1;                 // エラー時は 60.1℃
  } else if (tData.dt1a > 60.0) {
    tData.dt1a = 60.0;
  } else if (tData.dt1a < -10.0) {
    tData.dt1a = -10.0;
  }
  if (tData.dt1b == NULLDATA_MARK) {  // 湿度データの処理
    tData.dt1b = 100.1;               // 湿度エラー時は100.1%
  } else if (tData.dt1b > 100) {
    tData.dt1b = 100;
  } else if (tData.dt1b < 0) {
    tData.dt1b = 0;
  }
  if (tData.dt1c == NULLDATA_MARK) {  // CO2データの処理
    tData.dt1c = 5001;              // エラー時は5001ppm
  } else if (tData.dt1c > 5000) {
    tData.dt1c = 5000;
  }
// 温度、湿度、CO2を合わせて、0-251^4=3,969,126,001の整数値(tDta) に変換(小数点1桁まで) 
// 整数変換時に生じた誤差は切り捨てられるので、正しく整数変換＋四捨五入されるように補正するため0.05を加算している。
  tDta = (unsigned long)((tData.dt1a  + 10.05) * 10)  * 1002 * 5002 + (unsigned long)((tData.dt1b + 0.05) * 10) * 5002 + (unsigned long)tData.dt1c;
// 変換した整数値(tDta)を0-251の範囲の4バイトに分解
  tDtb = tDta / 252;
  tEmData.data1d = tDta % 252;
  tDta = tDtb / 252;
  tEmData.data1c = tDtb % 252;
  tEmData.data1a = tDta / 252;
  tEmData.data1b = tDta % 252;
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
    tDtb = tDta / 5002;
    tDta = tDta % 5002;
    tData.dt1c = (int)(tDta);
    if (tData.dt1c == 5001) tData.dt1c = NULLDATA_MARK;
    tDta = tDtb / 1002;
    tDtb = tDtb % 1002;
    tData.dt1b = (float)(tDtb) / 10;
    if (tData.dt1b == 100.1) tData.dt1b = NULLDATA_MARK;
    tData.dt1a = (float)(tDta) / 10 - 10.0;
    if (tData.dt1a == 60.1) tData.dt1a = NULLDATA_MARK;
  }
  return tData;
}
//#endif // SQ_DATA
