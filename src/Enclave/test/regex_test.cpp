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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <debug.h>
#include "scrapers/scrapers.h"
#include "../log.h"
#include "external/slre.h"

static int static_total_tests = 0;
static int static_failed_tests = 0;

#define FAIL(str, line) do {                      \
  LL_INFO("Fail on line %d: [%s]", line, str);   \
  static_failed_tests++;                          \
} while (0)

#define ASSERT(expr) do {               \
  static_total_tests++;                 \
  if (!(expr)) FAIL(#expr, __LINE__);   \
} while (0)

/* Regex must have exactly one bracket pair */
static char *slre_replace(const char *regex, const char *buf,
                          const char *sub) {
  char *s = NULL;
  int n, n1, n2, n3, s_len, len = strlen(buf);
  struct slre_cap cap = { NULL, 0 };

  do {
    s_len = s == NULL ? 0 : strlen(s);
    if ((n = slre_match(regex, buf, len, &cap, 1, 0)) > 0) {
      n1 = cap.ptr - buf, n2 = strlen(sub),
         n3 = &buf[n] - &cap.ptr[cap.len];
    } else {
      n1 = len, n2 = 0, n3 = 0;
    }
    s = (char *) realloc(s, s_len + n1 + n2 + n3 + 1);
    memcpy(s + s_len, buf, n1);
    memcpy(s + s_len + n1, sub, n2);
    memcpy(s + s_len + n1 + n2, cap.ptr + cap.len, n3);
    s[s_len + n1 + n2 + n3] = '\0';

    buf += n > 0 ? n : len;
    len -= n > 0 ? n : len;
  } while (len > 0);

  return s;
}

