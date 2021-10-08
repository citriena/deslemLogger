//////////////////////////////////////////////////////
// 電池長期間駆動Arduinoロガー
// deslemLogger (deep sleep EEPROM logger)用補助スケッチ
// わかりやすいようにメインのスケッチから日時処理を分離
//////////////////////////////////////////////////////

#ifndef _timeLibSub_h
#define _timeLibSub_h
#include <TimeLib.h>            // https://github.com/PaulStoffregen/Time


int daysDiff(tmElements_t sTm, tmElements_t eTm);
int hourDiff(tmElements_t sTm, tmElements_t eTm);
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
