#include <Arduino.h>
#include <Wire.h>
#include "sensorSHT.h"

// SHT21, SHT31はI2Cアドレスが異なるので、設定を作ればSHT21を１個、SHT31を2個の合計3個を一緒に使うこともできる。

#if defined(SHT3X_A1) || defined(SHT3X_A2)  // オリジナルは接続不良になった時に止まってしまう。止まらないように修正したライブラリを使っている。
#include <AE_SHT31.h>     // http://akizukidenshi.com/download/AE_SHT31.zip
#endif

#if defined(SHT3X_S1) || defined(SHT3X_S2) 
#include <SHTSensor.h>    // https://github.com/Sensirion/arduino-sht
#endif

#ifdef SHT2X_R
#include <SHT21.h>        // https://github.com/e-radionicacom/SHT21-Arduino-Library
#endif

#ifdef HTU21
#include <SparkFunHTU21D.h> // https://github.com/sparkfun/HTU21D_Breakout
#endif


#ifdef SHT3X_A1
AE_SHT31 SHT31A = AE_SHT31(0x45); // 秋月電子通商のAE-SHT31のデフォルト（4番ピンオープン）。GY-SHT31-DはAD端子をVddにつなぐ。
#ifdef DUAL_SENSORS
AE_SHT31 SHT31B = AE_SHT31(0x44); // 秋月電子通商のAE-SHT31では4番ピンをGNDに接続  GY-SHT31-Dのデフォルト
#endif
#endif

#ifdef SHT3X_A2
AE_SHT31 SHT31A = AE_SHT31(0x44); // 秋月電子通商のAE-SHT31では4番ピンをGNDに接続  GY-SHT31-Dのデフォルト。SHT-85はこちらのみ
#ifdef DUAL_SENSORS
AE_SHT31 SHT31B = AE_SHT31(0x45); // 秋月電子通商のAE-SHT31のデフォルト（4番ピンオープン）。GY-SHT31-DはAD端子をVddにつなぐ。
#endif
#endif


#ifdef SHT3X_S1
// Sensor with normal i2c address
// Sensor 1 with address pin pulled to GND
SHTSensor sht1(SHTSensor::SHT3X);      // 秋月電子通商のAE-SHT31では4番ピンをGNDに接続  GY-SHT31-Dのデフォルト。SHT-85はこちらのみ
#ifdef DUAL_SENSORS
// Sensor with alternative i2c address
// Sensor 2 with address pin pulled to Vdd
SHTSensor sht2(SHTSensor::SHT3X_ALT);  // 秋月電子通商のAE-SHT31のデフォルト（4番ピンオープン）。GY-SHT31-DはAD端子をVddにつなぐ。
#endif
#endif

#ifdef SHT3X_S2
// Sensor with normal i2c address
// Sensor 1 with address pin pulled to GND
SHTSensor sht1(SHTSensor::SHT3X_ALT);  // 秋月電子通商のAE-SHT31のデフォルト（4番ピンオープン）。GY-SHT31-DはAD端子をVddにつなぐ。
#ifdef DUAL_SENSORS
// Sensor with alternative i2c address
// Sensor 2 with address pin pulled to Vdd
SHTSensor sht2(SHTSensor::SHT3X);      // 秋月電子通商のAE-SHT31では4番ピンをGNDに接続  GY-SHT31-Dのデフォルト。SHT-85はこちらのみ
#endif
#endif



#ifdef SHT2X_R
// I2C address is 0x40
SHT21 sht;
#endif

#ifdef HTU21
//Create an instance of the object
// I2C address is 0x40
HTU21D myHumidity;
#endif





uint8_t _dataCount1a = 0;
uint8_t _dataCount1b = 0;
#ifdef DUAL_SENSORS
uint8_t _dataCount2a = 0;
uint8_t _dataCount2b = 0;
data_t _sumData = {0, 0, 0, 0};  // 平均値計算用
#else
data_t _sumData = {0, 0};        // 平均値計算用
#endif

/////////////////////////////////////////////////////////////////
//                  センサーデータ処理関係
/////////////////////////////////////////////////////////////////

