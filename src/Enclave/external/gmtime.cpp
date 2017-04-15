/*
 * gmtime - convert the calendar time into broken down time
 */
/* $Header: gmtime.c,v 1.4 91/04/22 13:20:27 ceriel Exp $ */

#include <time.h>
#include "loc_time.h"
#include "gmtime.h"

const int _ytab[2][12] = {
    {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
    {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
};

void gmtime_tc(const time_t *timer, tc::ext::date &date) {
  time_t time = *timer;
  unsigned long dayclock, dayno;
  int year = EPOCH_YR;

  struct tm btime;
  struct tm *timep = &btime;

  dayclock = (unsigned long) time % SECS_DAY;
  dayno = (unsigned long) time / SECS_DAY;

  timep->tm_sec = dayclock % 60;
  timep->tm_min = (dayclock % 3600) / 60;
  timep->tm_hour = dayclock / 3600;
  timep->tm_wday = (dayno + 4) % 7;       /* day 0 was a thursday */
  while (dayno >= YEARSIZE(year)) {
    dayno -= YEARSIZE(year);
    year++;
  }
  timep->tm_year = year - YEAR0;
  timep->tm_yday = dayno;
  timep->tm_mon = 0;
  while (dayno >= _ytab[LEAPYEAR(year)][timep->tm_mon]) {
    dayno -= _ytab[LEAPYEAR(year)][timep->tm_mon];
    timep->tm_mon++;
  }
  timep->tm_mday = dayno + 1;
  timep->tm_isdst = 0;

  date.year = timep->tm_year + YEAR0;
  date.month = timep->tm_mon + 1;
  date.day = timep->tm_mday;
}