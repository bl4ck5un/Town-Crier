#ifndef TOWN_CRIER_LOC_TIME_H_H
#define TOWN_CRIER_LOC_TIME_H_H

/*
2  * loc_time.h - some local definitions
3  */
/* $Header: /users/cosc/staff/paul/CVS/minix1.7/src/lib/ansi/loc_time.h,v 1.2 1996/04/10 21:04:30 paul Exp $ */

#define YEAR0           1900                    /* the first year */
#define EPOCH_YR        1970            /* EPOCH = Jan 1 1970 00:00:00 */
#define SECS_DAY        (24L * 60L * 60L)
#define LEAPYEAR(year)  (!((year) % 4) && (((year) % 100) || !((year) % 400)))
#define YEARSIZE(year)  (LEAPYEAR(year) ? 366 : 365)
#define FIRSTSUNDAY(timp)       (((timp)->tm_yday - (timp)->tm_wday + 420) % 7)
#define FIRSTDAYOF(timp)        (((timp)->tm_wday - (timp)->tm_yday + 420) % 7)
#define TIME_MAX        ULONG_MAX
#define ABB_LEN         3

extern const int _ytab[2][12];
extern const char *_days[];
extern const char *_months[];

void _tzset(void);
unsigned _dstget(struct tm *timep);

extern long _timezone;
extern long _dst_off;
extern int _daylight;
extern char *_tzname[2];

#endif //TOWN_CRIER_LOC_TIME_H_H
