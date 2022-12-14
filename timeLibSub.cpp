#include <Arduino.h>
#include "timeLibSub.h"

//////////////////////////////////////////////////////////////////////////////
// 以下はヘッダからの経過時間算出用なので、余り長い間隔には対応していない。必要ならlongにする。
//////////////////////////////////////////////////////////////////////////////
int daysDiff(tmElements_t sTm, tmElements_t eTm) { // 正確な日数は年の差1までしか対応していない。それ以上だと閏年で誤差が出る。最大89年
  return (dayOfYear(eTm.Year, eTm.Month, eTm.Day) - dayOfYear(sTm.Year, sTm.Month, sTm.Day)) + ((int)eTm.Year - (int)sTm.Year) * daysInYear(tmYearToCalendar(sTm.Year));
}


long hourDiff(tmElements_t sTm, tmElements_t eTm) { // EEPROMを一周したときに日時の逆転を正しく判定できるようにlongとした。intだと一周で1365日を超えると誤動作する。
  return (long)(daysDiff(sTm, eTm) * 24 - (long)sTm.Hour) + (long)eTm.Hour;
}


int minDiff(tmElements_t sTm, tmElements_t eTm) {  //　最大22日なので、ヘッダの書込み間隔がこれを超えないようにする。
  return (hourDiff(sTm, eTm) * 60 - (int)sTm.Minute) + (int)eTm.Minute;
}


int secDiff(tmElements_t sTm, tmElements_t eTm) { // 最大9時間なので、ヘッダの書込み間隔がこれを超えないようにする。
  return (minDiff(sTm, eTm) * 60 - (int)sTm.Second) + (int)eTm.Second;
}

/*
long minDiff(tmElements_t sTm, tmElements_t eTm) {  //　longにすれば事実上制限はなくなる。
  return (long)(hourDiff(sTm, eTm) * 60 - (long)sTm.Minute) + (long)eTm.Minute;
}

long secDiff(tmElements_t sTm, tmElements_t eTm) {
  return (minDiff(sTm, eTm) * 60 - (long)sTm.Second) + (long)eTm.Second;
}
*/

int daysInYear(int year) {
  if (year % 4) return 366;
  return 365;
}


boolean timeMinuteEqual(tmElements_t tm, tmElements_t tmLast) {
  return tm.Year  == tmLast.Year  &&
         tm.Month == tmLast.Month &&
         tm.Day   == tmLast.Day   &&  
         tm.Hour  == tmLast.Hour  &&  
         tm.Minute== tmLast.Minute;
}

/////////////////////////////////////////////////////////
//       calculate days from the begining of the year   //
/////////////////////////////////////////////////////////
// http://radiopench.blog96.fc2.com/blog-entry-735.html?sp&m2=res was modified
int dayOfYear(int yy, byte mm, byte dd) {

  int daySum[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334}; //the day of the year number for each month 
  if (!(yy % 4)) {  // leap year
    for (byte i = 2; i < 12; i++) {
      daySum[i]++;
    }
  }
  return daySum[mm - 1] + dd - 1; // return days from the begging of the year (1 Jan. = 0)
}

/////////////////////////////////////////////////////////
//    increment time specified minutes                 //
/////////////////////////////////////////////////////////

tmElements_t incTimeHours(tmElements_t tm, int intervalHours) {
  int tHour = tm.Hour + intervalHours;
  tm.Day  += tHour / 24;
  tm.Hour  = tHour % 24;
  if (tm.Day > daysInMonth(tm)) {
    tm.Day = 1;
    tm.Month++;
    if (tm.Month > 12) {
      tm.Month = 1;
      tm.Year++;
    }
  }
  return tm;
}

/*
tmElements_t incTimeMinutes(tmElements_t tm, byte intervalMin, boolean regularMinute) {
  if (regularMinute) {
    tm.Minute = (((int)tm.Minute / (int)intervalMin) + 1) * intervalMin;
  } else {
    tm.Minute += intervalMin;
  }
  byte incHours  = tm.Minute / 60;
  tm.Minute = tm.Minute % 60;
  return incTimeHours(tm, incHours);
}
*/

tmElements_t incTimeMinutes(tmElements_t tm, int intervalMin) {
  int tMinute = (int)tm.Minute + intervalMin;
  int incHours  = tMinute / 60;
  tm.Minute = tMinute % 60;
  return incTimeHours(tm, incHours);
}


tmElements_t incTimeSeconds(tmElements_t tm, int intervalSec) {
  int tSecond = (int)tm.Second + intervalSec;
  int incMinutes = tSecond / 60;
  tm.Second = tSecond % 60;
  return incTimeMinutes(tm, incMinutes);
}


byte daysInMonth(tmElements_t tm) {
  byte days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}; //days of eath month
  if (!((tmYearToCalendar(tm.Year)) % 4)) {   // leap year
    days[1] = 29;
  }
  return days[tm.Month - 1];
}