void initSensor() {

#if defined(SHT3X_A1) || defined(SHT3X_A2)
  // SHT31使用準備 set up SHT31 for measurement
  SHT31A.SoftReset(); // SHT31をソフトリセット soft reset SHT31
  SHT31A.Heater(0);   // 内蔵ヒーター 0:OFF 1:ON internal heater  0:OFF 1:ON
#ifdef DUAL_SENSORS
  SHT31B.SoftReset();
  SHT31B.Heater(0);
#endif
#endif


#if defined(SHT3X_S1) || defined(SHT3X_S2)
  sht1.init();// initialize sensor
#ifdef DUAL_SENSORS
  sht2.init(); // initialize sensor
#endif
#endif


#ifdef SHT2X_R
#endif


#ifdef HTU21
  myHumidity.begin();
#endif
}


//read data from Sensor
data_t getData() {

  data_t tData;

#if defined(SHT3X_A1) || defined(SHT3X_A2)
  SHT31A.GetTempHum();
  tData.dt1a = SHT31A.Temperature();
  tData.dt1b = SHT31A.Humidity();
#ifdef DUAL_SENSORS
  SHT31B.GetTempHum();
  tData.dt2a = SHT31B.Temperature();
  tData.dt2b = SHT31B.Humidity();
#endif
#endif


#if defined(SHT3X_S1) || defined(SHT3X_S2)
  if (sht1.readSample()) {
    tData.dt1a = sht1.getTemperature();
    tData.dt1b = sht1.getHumidity();
  } else {
    tData.dt1a = NULLDATA_MARK;
    tData.dt1b = NULLDATA_MARK;
  }

#ifdef DUAL_SENSORS
  if (sht2.readSample()) {
    tData.dt2a = sht2.getTemperature();
    tData.dt2b = sht2.getHumidity();
  } else {
    tData.dt2a = NULLDATA_MARK;
    tData.dt2b = NULLDATA_MARK;
  }
#endif
#endif


#ifdef SHT2X_R
  tData.dt1a = sht.getTemperature();  // get temp from SHT
  tData.dt1b = sht.getHumidity(); // get temp from SHT
#endif


#ifdef HTU21
  tData.dt1a = myHumidity.readTemperature();
  tData.dt1b = myHumidity.readHumidity();
#endif


  if (tData.dt1a != NULLDATA_MARK) {
    _sumData.dt1a += tData.dt1a;  // 平均用の処理も行う。
    _dataCount1a++;
  }
  if (tData.dt1b != NULLDATA_MARK) {
    _sumData.dt1b += tData.dt1b;  // 平均用の処理も行う。
    _dataCount1b++;
  }
#ifdef DUAL_SENSORS
  if (tData.dt2a != NULLDATA_MARK) {
    _sumData.dt2a += tData.dt2a;  // 平均用の処理も行う。
    _dataCount2a++;
  }
  if (tData.dt2b != NULLDATA_MARK) {
    _sumData.dt2b += tData.dt2b;  // 平均用の処理も行う。
    _dataCount2b++;
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
  _dataCount1a = 0;           // 平均を返したら平均処理用の変数をリセット
  _dataCount1b = 0;           // 平均を返したら平均処理用の変数をリセット
#ifdef DUAL_SENSORS
  if (_dataCount2a > 0) {
    avgData.dt2a = _sumData.dt2a / _dataCount2a;
  }
  if (_dataCount2b > 0) {
    avgData.dt2b = _sumData.dt2b / _dataCount2b;
  }
  _dataCount2a = 0;           // 平均を返したら平均処理用の変数をリセット
  _dataCount2b = 0;           // 平均を返したら平均処理用の変数をリセット
  _sumData = {0, 0, 0, 0};
#else
  _sumData = {0, 0};
#endif
  return avgData;
}

#ifdef ECO_DATA
////////////////////////////////////////////////
// ECO_DATAが設定されている場合
// EEPROM使用量節約のために温度、湿度で2バイトで保存する。
// ECO_DATAを設定しなければ温度、湿度を3バイトで保存する。
// 
// バイト数が少ないので、以下の制限がある。
// 温度：-9.9～52.7℃（0.1℃単位）
// 湿度：1～100%（1%$単位）
// 測定エラー時は-10℃、0%を返す。

// ECO_DATAが設定されている場合の変換方法は以下の通り
// 温度:tp, 湿度rhとする。
// ct = (tp + 10) * 10
// で-10.0~52.7℃を0~627の整数に変換
// 湿度
// ch = rh (0~100(%)
// の整数値
// tDt = ct * 101 * ch
// とすると上記の温度と湿度は
// 0 <= cDt <= 63,427 < 63,504（0-251の2バイト最大値） 
// で表現できる。
////////////////////////////////////////////////
emData_t setEmData(data_t tData) {   // センサデータをEEPROM記憶用に変換

  emData_t tEmData;
  unsigned int tDt;                  // tDtは0-63503しか使えない。

  if (tData.dt1a == NULLDATA_MARK) { // 複数データがある場合は個別に処理
    tData.dt1a = -10;                 // エラー時は-10℃
  } else if (tData.dt1a > 52.7) {     // emData変換最大値
    tData.dt1a = 52.7;
  } else if (tData.dt1a < -9.9) {
    tData.dt1a = -9.9;
  }
  if (tData.dt1b == NULLDATA_MARK) { // 複数データがある場合は個別に処理
    tData.dt1b = 0;                   // 湿度エラー時は0%
  } else if (tData.dt1b > 100) {      // 0％の処理は良いであろう。
    tData.dt1b = 100;
  }
  // 整数変換時に生じた誤差は切り捨てられるので、正しく整数変換＋四捨五入されるように補正するため10.05としている。
  tDt = (unsigned int)((tData.dt1a  + 10.05) * 10) * 101 + (unsigned int)tData.dt1b;
  tEmData.data1a = tDt / 252;        // tEmData.data1, data2は0-251まで
  tEmData.data1b = tDt % 252;
#ifdef DUAL_SENSORS
  if (tData.dt2a == NULLDATA_MARK) { // 複数データがある場合は個別に処理
    tData.dt2a = -10;                 // エラー時は-10℃
  } else if (tData.dt2a > 52.7) {     // emData変換最大値
    tData.dt2a = 52.7;
  } else if (tData.dt2a < -9.9) {
    tData.dt2a = -9.9;
  }
  if (tData.dt2b == NULLDATA_MARK) { // 複数データがある場合は個別に処理
    tData.dt2b = 0;                   // 湿度エラー時は0%
  } else if (tData.dt2b > 100) {      // 0％の処理は良いであろう。
    tData.dt2b = 100;
  }
  // 整数変換時に生じた誤差は切り捨てられるので、正しく整数変換＋四捨五入されるように補正するため10.05としている。
  tDt = (unsigned int)((tData.dt2a  + 10.05) * 10) * 101 + (unsigned int)tData.dt2b;
  tEmData.data2a = tDt / 252;        // tEmData.data1, data2は0-251まで
  tEmData.data2b = tDt % 252;
#endif
  return tEmData;
}


data_t restoreEmData(emData_t tEmData) { // EEPROM内の変換データを元に戻す。
  data_t tData;
  unsigned int tDt;
  
  if (tEmData.data1a == EM_NULLDATA_MARK) {
    tData = nullData;
  } else {
    tDt = (unsigned int)tEmData.data1a * 252 + (unsigned int)tEmData.data1b;
    tData.dt1a = (float)(tDt / 101) / 10 - 10.0;
    if (tData.dt1a == -10) tData.dt1a = NULLDATA_MARK;

    tData.dt1b = tDt % 101;
    if (tData.dt1b == 0) tData.dt1b = NULLDATA_MARK;
#ifdef DUAL_SENSORS
    tDt = (unsigned int)tEmData.data2a * 252 + (unsigned int)tEmData.data2b;
    tData.dt2a = (float)(tDt / 101) / 10 - 10.0;
    if (tData.dt2a == -10) tData.dt2a = NULLDATA_MARK;

    tData.dt1b = tDt % 101;
    if (tData.dt2b == 0) tData.dt2b = NULLDATA_MARK;
#endif
  }
  return tData;
}

#else

////////////////////////////////////////////////
// ECO_DATAが設定されて**いない**場合
// 以下の温度、湿度を3バイトで保存する。
// 温度：-40.0℃～125.0℃
// 湿度：0.1%～100.0%
// と仕様上のセンサの出力を大きな制限無く保存できる。
////////////////////////////////////////////////
emData_t setEmData(data_t tData) {   // センサデータをEEPROM記憶用に変換

  emData_t tEmData;                    // EEPROM内のバイトデータには処理上の都合により 0xFC, 0xFD, 0xFE, 0xFF が使えないので、
  unsigned long tDta;                  // tDtaは0-16,003,007とする必要がある。
  unsigned long tDtb;                  // tDtbは0-63503とする必要がある。

  if ((tData.dt1a == NULLDATA_MARK) || (tData.dt1a > 125.0) || (tData.dt1a < -40.0)) {   // 温度データの制限処理
    tData.dt1a = 150;                  // エラー時は 150℃ 125.0は仕様上の出力最高値
  }
  if (tData.dt1b == NULLDATA_MARK) {  // 湿度データの制限処理
    tData.dt1b = 0;                   // 湿度エラー時は0%
  } else if (tData.dt1b > 100) {      // 0％の処理は良いであろう。
    tData.dt1b = 100;
  }
  // 温度と湿度を合わせて、0-16,003,007の整数値(tDta) に変換(小数点1桁まで) 
  // 整数変換時に生じた誤差は切り捨てられるので、正しく整数変換＋四捨五入されるように補正するため40.05としている。
  tDta = (unsigned long)((tData.dt1a  + 40.05) * 10)  * 1001 + (unsigned long)(tData.dt1b * 10);
  // 変換した整数値(tDta)を0-251の範囲の3バイトに分解
  tDtb = tDta / 252;
  tEmData.data1a = tDtb / 252; // tEmData.data1a, data1b, data1cは0-251まで
  tEmData.data1b = tDtb % 252;
  tEmData.data1c = tDta % 252;

#ifdef DUAL_SENSORS
  if ((tData.dt2a == NULLDATA_MARK) || (tData.dt2a > 125.0) || (tData.dt2a < -40.0)) {   // 温度データの制限処理
    tData.dt1a = 150;                  // エラー時は 150℃ 125.0は仕様上の出力最高値
  }
  if (tData.dt2b == NULLDATA_MARK) {  // 複数データがある場合は個別に処理
    tData.dt2b = 0;                   // 湿度エラー時は0%
  } else if (tData.dt2b > 100) {      // 0％の処理は良いであろう。
    tData.dt2b = 100;
  }
  // 温度と湿度を合わせて、0-16,003,007の整数値(tDta) に変換(小数点1桁まで)
  // 整数変換時に生じた誤差は切り捨てられるので、正しく整数変換＋四捨五入されるように補正するため40.05としている。
  tDta = (unsigned long)((tData.dt2a  + 40.05) * 10)  * 1001 + (unsigned long)(tData.dt2b * 10);
  // 変換した整数値(tDta)を0-251の範囲の3バイトに分解
  tDtb = tDta / 252;
  tEmData.data2a = tDtb / 252; // tEmData.data2a, data2b, data2cは0-251まで
  tEmData.data2b = tDtb % 252;
  tEmData.data2c = tDta % 252;
#endif
  return tEmData;
}


data_t restoreEmData(emData_t tEmData) { // EEPROM内の変換データを元に戻す。
  data_t tData;
  unsigned long tDt;
  
  if (tEmData.data1a == EM_NULLDATA_MARK) {
    tData = nullData;
  } else {
    tDt = (unsigned long)(tEmData.data1a) * 252 * 252 + (unsigned long)(tEmData.data1b) * 252 + (unsigned long)(tEmData.data1c);
    tData.dt1a = ((float)(tDt / 1001) / 10) - 40.0;
    if (tData.dt1a == 150) tData.dt1a = NULLDATA_MARK;

    tData.dt1b = (float)(tDt % 1001) / 10;
    if (tData.dt1b == 0) tData.dt1b = NULLDATA_MARK;

#ifdef DUAL_SENSORS
    tDt = (unsigned long)(tEmData.data2a) * 252 * 252 + (unsigned long)(tEmData.data2b) * 252 + (unsigned long)(tEmData.data2c);
    tData.dt2a = ((float)(tDt / 1001) / 10) - 40.0;
    if (tData.dt2a == 150) tData.dt2a = NULLDATA_MARK;

    tData.dt2b = (float)(tDt % 1001) / 10;
    if (tData.dt2b == 0) tData.dt2b = NULLDATA_MARK;
#endif
  }
  return tData;
}

#endif