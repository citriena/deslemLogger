#include "deslemLoggerConfig.h"
#ifdef SENSOR_MHZ19

#include "sensorMHZ19.h"
#include <MHZ19.h>   // https://github.com/crisap94/MHZ19

// #define MHZ19_UART
#define MHZ19_PWM

uint8_t _dataCount1a = 0;
data_t _sumData = {0};     // 平均値計算用

/////////////////////////////////////////////////////////////////
//                       ライブラリ初期化
/////////////////////////////////////////////////////////////////

#ifdef MHZ19_UART
const int rx_pin = 13; //Serial rx pin no
const int tx_pin = 15; //Serial tx pin no
MHZ19 *mhz19_uart = new MHZ19(rx_pin,tx_pin);
#else
const int pwmpin = 4;  // pwmを取り出すピン番号
MHZ19 *mhz19_pwm = new MHZ19(pwmpin);
#endif

/////////////////////////////////////////////////////////////////
// 　　　　　　　　　　　　センサーデータ処理関係
/////////////////////////////////////////////////////////////////

void initSensor() {

#ifdef MHZ19_UART
  mhz19_uart.begin(rx_pin, tx_pin);
  mhz19_uart.setAutoCalibration(false);
#else
  pinMode(pwmpin, OUTPUT);
  mhz19_pwm->setAutoCalibration(false);
#endif
}


//read data from Sensor
data_t getData() {

  data_t tData;

#ifdef MHZ19_UART
  measurement_t m;

  m = mhz19_uart->getMeasurement();
  tData.dt1a = m.co2_ppm;
#else
  tData.dt1a = mhz19_pwm->getPpmPwm();
#endif

  if (tData.dt1a != ERR_DATA) { // エラーだったら平均処理に使わない
    _sumData.dt1a += tData.dt1a;  // 平均用の処理も行う。
    _dataCount1a++;
  } else {
    tData.dt1a = NULLDATA_MARK; // エラーの場合は指定の値を返す。
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


emData_t setEmData(data_t tData) {  // センサデータをEEPROM記憶用に変換

  emData_t tEmData;

  if (tData.dt1a == NULLDATA_MARK) { // 複数データがある場合は個別に処理
    tEmData.data1a = EM_NULLDATA_MARK;
    tEmData.data1b = EM_NULLDATA_MARK;
  } else {
    tEmData.data1a = tData.dt1a / 252;
    tEmData.data1b = tData.dt1a % 252;
  }
  return tEmData;
}


data_t restoreEmData(emData_t tEmData) { // EEPROM内の変換データを元に戻す。

  data_t tData;
  if (tEmData.data1a == EM_NULLDATA_MARK) { // データ初期化のままの場合
    tData.dt1a = NULLDATA_MARK;
  } else {
    tData.dt1a = tEmData.data1a * 252 + tEmData.data1b;
  }
  return tData;
}

#endif // SENSOR_MHZ19
