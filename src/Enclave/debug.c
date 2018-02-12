/* * Copyright (c) 2016-2018 by Cornell University.  All Rights Reserved.
 *
 * Permission to use the "TownCrier" software ("TownCrier"), officially
 * docketed at the Center for Technology Licensing at Cornell University
 * as D-7364, developed through research conducted at Cornell University,
 * and its associated copyrights solely for educational, research and
 * non-profit purposes without fee is hereby granted, provided that the
 * user agrees as follows:
 *
 * The permission granted herein is solely for the purpose of compiling
 * the TownCrier source code. No other rights to use TownCrier and its
 * associated copyrights for any other purpose are granted herein,
 * whether commercial or non-commercial.
 *
 * Those desiring to incorporate TownCrier software into commercial
 * products or use TownCrier and its associated copyrights for commercial
 * purposes must contact the Center for Technology Licensing at Cornell
 * University at 395 Pine Tree Road, Suite 310, Ithaca, NY 14850; email:
 * ctl-connect@cornell.edu; Tel: 607-254-4698; FAX: 607-254-5454 for a
 * commercial license.
 *
 * IN NO EVENT SHALL CORNELL UNIVERSITY BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES,
 * INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF TOWNCRIER AND ITS
 * ASSOCIATED COPYRIGHTS, EVEN IF CORNELL UNIVERSITY MAY HAVE BEEN
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * THE WORK PROVIDED HEREIN IS ON AN "AS IS" BASIS, AND CORNELL
 * UNIVERSITY HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
 * ENHANCEMENTS, OR MODIFICATIONS.  CORNELL UNIVERSITY MAKES NO
 * REPRESENTATIONS AND EXTENDS NO WARRANTIES OF ANY KIND, EITHER IMPLIED
 * OR EXPRESS, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, OR THAT THE USE
 * OF TOWNCRIER AND ITS ASSOCIATED COPYRIGHTS WILL NOT INFRINGE ANY
 * PATENT, TRADEMARK OR OTHER RIGHTS.
 *
 * TownCrier was developed with funding in part by the National Science
 * Foundation (NSF grants CNS-1314857, CNS-1330599, CNS-1453634,
 * CNS-1518765, CNS-1514261), a Packard Fellowship, a Sloan Fellowship,
 * Google Faculty Research Awards, and a VMWare Research Award.
 */

#include "debug.h"
#include "string.h"
#include "glue.h"

#define MIN(x, y) (x < y ? x : y)

void dump_buf(const char *title, const unsigned char *buf, size_t len) {
  hexdump(title, buf, len);
}

void hexdump(const char *title, void const *data, size_t len) {
  unsigned int i;
  unsigned int r, c;

  if (!data)
    return;

  printf_sgx("%s\n", title);

  for (r = 0, i = 0; r < (len / 16 + (len % 16 != 0)); r++, i += 16) {
    printf_sgx("0x%04X:   ", i); /* location of first byte in line */

    for (c = i; c < i + 8; c++) /* left half of hex dump */
      if (c < len)
        printf_sgx("%02X ", ((unsigned char const *) data)[c]);
      else
        printf_sgx("   "); /* pad if short line */

    printf_sgx("  ");

    for (c = i + 8; c < i + 16; c++) /* right half of hex dump */
      if (c < len)
        printf_sgx("%02X ", ((unsigned char const *) data)[c]);
      else
        printf_sgx("   "); /* pad if short line */

    printf_sgx("   ");

    for (c = i; c < i + 16; c++) /* ASCII dump */
      if (c < len)
        if (((unsigned char const *) data)[c] >= 32 &&
            ((unsigned char const *) data)[c] < 127)
          printf_sgx("%c", ((char const *) data)[c]);
        else
          printf_sgx("."); /* put this for non-printables */
      else
        printf_sgx(" "); /* pad if short line */

    printf_sgx("\n");
  }
}

static void bin_to_strhex(const unsigned char *bin, size_t binsz, char *result) {
  char hex_str[] = "0123456789abcdef";
  unsigned int i;

  result[binsz * 2] = 0;

  if (!binsz)
    return;

  for (i = 0; i < binsz; i++) {
    result[i * 2 + 0] = hex_str[(bin[i] >> 4) & 0x0F];
    result[i * 2 + 1] = hex_str[(bin[i]) & 0x0F];
  }
}

void print_str_dbg(const char *title, const unsigned char *data, size_t len) {
#ifdef DEBUG
  size_t PRINT_WIDTH = 64;

  char buf[len * 2];
  bin_to_strhex(data, len, buf);
  printf_sgx("%s\n", title);

  size_t printed = 0;
  while (printed < strlen(buf)) {
    printf_sgx("%.64s\n", buf + printed);
    printed += MIN(PRINT_WIDTH, strlen(buf + printed));
  }
#endif
}