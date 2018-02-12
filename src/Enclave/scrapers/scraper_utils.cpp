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

#include "scraper_utils.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <string>

using std::string;

/* Converts a hex character to its integer value */
static char from_hex(char ch) {
  return isdigit(ch) ? ch - '0' : tolower(ch) - 'a' + 10;
}

/* Converts an integer value to its hex character*/
static char to_hex(char code) {
  static char hex[] = "0123456789ABCDEF";
  return hex[code & 15];
}

/* Returns a url-encoded version of str */
string url_encode(const char *str) {
  const char *pstr = str;
  char *buf = reinterpret_cast<char *>(malloc(strlen(str) * 3 + 1)), *pbuf = buf;
  while (*pstr) {
    if (isalnum(*pstr) || *pstr == '-' || *pstr == '_' || *pstr == '.' || *pstr == '~' || *pstr == '*')
      *pbuf++ = *pstr;
    else if (*pstr == ' ')
      *pbuf++ = '%', *pbuf++ = '2', *pbuf++ = '0';
    else
      *pbuf++ = '%', *pbuf++ = to_hex(*pstr >> 4), *pbuf++ = to_hex(*pstr & 15);
    pstr++;
  }
  *pbuf = '\0';

  string ret = string(buf);
  free(buf);
  return ret;
}

/* Returns a url-decoded version of str */
string url_decode(const char *str) {
  const char *pstr = str;
  char *buf = reinterpret_cast<char *>(malloc(strlen(str) + 1)), *pbuf = buf;
  while (*pstr) {
    if (*pstr == '%') {
      if (pstr[1] && pstr[2]) {
        *pbuf++ = from_hex(pstr[1]) << 4 | from_hex(pstr[2]);
        pstr += 2;
      }
    } else if (*pstr == '+') {
      *pbuf++ = ' ';
    } else {
      *pbuf++ = *pstr;
    }
    pstr++;
  }
  *pbuf = '\0';

  string ret = string(buf);
  free(buf);
  return ret;
}