extern "C" int regex_self_test();
int regex_self_test() {
  struct slre_cap caps[10];

  /* Metacharacters */
  ASSERT(slre_match("$", "abcd", 4, NULL, 0, 0) == 4);
  ASSERT(slre_match("^", "abcd", 4, NULL, 0, 0) == 0);
  ASSERT(slre_match("x|^", "abcd", 4, NULL, 0, 0) == 0);
  ASSERT(slre_match("x|$", "abcd", 4, NULL, 0, 0) == 4);
  ASSERT(slre_match("x", "abcd", 4, NULL, 0, 0) == SLRE_NO_MATCH);
  ASSERT(slre_match(".", "abcd", 4, NULL, 0, 0) == 1);
  ASSERT(slre_match("^.*\\\\.*$", "c:\\Tools", 8, NULL, 0, SLRE_IGNORE_CASE)
    == 8);
  ASSERT(slre_match("\\", "a", 1, NULL, 0, 0) == SLRE_INVALID_METACHARACTER);
  ASSERT(slre_match("\\x", "a", 1, NULL, 0, 0) == SLRE_INVALID_METACHARACTER);
  ASSERT(slre_match("\\x1", "a", 1, NULL, 0, 0) == SLRE_INVALID_METACHARACTER);
  ASSERT(slre_match("\\x20", " ", 1, NULL, 0, 0) == 1);

  ASSERT(slre_match("^.+$", "", 0, NULL, 0, 0) == SLRE_NO_MATCH);
  ASSERT(slre_match("^(.+)$", "", 0, NULL, 0, 0) == SLRE_NO_MATCH);
  ASSERT(slre_match("^([\\+-]?)([\\d]+)$", "+", 1,
                    caps, 10, SLRE_IGNORE_CASE) == SLRE_NO_MATCH);
  ASSERT(slre_match("^([\\+-]?)([\\d]+)$", "+27", 3,
                    caps, 10, SLRE_IGNORE_CASE) == 3);
  ASSERT(caps[0].len == 1);
  ASSERT(caps[0].ptr[0] == '+');
  ASSERT(caps[1].len == 2);
  ASSERT(memcmp(caps[1].ptr, "27", 2) == 0);

  ASSERT(slre_match("tel:\\+(\\d+[\\d-]+\\d)",
                    "tel:+1-201-555-0123;a=b", 23, caps, 10, 0) == 19);
  ASSERT(caps[0].len == 14);
  ASSERT(memcmp(caps[0].ptr, "1-201-555-0123", 14) == 0);

  /* Character sets */
  ASSERT(slre_match("[abc]", "1c2", 3, NULL, 0, 0) == 2);
  ASSERT(slre_match("[abc]", "1C2", 3, NULL, 0, 0) == SLRE_NO_MATCH);
  ASSERT(slre_match("[abc]", "1C2", 3, NULL, 0, SLRE_IGNORE_CASE) == 2);
  ASSERT(slre_match("[.2]", "1C2", 3, NULL, 0, 0) == 1);
  ASSERT(slre_match("[\\S]+", "ab cd", 5, NULL, 0, 0) == 2);
  ASSERT(slre_match("[\\S]+\\s+[tyc]*", "ab cd", 5, NULL, 0, 0) == 4);
  ASSERT(slre_match("[\\d]", "ab cd", 5, NULL, 0, 0) == SLRE_NO_MATCH);
  ASSERT(slre_match("[^\\d]", "ab cd", 5, NULL, 0, 0) == 1);
  ASSERT(slre_match("[^\\d]+", "abc123", 6, NULL, 0, 0) == 3);
  ASSERT(slre_match("[1-5]+", "123456789", 9, NULL, 0, 0) == 5);
  ASSERT(slre_match("[1-5a-c]+", "123abcdef", 9, NULL, 0, 0) == 6);
  ASSERT(slre_match("[1-5a-]+", "123abcdef", 9, NULL, 0, 0) == 4);
  ASSERT(slre_match("[1-5a-]+", "123a--2oo", 9, NULL, 0, 0) == 7);
  ASSERT(slre_match("[htps]+://", "https://", 8, NULL, 0, 0) == 8);
  ASSERT(slre_match("[^\\s]+", "abc def", 7, NULL, 0, 0) == 3);
  ASSERT(slre_match("[^fc]+", "abc def", 7, NULL, 0, 0) == 2);
  ASSERT(slre_match("[^d\\sf]+", "abc def", 7, NULL, 0, 0) == 3);

  /* Flags - case sensitivity */
  ASSERT(slre_match("FO", "foo", 3, NULL, 0, 0) == SLRE_NO_MATCH);
  ASSERT(slre_match("FO", "foo", 3, NULL, 0, SLRE_IGNORE_CASE) == 2);
  ASSERT(slre_match("(?m)FO", "foo", 3, NULL, 0, 0) ==
    SLRE_UNEXPECTED_QUANTIFIER);
  ASSERT(slre_match("(?m)x", "foo", 3, NULL, 0, 0) ==
    SLRE_UNEXPECTED_QUANTIFIER);

  ASSERT(slre_match("fo", "foo", 3, NULL, 0, 0) == 2);
  ASSERT(slre_match(".+", "foo", 3, NULL, 0, 0) == 3);
  ASSERT(slre_match(".+k", "fooklmn", 7, NULL, 0, 0) == 4);
  ASSERT(slre_match(".+k.", "fooklmn", 7, NULL, 0, 0) == 5);
  ASSERT(slre_match("p+", "fooklmn", 7, NULL, 0, 0) == SLRE_NO_MATCH);
  ASSERT(slre_match("ok", "fooklmn", 7, NULL, 0, 0) == 4);
  ASSERT(slre_match("lmno", "fooklmn", 7, NULL, 0, 0) == SLRE_NO_MATCH);
  ASSERT(slre_match("mn.", "fooklmn", 7, NULL, 0, 0) == SLRE_NO_MATCH);
  ASSERT(slre_match("o", "fooklmn", 7, NULL, 0, 0) == 2);
  ASSERT(slre_match("^o", "fooklmn", 7, NULL, 0, 0) == SLRE_NO_MATCH);
  ASSERT(slre_match("^", "fooklmn", 7, NULL, 0, 0) == 0);
  ASSERT(slre_match("n$", "fooklmn", 7, NULL, 0, 0) == 7);
  ASSERT(slre_match("n$k", "fooklmn", 7, NULL, 0, 0) == SLRE_NO_MATCH);
  ASSERT(slre_match("l$", "fooklmn", 7, NULL, 0, 0) == SLRE_NO_MATCH);
  ASSERT(slre_match(".$", "fooklmn", 7, NULL, 0, 0) == 7);
  ASSERT(slre_match("a?", "fooklmn", 7, NULL, 0, 0) == 0);
  ASSERT(slre_match("^a*CONTROL", "CONTROL", 7, NULL, 0, 0) == 7);
  ASSERT(slre_match("^[a]*CONTROL", "CONTROL", 7, NULL, 0, 0) == 7);
  ASSERT(slre_match("^(a*)CONTROL", "CONTROL", 7, NULL, 0, 0) == 7);
  ASSERT(slre_match("^(a*)?CONTROL", "CONTROL", 7, NULL, 0, 0) == 7);

  ASSERT(slre_match("\\_", "abc", 3, NULL, 0, 0) == SLRE_INVALID_METACHARACTER);
  ASSERT(slre_match("+", "fooklmn", 7, NULL, 0, 0) == SLRE_UNEXPECTED_QUANTIFIER);
  ASSERT(slre_match("()+", "fooklmn", 7, NULL, 0, 0) == SLRE_NO_MATCH);
  ASSERT(slre_match("\\x", "12", 2, NULL, 0, 0) == SLRE_INVALID_METACHARACTER);
  ASSERT(slre_match("\\xhi", "12", 2, NULL, 0, 0) == SLRE_INVALID_METACHARACTER);
  ASSERT(slre_match("\\x20", "_ J", 3, NULL, 0, 0) == 2);
  ASSERT(slre_match("\\x4A", "_ J", 3, NULL, 0, 0) == 3);
  ASSERT(slre_match("\\d+", "abc123def", 9, NULL, 0, 0) == 6);

  /* Balancing brackets */
  ASSERT(slre_match("(x))", "fooklmn", 7, NULL, 0, 0) == SLRE_UNBALANCED_BRACKETS);
  ASSERT(slre_match("(", "fooklmn", 7, NULL, 0, 0) == SLRE_UNBALANCED_BRACKETS);

  ASSERT(slre_match("klz?mn", "fooklmn", 7, NULL, 0, 0) == 7);
  ASSERT(slre_match("fa?b", "fooklmn", 7, NULL, 0, 0) == SLRE_NO_MATCH);

  /* Brackets & capturing */
  ASSERT(slre_match("^(te)", "tenacity subdues all", 20, caps, 10, 0) == 2);
  ASSERT(slre_match("(bc)", "abcdef", 6, caps, 10, 0) == 3);
  ASSERT(slre_match(".(d.)", "abcdef", 6, caps, 10, 0) == 5);
  ASSERT(slre_match(".(d.)\\)?", "abcdef", 6, caps, 10, 0) == 5);
  ASSERT(caps[0].len == 2);
  ASSERT(memcmp(caps[0].ptr, "de", 2) == 0);
  ASSERT(slre_match("(.+)", "123", 3, caps, 10, 0) == 3);
  ASSERT(slre_match("(2.+)", "123", 3, caps, 10, 0) == 3);
  ASSERT(caps[0].len == 2);
  ASSERT(memcmp(caps[0].ptr, "23", 2) == 0);
  ASSERT(slre_match("(.+2)", "123", 3, caps, 10, 0) == 2);
  ASSERT(caps[0].len == 2);
  ASSERT(memcmp(caps[0].ptr, "12", 2) == 0);
  ASSERT(slre_match("(.*(2.))", "123", 3, caps, 10, 0) == 3);
  ASSERT(slre_match("(.)(.)", "123", 3, caps, 10, 0) == 2);
  ASSERT(slre_match("(\\d+)\\s+(\\S+)", "12 hi", 5, caps, 10, 0) == 5);
  ASSERT(slre_match("ab(cd)+ef", "abcdcdef", 8, NULL, 0, 0) == 8);
  ASSERT(slre_match("ab(cd)*ef", "abcdcdef", 8, NULL, 0, 0) == 8);
  ASSERT(slre_match("ab(cd)+?ef", "abcdcdef", 8, NULL, 0, 0) == 8);
  ASSERT(slre_match("ab(cd)+?.", "abcdcdef", 8, NULL, 0, 0) == 5);
  ASSERT(slre_match("ab(cd)?", "abcdcdef", 8, NULL, 0, 0) == 4);
  ASSERT(slre_match("a(b)(cd)", "abcdcdef", 8, caps, 1, 0) ==
      SLRE_CAPS_ARRAY_TOO_SMALL);
  ASSERT(slre_match("(.+/\\d+\\.\\d+)\\.jpg$", "/foo/bar/12.34.jpg", 18,
                    caps, 1, 0) == 18);
  ASSERT(slre_match("(ab|cd).*\\.(xx|yy)", "ab.yy", 5, NULL, 0, 0) == 5);
  ASSERT(slre_match(".*a", "abcdef", 6, NULL, 0, 0) == 1);
  ASSERT(slre_match("(.+)c", "abcdef", 6, NULL, 0, 0) == 3);
  ASSERT(slre_match("\\n", "abc\ndef", 7, NULL, 0, 0) == 4);
  ASSERT(slre_match("b.\\s*\\n", "aa\r\nbb\r\ncc\r\n\r\n", 14,
                    caps, 10, 0) == 8);

  /* Greedy vs non-greedy */
  ASSERT(slre_match(".+c", "abcabc", 6, NULL, 0, 0) == 6);
  ASSERT(slre_match(".+?c", "abcabc", 6, NULL, 0, 0) == 3);
  ASSERT(slre_match(".*?c", "abcabc", 6, NULL, 0, 0) == 3);
  ASSERT(slre_match(".*c", "abcabc", 6, NULL, 0, 0) == 6);
  ASSERT(slre_match("bc.d?k?b+", "abcabc", 6, NULL, 0, 0) == 5);

  /* Branching */
  ASSERT(slre_match("|", "abc", 3, NULL, 0, 0) == 0);
  ASSERT(slre_match("|.", "abc", 3, NULL, 0, 0) == 1);
  ASSERT(slre_match("x|y|b", "abc", 3, NULL, 0, 0) == 2);
  ASSERT(slre_match("k(xx|yy)|ca", "abcabc", 6, NULL, 0, 0) == 4);
  ASSERT(slre_match("k(xx|yy)|ca|bc", "abcabc", 6, NULL, 0, 0) == 3);
  ASSERT(slre_match("(|.c)", "abc", 3, caps, 10, 0) == 3);
  ASSERT(caps[0].len == 2);
  ASSERT(memcmp(caps[0].ptr, "bc", 2) == 0);
  ASSERT(slre_match("a|b|c", "a", 1, NULL, 0, 0) == 1);
  ASSERT(slre_match("a|b|c", "b", 1, NULL, 0, 0) == 1);
  ASSERT(slre_match("a|b|c", "c", 1, NULL, 0, 0) == 1);
  ASSERT(slre_match("a|b|c", "d", 1, NULL, 0, 0) == SLRE_NO_MATCH);

  /* Optional match at the end of the string */
  ASSERT(slre_match("^.*c.?$", "abc", 3, NULL, 0, 0) == 3);
  ASSERT(slre_match("^.*C.?$", "abc", 3, NULL, 0, SLRE_IGNORE_CASE) == 3);
  ASSERT(slre_match("bk?", "ab", 2, NULL, 0, 0) == 2);
  ASSERT(slre_match("b(k?)", "ab", 2, NULL, 0, 0) == 2);
  ASSERT(slre_match("b[k-z]*", "ab", 2, NULL, 0, 0) == 2);
  ASSERT(slre_match("ab(k|z|y)*", "ab", 2, NULL, 0, 0) == 2);
  ASSERT(slre_match("[b-z].*", "ab", 2, NULL, 0, 0) == 2);
  ASSERT(slre_match("(b|z|u).*", "ab", 2, NULL, 0, 0) == 2);
  ASSERT(slre_match("ab(k|z|y)?", "ab", 2, NULL, 0, 0) == 2);
  ASSERT(slre_match(".*", "ab", 2, NULL, 0, 0) == 2);
  ASSERT(slre_match(".*$", "ab", 2, NULL, 0, 0) == 2);
  ASSERT(slre_match("a+$", "aa", 2, NULL, 0, 0) == 2);
  ASSERT(slre_match("a*$", "aa", 2, NULL, 0, 0) == 2);
  ASSERT(slre_match( "a+$" ,"Xaa", 3, NULL, 0, 0) == 3);
  ASSERT(slre_match( "a*$" ,"Xaa", 3, NULL, 0, 0) == 3);

  /* Ignore case flag */
  ASSERT(slre_match("[a-h]+", "abcdefghxxx", 11, NULL, 0, 0) == 8);
  ASSERT(slre_match("[A-H]+", "ABCDEFGHyyy", 11, NULL, 0, 0) == 8);
  ASSERT(slre_match("[a-h]+", "ABCDEFGHyyy", 11, NULL, 0, 0) == SLRE_NO_MATCH);
  ASSERT(slre_match("[A-H]+", "abcdefghyyy", 11, NULL, 0, 0) == SLRE_NO_MATCH);
  ASSERT(slre_match("[a-h]+", "ABCDEFGHyyy", 11, NULL, 0, SLRE_IGNORE_CASE) == 8);
  ASSERT(slre_match("[A-H]+", "abcdefghyyy", 11, NULL, 0, SLRE_IGNORE_CASE) == 8);

  {
    /* Example: HTTP request */
    const char *request = " GET /index.html HTTP/1.0\r\n\r\n";
    struct slre_cap caps[4];

    if (slre_match("^\\s*(\\S+)\\s+(\\S+)\\s+HTTP/(\\d)\\.(\\d)",
                   request, strlen(request), caps, 4, 0) > 0) {
      LL_LOG("Method: [%.*s], URI: [%.*s]",
             caps[0].len, caps[0].ptr,
             caps[1].len, caps[1].ptr);
    } else {
      LL_CRITICAL("Error parsing [%s]\n", request);
    }

    ASSERT(caps[1].len == 11);
    ASSERT(memcmp(caps[1].ptr, "/index.html", caps[1].len) == 0);
  }

  {
    /* Example: string replacement */
    char *s = slre_replace("({{.+?}})",
                           "Good morning, {{foo}}. How are you, {{bar}}?",
                           "Bob");
    ASSERT(strcmp(s, "Good morning, Bob. How are you, Bob?") == 0);
    free(s);
  }

  {
    /* Example: find all URLs in a string */
    static const char *str =
      "<img src=\"HTTPS://FOO.COM/x?b#c=tab1\"/> "
      "  <a href=\"http://cesanta.com\">some link</a>";

    static const char *regex = "((https?://)[^\\s/'\"<>]+/?[^\\s'\"<>]*)";
    struct slre_cap caps[2];
    int i, j = 0, str_len = (int) strlen(str);

    while (j < str_len &&
           (i = slre_match(regex, str + j, str_len - j, caps, 2, SLRE_IGNORE_CASE)) > 0) {
      LL_LOG("Found URL: [%.*s]", caps[0].len, caps[0].ptr);
      j += i;
    }
  }

  {
    /* Example more complex regular expression */
    static const char * str = "aa 1234 xy\nxyz";
    static const char * regex = "aa ([0-9]*) *([x-z]*)\\s+xy([yz])";
    struct slre_cap caps[3];
    ASSERT(slre_match(regex, str, strlen(str), caps, 3, 0) > 0);
    ASSERT(caps[0].len == 4);
    ASSERT(caps[1].len == 2);
    ASSERT(caps[2].len == 1);
    ASSERT(caps[2].ptr[0] == 'z');
  }

  return static_failed_tests == 0 ? 0 : -1;
}
