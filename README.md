# 電池長期間稼働Arduinoロガー
deslemLogger (deep sleep EEPROM logger)  
https://github.com/citriena/deslemLogger  
Copyright (C) 2021 by citriena

自作の[電池長期間駆動Arduinoロガー](https://grafting.at.webry.info/202001/article_2.html)で使っているスケッチです。サーミスタであれば、1分毎測定、10分毎記録で1年間以上連続稼働が可能です。LCDを切れば電池交換無しで数年は稼働可能です。

![電池長期間駆動Arduinoロガー](https://userdisk.webry.biglobe.ne.jp/043/676/11/N000/000/000/157995619336870194040.jpg "外観")
![電池長期間駆動Arduinoロガー](https://userdisk.webry.biglobe.ne.jp/043/676/11/N000/000/000/157995623933730891559.jpg "内部")


## 特徴

* スリープモードを基本とし、測定時刻のみRTCのタイマー割り込みで各種処理を行うことで低消費電流化
* タイマー割り込み間隔、センサデータ取得間隔、ロギング間隔を設定可能（一部制限有）
* データ記憶用メモリにはEEPROMを使い、消費電流低減およびSDカード書き込み時の電圧低下によるトラブル回避
* センサで異なる処理はライブラリとして分けるとともに、各種設定はdeslemLoggerConfig.hに集約しているため、メインスケッチを変更すること無く各種センサへの対応が可能
* 積算温度計機能設定可能

## データロガーの操作
概要だけです。細かい動作までは説明し切れていません。
### ボタン操作
* ボタンを押すとデータ表示、ロギング設定表示、SHORI MENU表示が切替わる。
* データ表示はセンサー毎に設定可能。上の写真はSHT-31を2本使用時で以下の通り

```
時 温度1 湿度1
分 温度2 湿度2
```

### ロギング設定表示

LCD表示例
```
M01,L10min 05%
RUN  ENDLES 
```
* 1行目 左から　測定間隔(M01)、ログ間隔(L01)、時間単位(sec, min, hr)、メモリ使用量(%)
 * メモリ使用量はENDLESSモードで一周すると0に戻ります。
* 2行目 左から　動作状況、ログモード
 * 動作状況：
   * RUN：ロギング実行中
   * STOP：ロギング停止中、測定＆LCD表示は実行
   * FULL：データ満杯で停止

 * ログモード：
   * ENDLESS：EEPROM最後まで書き込んだら古いデータから上書きしながらロギング継続
   * WRITE ONCE：EEPROM最後まで書き込んだらロギング停止

### SHORI MENU（日時設定、データ回収、データ消去）
SHORU MENUでは以下のように、処理画面表示と日時表示となる。
```
SHORI
2021/10/24 11:30
```
この表示時にボタンを長押しすると以下の処理選択表示となる。ボタンを押すと処理切替、ボタンの長押しで処理実行となる。５秒操作しないと基本表示に戻る。


#### DATE&TIME（日付、時刻設定）
日時設定する（年月日時分）。ボタンを押すとカーソルが点滅している項目（最初は「年」）の値が大きくなる。値を小さくするボタンはないので、オーバーした場合は一周するまでボタンを押し続ける。約５秒ボタンを押さないと次の設定項目に移動する。「分」の設定が完了すると「Press to set」と表示される。ボタンを押した時点を０秒として設定完了する。

#### DATA COPY（データ回収）
記録データをmicroSDカードにコピーする（CSV形式）。事前にmicroSDカードスロットにカードを挿入しておく。最大32GBまでのカードが使用できる（SD, SDHC）。LCDにはコピーしたデータ数が表示される。データは消去されない。
コピーが完了したら通常の測定表示になるのでmicroSDカードを取り出す。カードを挿しておくと消費電流が増加するので、データ回収時以外は抜いておく。

#### DATA RESET（データ消去）
データを全消去する。消去の進行状況が％で表示される。全消去のみで、個別にデータ消去はできない。

#### BACK（戻る）
基本表示に戻る。

### 電池交換
単三電池の電圧が低下するとLCDに電池マークが表示される（中少空の３段階）。電池空マークになる前に電池を交換する。計時用バックアップ充電池があるので、単三電池が切れても１０日程度は日時が維持される。

## 使用ハードウェア

* Arduino: Arduino Pro Mini 8MHz 3.3V
  * スリープ時の消費電流を下げるためにレギュレータを交換しています。
* RTC: SEIKO EPRON RX8900（秋月電子通商 AE-RX8900）
* EEPROM: MICROCHIP 24FC1025（最大４個（512Kbyte）接続対応）
  * 4個使用時の保存可能データ数は約25万（温度）、約17万（温湿度）
* microSDカードスロット（秋月電子通商 AE-MICRO-SD-DIP）
* LCD: Strawberry Linux I2C低電圧キャラクタ液晶モジュール（１６ｘ２行）
* センサ: サーミスタ（SEMITEC 103AT-11等）、Sensirion SHT21/25, SHT31/35、Bosch BME280等

その他細かいパーツまで含めたリストを最後に掲載します。


## Arduino入出力ピン
Arduino Pro Mini のピン使用状況は以下の通りです。
```
D00: Serial
D01: Serial
D02: RTC /INT
D03: Manual /INT（プッシュボタン）
D04:
D05:
D06:
D07:
D08:
D09: サーミスタ加電圧
D10: SPI(SS)
D11: SPI(MOSI)
D12: SPI(MISO)
D13: SPI(SCK)
A00:
A01:
A02: サーミスタ測定ピン２
A03: サーミスタ測定ピン１
A04: I2C(SDA)
A05: I2C(SCL)
A06:
A07:
```

## センサーの設定
センサーによって異なる処理はライブラリ等にまとめています。
このため、メインのスケッチを変更すること無くライブラリを入れ替えるだけで各種センサーに対応可能です。
画面表示とmicroSDへの書き出し内容はセンサーが異なっても同じものが使えることもあるので別ファイルとし、選ぶようにしています。
センサは、種類によっては励起が必要だったり、交流駆動が必要だったりなど少し面倒なこともありますが、I2CやSPI接続センサであれば
大抵はライブラリの対応だけで済むと思います。


### 対応済みセンサー（ライブラリ）
* 現在対応済みセンサーのライブラリは、以下のsensorSHT.h（Sensirion SHT-21/25, SHT-31/35等用）、sensorBME280.h（Bosch BME280用），およびsensorNTC.h（サーミスタ用） の３つです。それ以外のセンサーを使う場合はセンサライブラリの作成が必要です。異なるセンサの組み合わせも可能です。
* センサライブラリのファイル（xxxx.h、およびxxxx.cpp）はメインスケッチと同じフォルダに置きます。
使用しないセンサのライブラリファイルが同じフォルダにあるとコンパイルエラーとなります。別のフォルダに移動させておいてください。このことでコンパイルエラーが出たら、ファイルを移動後Arduino IDEの再起動が必要です。普通のライブラリにしてライブラリフォルダに置けば解決しますが、それはやりたくありません。良い解決法がありましたらご教示いただけると助かります。
* 各センサー設定有効化のマクロ定義は deslemLoggerConfig.h にあります。
 基本的にはセンサー有効化のマクロ定義は1つだけ設定し、使用しないセンサーのマクロ定義はコメントアウト等で無効化しておきます。同時使用が可能なセンサであれば、適切にセンサーライブラリを作成すれば、同時使用することが可能です。
* センサーの設定を変更するとEEPROMの記録済みデータは正しく読み出すことができなくなくなります。必要ならセンサー設定変更前に読み出してください。センサー設定変更後はMENUからリセットしてEEPROMのデータを消去してください。
* センサーの設定を変更しなければ、ログ間隔の設定を変更してもEEPROMの記録済みデータは正しく読み出すことが可能です。

#### sensorSHT.h
* Sensirion SHT-21/25, SHT-31/35, SHT-85, TE Connectivity HTU21D用
* SHT-31/35は異なるI2Cアドレス設定にすることによりセンサー2個同時使用可能
* センサーからデータ取得するライブラリが別途必要。sensorSHT.cppを参照

#### sensorBME280.h
* Bosch BME280用。BMP280も使用可能だが、当然ながら湿度は無効
* 異なるI2Cアドレス設定にすることによりセンサー2個同時使用可能
* センサーからデータ取得するライブラリが別途必要。sensorBME280.cppを参照

#### sensorNTC.h
* サーミスタ用
* センサー2個同時使用可能
  * deslemLoggerConfig.h内のDUAL_SENSORSマクロ定義を有効化
* センサーからデータ取得するライブラリ（SHthermistor）が別途必要。sensorNTC.cppを参照
* サーミスタの特性はsensorNTC.cpp内のSHthermistorコンストラクタで設定
  * デフォルトでSEMITEC株式会社 103AT-11の特性を設定してある。特性が異なる場合は設定変更必要

#### sensorMHZ19.h
* MHZ19 CO2センサ用
* センサーからデータ取得するライブラリが別途必要。sensorMHZ19.cppを参照
* 接続はPWM

#### sensorSCD40.h
* Sensirion SCD40 CO2センサ用
* センサーからデータ取得するライブラリが別途必要。sensorSCD40.cppを参照
* 接続はI2C


## 変更可能なセンサー等設定（マクロ定義）

### DUAL_SENSORS
* 設定可能値：定義／無定義
* デフォルト値：無定義
* 内容
  * サーミスタを2本使う場合に定義
  * sensorSHTでSHT31/35を使う場合等はI2Cアドレスの異なる設定を２つ有効にすれば自動的にこの設定が有効となり、２本使用可能です。
  * 当然ですがハードウェアも2本使用に対応させる必要があります。

### SEKISAN
* 設定可能値：定義／無定義
* デフォルト値：無定義
* 内容
  * 積算温度計として使う場合に定義
  * 一番目のセンサ出力を温度として処理
  * 日付が変わるときに前日の平均温度を0.1℃単位で計算し加算する。
  * 積算温度の最大値は6502℃日。不足ならSEKISAN_LEVERAGEを小さくする（現在は10）。しかし、たとえばこれを5にすると積算温度の最大値は13004℃日となるが、平均温度の計算は0.2℃単位となる。gAccumTempの定義をlongとしてもよいが，その場合は同時にprintAT()の引数定義も変更必要

## ロギング設定（マクロ定義）
以下の設定は全てdeslemLoggerConfig.h内に集約しています。  
現在はロギング設定の変更に再コンパイルが必要です。いずれメニューから設定できるようにしたいと考えています。
### TIMER_INTERVAL
* 設定可能値：60の約数
* デフォルト値: 1
* 内容
  * タイマー割り込み間隔
  * 現在、この設定が有効なのは```#define INTERVAL_UNIT SEC_INTERVAL```の時だけで、それ以外の時は
タイマー割り込み間隔は１分となります。  
LCD の時計表示更新と連動しているので、これを1分よりも長くすると表示時刻が遅れるためこのような仕様となっています。

### MEASURE_INTERVAL
* 設定可能値：60の約数（単位が分、秒の場合）、24の約数（単位が時の場合）。ただし、```MEASURE_INTERVAL >= TIMER_INTERVAL```
* デフォルト値: 1
* 内容
  * 測定間隔
  * 単位は INTERVAL_UNIT で設定（秒、分、時）

### LOG_INTERVAL
* 設定可能値：60の約数（単位が分、秒の場合）、24の約数（単位が時の場合）。ただし、```LOG_INTERVALl >= MEASURE_INTERVAL```
* デフォルト値: 10
* 内容
  * 記録間隔。前回記録以降の測定データの平均値を記録
  * 単位は INTERVAL_UNIT で設定（秒、分、時）
  * センサーの設定を変更しなければ、この設定と次のINTERVAL_UNITの設定を変更してもEEPROM内の記録済みデータは正しく読み出すことができます。

### INTERVAL_UNIT
* 設定可能値：
  * SEC_INTERVAL：秒
  * MIN_INTERVAL：分
  * HOUR_INTERVAL：時
* デフォルト値: MIN_INTERVAL
* 内容
  * TIMER_INTERVAL, MEASURE_INTERVAL, LOG_INTERVALの単位（秒、分、時）

## ロガー動作設定（マクロ定義）
以下の設定は全てdeslemLoggerConfig.h内に集約しています。  
作成済みのセンサーライブラリでは適切な値を設定するようにしてあるので変更の必要はありません。
新たにセンサーライブラリを作成したら対応する設定の作成が必要です。

### EM_DATA_PER_BUFF
* 設定可能値：1-29 （30以上の設定も可能だが，適切ではない。）
* デフォルト値：12（sensorNTC.hをシングルセンサで使う場合）
* 内容
  * データ書き込みバッファ（gEmDataBuff）内データ数。小さくするとEEPROMに書き込む回数が増えます。
EEPROMには書込回数制限があるのであまり小さい値は好ましくありません。
しかし、大きくするとその分バッファが大きくなり、ArduinoのRAMを消費します。
  * 適切な設定値の基準としては、gEmDataBuffのサイズが30バイトを超えないようにします。
gEmDataBuffのサイズは
```emData_tのbyte数 x EM_DATA_PER_BUFF + 1```
です。
  * sensorNTC.hをシングルセンサ使う場合は emData_t は2バイトなので、EM_DATA_PER_BUFFが12では gEmDataBuff は2x12+1=25バイトと30バイト以下となります。
  * ここでの設定とは別にWire.hライブラリのバッファが32バイトで、書き込みに使えるのはそのうち30バイトという制限があります。
30バイトを超えてもEEPROM_24xx1025ライブラリが30バイト毎に分割して書き込みますので動作はしますが
効率的なメモリの使用とは言えません。このため適切な設定値の基準としてgEmDataBuffのサイズを30バイト以下にしています。
 * microSDへのデータ転送時は事前にgEmDataBuffの内容をEEPROMに書き込みます。このため、その時点でのデータは全て転送されます。

### EM_BUFF_WRITE_PER_HEADER
* 設定可能値：1-256程度 （ただし、16より大きな値では未検証）  
ヘッダ以降の（gIntervalUnit単位での）経過時間をintで計算しているので、
```EM_DATA_PER_BUFF x EM_BUFF_WRITE_PER_HEADER x LOG_INERVAL```がintの制限を超えないようにします。そんなに大きな値にすることはないと思いますが。
単位が秒であれば9時間、分であれば22日です。デフォルト設定値は```EM_DATA_PER_BUFF x EM_BUFF_WRITE_PER_HEADER```は144程度なので、LOG_INERVALを60としても8,640で問題ありません。
* デフォルト値: 12（sensorNTC.hをシングルセンサで使う場合）
* 内容
  * EEPROMにデータを書き込む際、データ毎に時刻を書き込むのでは無く、
一つの時刻に対して複数のデータを書き込んでメモリ節約しています。
すなわち、ヘッダに最初のデータの時刻、およびロギング間隔を記録し、
その後のデータはこれらの情報から時刻を算出します。
  * ヘッダは一定回数バッファ書込毎に書き込みます。
```EM_BUFF_WRITE_PER_HEADER```はこの```バッファ書込回数／ヘッダ```を指定します。
  * ヘッダには8バイト使いますので、EM_BUFF_WRITE_PER_HEADERが大きい方がEEPROMに記憶できるデータが多くなりますが、
EEPROMメモリを一周したらヘッダ単位で前のデータが順に読み出せなくなります。
  * デフォルトの設定でsensorNTC.hをシングルセンサで使う場合、ロギング間隔は10分なので、ヘッダの書き込み間隔は 10x12x12=1440分=24時間=1日。
メモリ使用量はヘッダあたり（=1日あたり）2x12x12+8=296バイト。EEPROM 512Kバイトだと1771日記憶できます。

## LCD動作設定（マクロ定義）
消費電流節約のために夜間LCDを消す場合に設定します。  
この機能を使用しない場合はLCD_OFF_TIMEとLCD_ON_TIMEに同じ値を設定します。  
必要ならLED消灯時にボタンを押すと点灯します。

### LCD_ON_TIME
* 設定可能値：0-23
* デフォルト値: 6
* 内容
  * LCDを表示開始する時間を指定

### LCD_OFF_TIME
* 設定可能値：0-23
* デフォルト値: 20
* 内容
  * LCDを表示停止する時間を指定

## 使用ライブラリの設定（コンストラクタ）

### EEPROM_24xx1025
* データを保存するEEPROMの24xx1025を扱うライブラリ
* 場所：deslemLogger.ino
* 内容：使用する24xx1025の個数、I2Cアドレスを設定します。詳細はEEPROM_24xx1025ライブラリの説明を参照してください。デフォルトでは最大数の4個使用するように設定してあります。

## EEPROM内のデータ構造

### 各情報の識別
EEPROM内の内容には、[ヘッダ]、[データ]、[未書込]の三種類＋αがあり、これらを識別する必要があります。ファイルシステムのような高度な管理方法は使っておらず、以下のように使用する値によってこれらを識別しています。
~~~
0xFF: EEPROM初期化（未書込）マーク
0xFE: ヘッダマーク（ヘッダ開始）
0xFD: データ領域初期化マーク（データ未保存の識別）
0xFC: 識別マーク予備
0x00-0xFB: データ
~~~
上記のように0xFC-0xFFはEEPROM内容の識別に使っていますので、データ、ヘッダ内容には使えません。
### ヘッダの構成内容
ヘッダの構成内容は以下の通りです。
~~~
1バイト目：ヘッダマーク(0xFE)
2バイト目：年（tmElements_t と同じ（1970年基準））
3バイト目：月（ただし最上位1ビットはファイル更新フラグ）
4バイト目：日
5バイト目：時
6バイト目：分
7バイト目：秒
8バイト目：記録間隔（上位2ビット：単位、下位6ビット：数値）
~~~
ヘッダの最初は0xFEで、これでヘッダだと識別しています。
### EEPROM内の記憶内容構成
~~~
EM_DATA_PER_BUFF = 4
EM_BUFF_WRITE_PER_HEADER = 2
~~~

の場合は以下のようになります。

```
||HEADER||data1|data2|data3|data4||data5|data6|data7|data8||HEADER||data9||
        ||        Buffer         ||        Buffer         ||      ||
```
EEPROMへのデータ書き込み動作は以下の通りです。

---
1. バッファの最初のデータ情報を基にヘッダを作成し、書込
1. 取得したデータは一旦Arduino内のメモリに確保したバッファ（複数データ保存可能）に保存
1. バッファが満杯(EM_DATA_PER_BUFF)になったらEEPROMに書込み、バッファをクリア
1. 2-3の動作を繰り返す。
1. バッファ書き込み数が一定数(EM_BUFF_WRITE_PER_HEADER)となったら1から再度処理反復
---
データ（バッファ）の最後には毎回未書込マーク(0xFF)を書込み、次回データ（バッファ）書込み時は0xFFを上書きします。これにより、メモリを一周した場合でもデータの最後を識別可能です。

また、上記のことからエンドレスモードでEEPROMメモリを一周回った後はHeader単位で前のデータが読めなくなります。このため、ヘッダの書き込み頻度はこれを勘案して設定します。ライトワンスモードであればEEPROMの最後までデータを書き込んだらロギングを停止します。

一旦ロギングを終了し再度ロギングを開始した場合は当然ながら別ファイルになります。また、データの書き出しやセンサトラブル等でロギング間隔が大きく空いた場合（具体的には前回書き込んだバッファの2つ以上先のバッファ位置のデータとなった場合）も別ファイルになります。それ以内のデータの欠落はデータ無で同じファイルで処理します。バッファ内のデータ位置はヘッダ時刻からの経過時間で管理していますので、このように欠測データが出ても計測時刻は正しく記録されます。

ファイル番号はEEPROMメモリの最初から読み出した順になりますので、エンドレスモードでメモリを一周してからロギングを一旦停止後、ロギングを再開するとその後のログファイルの番号が最初になるため、メモリ一周前のログファイルと番号が逆転します。ファイルの更新時刻はログファイル最初のデータ時刻としていますので、更新時刻でソートすれば記録順になります。

エンドレスモードでは、メモリの最後になるとメモリの最初から続きを書き込みます。読み出し時はメモリの一番最初にヘッダがないとメモリを一周して書き込まれたと判断し、書込みの最後（0xFF）を探してその先からデータの読み出しを開始します。このため、メモリの最後と最初をまたいだデータのヘッダが上書きされて無くなるとメモリの最初のデータは読み出されません。やろうと思えば対応はできますが、通常はエンドレスモードでもそれ以前にデータの読み出しはすると思いますし、そもそもプログラム領域の余裕がないので放置しています。

### EEPROM内のデータ変換
センサの出力データが浮動小数点の場合、そのままEEPROMに保存すると4バイト必要なので、限られたEEPROMを有効に利用するために適切に変換して保存に必要なメモリを少なくしています。加えて、前述のようにEEPROM内のデータには管理上0-0xFBしか使えませんので、データ変換時はこの対処も必要です。付属のライブラリではこれらへの対応を行ない、sensorSHTの場合は温度-40.0～117.0℃、湿度0.0～100.0%で3バイト、sensorBME280の場合は温度-40.0～85.0℃、湿度0.0～100.0%、気圧300.0～1100.0hPaで5バイト、sensorNTCの場合は温度-50.0～6250.0℃で2バイトに変換しています。

しかし、状況によってはそこまでの範囲や桁は不要な場合もあります。その場合、必要最小限の範囲、桁の変換とすることにより使用EEPROMメモリがさらに少なくなり、保存できるデータ数を多くできることもあります。その試みとしてsensorSHTライブラリとsensorBME280ライブラリでは、LQ_DATA（節約変換）, SQ_DATA（実用変換）, HQ_DATA（ほぼ完全変換）の設定を切り替えることによりデータ変換方法を変更できるようにしています。デフォルトはHQ_DATAで上記の変換内容です。

具体的に変換を行なっているのはセンサーライブラリの以下の関数です。

~~~
センサーデータをEEPROMに書込むデータに変換
emData_t setEmData(data_t tData);
~~~
~~~
EEPROMから読出したデータを元のデータに復元
data_t restoreEmData(emData_t tEmData);
~~~


具体的な変換、復元方法は付属ライブラリの上記の関数を参照してください。別のセンサ用のライブラリを作成する場合は参考になると思います。

## 使用しているパーツリスト
参考までに私が使っている全パーツリストです。RTCのバックアップ電源についてはボタン型電池を使う場合と電気二重層コンデンサを使う場合の両方のパーツを載せていますので、必要に応じて選択します。

| 部品 | 製品名 | 単価 | 数量 | 金額（円） | 購入元 | 備考 |
| - | - | - | - | - | - | - |
| ケース | タカチ電機工業　LC135H-M3-W | 700 | 1 | 700 | MonotaRO 88206404 | 単三乾電池３個使用可。LCD窓、パネルをスライド式蓋への加工等必要 |
| マイコン | Arduino Pro Mini 3.3V 8MHz | 1243 | 1 | 1243 | スイッチサイエンス | 互換品なら400円程度で入手可 パイロットランプは無効化 |
| MicroSDスロット | 秋月電子通商 AE-MICRO-SD-DIP | 300 | 1 | 300 | 秋月電子通商 K-05488 | 記録したデータ回収用 |
| リアルタイムクロック（RTC） | 秋月電子通商 AE-RX8900 | 500 | 1 | 500 | 秋月電子通商 K-13009 | 計時用；手持ちのDS3231モジュールより消費電流遙かに小さい。 |
| EEPROM | Microchip 24FC1025-I/P | 250 | 4 | 1000 | 秋月電子通商 I-03570 | データ記録用。最大４個。データが少なければ１個でも可 |
| サーミスタ | SEMITEC株式会社 103AT-11 | 200 | 1 | 200 | 秋月電子通商 P-07257 | 他製品でも可だが、スケッチの変更が必要 |
| LCD | Strawberry LinuxI2C低電圧キャラクタ液晶モジュール（１６ｘ２行） | 720 | 1 | 720 | Strawberry Linux #27001 | 3.3V動作で低消費電力。もっと低消費電力のがあれば教えてほしい。 |
| 金属皮膜抵抗 10kΩ | 高精度　金属皮膜抵抗１／４Ｗ１０ｋΩ±０．１％ | 120 | 1 | 120 | 秋月電子通商 R-08506 | サーミスタ温度測定用 |
| カーボン抵抗 2kΩ | - | 1 | 2 | 2 | - | I2C通信のプルアップ抵抗。抵抗値は波形を見て決定 |
| セラミックコンデンサ　47uF | チップ積層セラミックコンデンサー　47μF16V X6 Ｓ3225（５個入） | 150 | 1 | 150 | 秋月電子通商 P-16078 | microSDカードの電源安定用 |
| セラミックコンデンサ　0.1uF | 絶縁ラジアルリード型積層セラミックコンデンサー　0.1μＦ５０Ｖ２．５４ｍｍ | 100 | 1 | 100 | 秋月電子通商 P-04065 | パスコン |
| セラミックコンデンサ　1uF | 絶縁ラジアルリード型積層セラミックコンデンサー　1μF50V ５ｍｍ | 20 | 1 | 20 | 秋月電子通商 P-08150 | サーミスタ温度測定安定用 |
| 小信号用汎用ダイオード | フェアチャイルド 1N4148 | 2 | 1 | 2 | 秋月電子通商 I-00941 | RTCバックアップ電池逆流防止用 |
| 電気二重層コンデンサー１．５Ｆ５．５Ｖ | SE-5R5-D155VY | 150 | 1 | 150 | 秋月電子通商 P-04250 | RTCバックアップ電源用。これならボタン電池交換の必要が無い。 |
| カーボン抵抗 100Ω | - | 1 | - | 1 | - | RTCバックアップ電源用電気二重層コンデンサーへの電流制限用 |
| XHコネクタ ベース2P | ＸＨコネクタ　ベース付ポスト　トップ型　２Ｐ　Ｂ２Ｂ－ＸＨ－Ａ（ＬＦ）（ＳＮ） | 10 | 3 | 30 | 秋月電子通商 C-12247 | サーミスタ、電池、RTC電源用 |
| XHコネクタ ハウジング2P | ＸＨコネクタ　ハウジング　２Ｐ　ＸＨＰ－２ | 5 | 3 | 15 | 秋月電子通商 C-12255 | サーミスタ、電池、RTC電源用 |
| XHコネクタ ベース5P | ＸＨコネクタ　ベース付ポスト　トップ型　５Ｐ　Ｂ５Ｂ－ＸＨ－Ａ（ＬＦ）（ＳＮ） | 15 | 1 | 15 | 秋月電子通商 C-12250 | LCD用 |
| XHコネクタ ハウジング5P | ＸＨコネクタ　ハウジング　５Ｐ　ＸＨＰ－５ | 10 | 1 | 10 | 秋月電子通商 C-12258 | LCD用 |
| XHコネクタ ハウジング用コンタクト | ＸＨコネクタ　ハウジング用コンタクト　ＳＸＨ－００１Ｔ－Ｐ０．６　（１０個入） | 30 | 2 | 60 | 秋月電子通商 C-12264 | - |
| タクトスイッチ | - | 10 | 1 | 10 | 秋月電子通商 P-03647 | - |
| シースケーブル20ｍ | VCTF0.3SQx2C等 | 500 | 1 | 500 | - | サーミスタ延長用 |
| ボタン電池ホルダ | ボタン電池基板取付用ホルダー　ＣＲ１２２０用　ＣＨ２９１－１２２０ＬＦ | 60 | 1 | 60 | 秋月電子通商 P-09561 | RTCバックアップ電源用　CR2032用でも可だが、固定しにくい。 |
| リボンケーブル | １０Ｐリボンケーブル（フラットカラーケーブル） | 60 | 1 | 60 | 秋月電子通商 C-06973 | LCD配線用他 |
| ナベタッピンねじ | 2.6x6mm | 10 | 6 | 60 | MonotaRO 41718171 | 基板、LCD固定用 |
| 3.5mmモノラルジャック | マル信無線電機 MJ164H | 75 | 1 | 75 | マルツバーツ館 | サーミスタ用 |
| .5mmモノラルプラグ L型 | マル信無線電機 MP011LN | 64 | 1 | 64 | マルツバーツ館 | サーミスタ用 |
| Ｓ端子ケーブル 3m | - | 80 | 1 | 80 | 秋月電子通商 C-11739 | I2Cセンサ接続用 半分に切って２本にして使う。 |
| ミニＤＩＮソケット（メス） | パネル取付用４ピン | 100 | 1 | 100 | 秋月電子通商 C-00174 | I2Cセンサ接続用 |
| ポリカーボネート板 厚さ2mm | 光 KPAC302-1 2 x 300 x 450 | 999 | 1 | 999 | MonotaRO 48889355 | LCD固定用 15x68mmに切って使う。 |
| アクリル板 厚さ1mm | ノーブランド　1ｘ100ｘ100 | 559 | 1 | 559 | MonotaRO 45547101 | LCD保護用 40x58mmに切って使う。 |
| クッションテープ | エーモン工業　ショッックノンテープN864 | 189 | 1 | 189 | - | LCD固定用 DAISOのクッションテープでも良い。 |
| 基板 | - | 200 | 1 | 200 | ELECROW | 自作設計を外注 |
| シリアル-USBアダプタ | FTDI USBシリアル変換アダプター Rev.2 | 1080 | 1 | 1080 | スイッチサイエンス | スケッチ転送に必要<br>3.3V給電必須  ノーブランドなら400円程度で入手可 |
|  |
## RS-274Xデータについて
Fritsingで作成したプリント基板データをRS-274X形式で出力し、製造委託先（Elecrow）に合わせてファイル名を変更したものです。他の会社では使えないかもしれません。
画像はGround Fillされていませんが、出力データはGround Fillされています。

## 今後の予定
* センサの設定によってはFLASHメモリ使用量が90%を超えます。機能追加の余地を残すためにコードを見直してメモリ節約したいところです。
* DUAL_SENSORSの処理を配列とループに変更（メモリ節約のため）
* 再コンパイルしなくてもロギング条件の設定を可能にする。ボタン１個での操作なのでちょっと面倒

## 本資料について
* 本資料の内容は無保証です。

## Releases

### 0.0.1 - Jan. 26, 2020 Test version
#### 機能はするが、ログ間隔の制約が大きく、かつセンサや機能の設定用#ifdefが多すぎる暫定版
### 1.0.0 - Jul.  5, 2021 First release センサ、機能別の処理をライブラリに分離。データ保存、書き出しコードを整理、改良
### 1.0.1 - Aug.  6, 2021 Bug fix
### 1.1.0 - Oct.  8, 2021 Bosch BME280対応，細かな修正
### 1.2.0 - Oct. 24, 2021 ロギング設定表示追加、microSDへの書出し修正（メモリ一周したときの動作改善、Bug fix）
### 1.2.1 - Oct. 27, 2021 WRITE ONCEモードメモリ最後での書込み不正、メモリ一周後の判定不正 fix
### 1.2.2 - Oct. 28, 2021 ENDLESSモードで２周目先頭がヘッダの時のデータ読出し漏れ漏れ fix
