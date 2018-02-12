//
// Copyright (c) 2016-2018 by Cornell University.  All Rights Reserved.
//
// Permission to use the "TownCrier" software ("TownCrier"), officially
// docketed at the Center for Technology Licensing at Cornell University
// as D-7364, developed through research conducted at Cornell University,
// and its associated copyrights solely for educational, research and
// non-profit purposes without fee is hereby granted, provided that the
// user agrees as follows:
//
// The permission granted herein is solely for the purpose of compiling
// the TownCrier source code. No other rights to use TownCrier and its
// associated copyrights for any other purpose are granted herein,
// whether commercial or non-commercial.
//
// Those desiring to incorporate TownCrier software into commercial
// products or use TownCrier and its associated copyrights for commercial
// purposes must contact the Center for Technology Licensing at Cornell
// University at 395 Pine Tree Road, Suite 310, Ithaca, NY 14850; email:
// ctl-connect@cornell.edu; Tel: 607-254-4698; FAX: 607-254-5454 for a
// commercial license.
//
// IN NO EVENT SHALL CORNELL UNIVERSITY BE LIABLE TO ANY PARTY FOR
// DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES,
// INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF TOWNCRIER AND ITS
// ASSOCIATED COPYRIGHTS, EVEN IF CORNELL UNIVERSITY MAY HAVE BEEN
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// THE WORK PROVIDED HEREIN IS ON AN "AS IS" BASIS, AND CORNELL
// UNIVERSITY HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
// ENHANCEMENTS, OR MODIFICATIONS.  CORNELL UNIVERSITY MAKES NO
// REPRESENTATIONS AND EXTENDS NO WARRANTIES OF ANY KIND, EITHER IMPLIED
// OR EXPRESS, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, OR THAT THE USE
// OF TOWNCRIER AND ITS ASSOCIATED COPYRIGHTS WILL NOT INFRINGE ANY
// PATENT, TRADEMARK OR OTHER RIGHTS.
//
// TownCrier was developed with funding in part by the National Science
// Foundation (NSF grants CNS-1314857, CNS-1330599, CNS-1453634,
// CNS-1518765, CNS-1514261), a Packard Fellowship, a Sloan Fellowship,
// Google Faculty Research Awards, and a VMWare Research Award.
//

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