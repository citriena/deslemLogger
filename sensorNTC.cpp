#include <EEPROM.h>
#include "sensorNTC.h"
#include <SHthermistor.h>   // https://github.com/citriena/SHthermistor

uint8_t _dataCount1a = 0;
#ifdef DUAL_SENSORS
uint8_t _dataCount2a = 0;
data_t _sumData = {0, 0};  // 平均値計算用
#else
data_t _sumData = {0};     // 平均値計算用
#endif

/////////////////////////////////////////////////////////////////
//                       ライブラリ初期化
/////////////////////////////////////////////////////////////////
// https://edwardmallon.files.wordpress.com/2017/04/ntc-steinhart_and_hart_calculator.xls
// 使用するサーミスタ、および使用条件に応じた値を以下に設定する。
// SHthermistor thermistor(T1,T2,T3,R1,R2,R3,,分圧抵抗値入力ピン,サーミスタ接続方法,加電圧ピン,温度補正値）
// データシートを参照して温度T1, T2, T3時の抵抗値R1, R2, R3を設定する。
// T1, T2, T3：それぞれ等間隔で10℃以上離れている温度を選定する（必ずしも正確に等間隔ではなくても良い）。
// 温度補正値（℃）。計算値にこの補正値を加算し、測定値とする。高精度の温度計との差から調整。簡易には氷を使った氷点で調整。
// 以下の数値は秋月電子通商で扱っているSEMITEC株式会社103AT-11の場合

SHthermistor thermistor1(0, 25, 50, 27280, 10000, 4160, THERMISTOR_DVR1, THERMISTOR_ADC1, NTC_GND, THERMISTOR_EXC, 0.0);
//SHthermistor thermistor1(8.88073909E-04, 2.51425171E-04, 1.92279449E-07, THERMISTOR_VDR, THERMISTOR_ADC1, NTC_GND, 9, 0.0, DEFAULT_EXCITE_VALUE);

#ifdef DUAL_SENSORS
SHthermistor thermistor2(0, 25, 50, 27280, 10000, 4160, THERMISTOR_DVR2, THERMISTOR_ADC12, NTC_GND, THERMISTOR_EXC, 0.0);
//SHthermistor thermistor2(8.88073909E-04, 2.51425171E-04, 1.92279449E-07, THERMISTOR_VDR, THERMISTOR_ADC2, NTC_GND, THERMISTOR_EXC, 0.0, DEFAULT_EXCITE_VALUE);
#endif

/////////////////////////////////////////////////////////////////
// 　　　　　　　　　　　　センサーデータ処理関係
/////////////////////////////////////////////////////////////////

void initSensor() {
  long dv_r;
  const unsigned int setMark = 0xfdfe;

  unsigned int getMark;
  EEPROM.get(DV_R_ADDRESS, getMark);
  if (getMark == setMark) {
    EEPROM.get(DV_R_ADDRESS + sizeof(getMark), dv_r);
    thermistor1.setDivR(dv_r);
#ifdef DUAL_SENSORS
    EEPROM.get(DV_R_ADDRESS + sizeof(getMark) + sizeof(dv_r), dv_r);
    thermistor2.setDivR(dv_r);
#endif
  }
}


//read data from Sensor
data_t getData() {

  data_t tData;

  tData.dt1a = thermistor1.readTemp();
  if (tData.dt1a != TH_ERR_DATA) { // エラーだったら平均処理に使わない
    _sumData.dt1a += tData.dt1a;  // 平均用の処理も行う。
    _dataCount1a++;
  } else {
    tData.dt1a = NULLDATA_MARK; // エラーの場合は指定の値を返す。
  }
#ifdef DUAL_SENSORS
  tData.dt2a = thermistor2.readTemp();
  if (tData.dt2a != TH_ERR_DATA) { // エラーだったら平均処理に使わない
    _sumData.dt2a += tData.dt2a;  // 平均用の処理も行う。
    _dataCount2a++;
  } else {
    tData.dt2a = NULLDATA_MARK; // エラーの場合は指定の値を返す。
  }
#endif
  return tData;
}


data_t avgData() {

  data_t avgData = nullData;

  if (_dataCount1a > 0) {
    avgData.dt1a = _sumData.dt1a / _dataCount1a;
  }
  _dataCount1a = 0;           // 平均を返したら平均処理用の変数をリセット
#ifdef DUAL_SENSORS
  if (_dataCount2a > 0) {
    avgData.dt2a = _sumData.dt2a / _dataCount2a;
  }
  _dataCount1a = 0;           // 平均を返したら平均処理用の変数をリセット
  _sumData = {0, 0};
#else
  _sumData = {0};
#endif
  return avgData;
}


emData_t setEmData(data_t tData) {  // センサデータをEEPROM記憶用に変換

  unsigned int tDt;
  emData_t tEmData;

  if (tData.dt1a == NULLDATA_MARK) { // 複数データがある場合は個別に処理
    tEmData.data1a = EM_NULLDATA_MARK;
    tEmData.data1b = EM_NULLDATA_MARK;
  } else {
  // 整数変換時に生じた誤差は切り捨てられるので、正しく整数変換＋四捨五入されるように補正するため50.05としている。
    tDt = (tData.dt1a + 50.05) * 10;
    tEmData.data1a = tDt / 252; // -50℃から, 0.1℃単位
    tEmData.data1b = tDt % 252;
  }
#ifdef DUAL_SENSORS
  if (tData.dt2a == NULLDATA_MARK) { // 複数データがある場合は個別に処理
    tEmData.data2a = EM_NULLDATA_MARK;
    tEmData.data2b = EM_NULLDATA_MARK;
  } else {
  // 整数変換時に生じた誤差は切り捨てられるので、正しく整数変換＋四捨五入されるように補正するため50.05としている。
    tDt = (tData.dt2a + 50.05) * 10;
    tEmData.data2a = tDt / 252; // -50℃から, 0.1℃単位
    tEmData.data2b = tDt % 252;
  }
#endif
  return tEmData;
}


data_t restoreEmData(emData_t tEmData) { // EEPROM内の変換データを元に戻す。

  data_t tData;
  if (tEmData.data1a == EM_NULLDATA_MARK) { // データ初期化のままの場合
    tData.dt1a = NULLDATA_MARK;
  } else {
    tData.dt1a = (float)(tEmData.data1a * 252 + tEmData.data1b) / 10 - 50;
  }
#ifdef DUAL_SENSORS
  if (tEmData.data2a == EM_NULLDATA_MARK) { // データ初期化のままの場合
    tData.dt2a = NULLDATA_MARK;
  } else {
    tData.dt2a = (float)(tEmData.data2a * 252 + tEmData.data2b) / 10 - 50;
  }
#endif
  return tData;
}


// restore the specific DV_R resistance from the AVR EEPROM
void getDivR() {
  long dv_r;
  const unsigned int setMark = 0xfdfe;

  unsigned int getMark;
  EEPROM.get(DV_R_ADDRESS, getMark);
  if (getMark == setMark) {
    EEPROM.get(DV_R_ADDRESS + sizeof(getMark), dv_r);
//    thermistor1.setDivR(dv_r);
  }
}
