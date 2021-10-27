//////////////////////////////////////////////////////
// 電池長期間駆動Arduinoロガー
// deslemLogger (deep sleep EEPROM logger)用補助スケッチ
// わかりやすいようにメインのスケッチから日時処理を分離
// 2038年問題を避けるため、UNIX時間は使わない。Arduinoはlongまでしか使えない。unsigned longにしてもよいが引算がやっかい。
// 時刻にはtmElements_t型を使っている。これはTimeLib.hで定義されている。
// 年には1970年からの経過年数をⅠバイトで管理している。このため、1970+255=2225年まで対応
// ただし、ところどころで西暦を2桁で扱っているので実質的に2099年までしか対応しない。
// このプログラムがいつまで使用されるかわからないが、十分であろう。
//////////////////////////////////////////////////////

#ifndef _timeLibSub_h
#define _timeLibSub_h
#include <TimeLib.h>            // https://github.com/PaulStoffregen/Time


//////////////////////////////////////////////////////////////////////////////
// 以下はヘッダからの経過時間算出用なので、余り長い間隔には対応していない。必要ならlongにする。
//////////////////////////////////////////////////////////////////////////////
int daysDiff(tmElements_t sTm, tmElements_t eTm);
long hourDiff(tmElements_t sTm, tmElements_t eTm);  // ログファイル書出し時対応のためこれだけはlong
int minDiff(tmElements_t sTm, tmElements_t eTm);
int secDiff(tmElements_t sTm, tmElements_t eTm);

int daysInYear(int year);
boolean timeMinuteEqual(tmElements_t tm, tmElements_t tmLast);
int dayOfYear(int yy, byte mm, byte dd);
tmElements_t incTimeHours(tmElements_t tm, int intervalHours);
tmElements_t incTimeMinutes(tmElements_t tm, int intervalMin);
tmElements_t incTimeSeconds(tmElements_t tm, int intervalSec);
byte daysInMonth(tmElements_t tm);

#endif
