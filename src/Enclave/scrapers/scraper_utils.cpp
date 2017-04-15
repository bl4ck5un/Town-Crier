//
// Created by fanz on 4/12/17.
//

#include "scraper_utils.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <string>

using namespace std;

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
  char *buf = (char*) malloc(strlen(str) * 3 + 1), *pbuf = buf;
  while (*pstr) {
    if (isalnum(*pstr) || *pstr == '-' || *pstr == '_' || *pstr == '.' || *pstr == '~' || *pstr == '*')
      *pbuf++ = *pstr;
    else if (*pstr == ' ')
      *pbuf++ = '%',*pbuf++ = '2', *pbuf++ = '0';
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
  char *buf = (char*) malloc(strlen(str) + 1), *pbuf = buf;
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