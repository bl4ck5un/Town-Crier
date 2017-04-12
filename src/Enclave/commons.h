/* * Copyright (c) 2016-2017 by Cornell University.  All Rights Reserved.
 *
 * Permission to use the "TownCrier" software ("TownCrier"), officially docketed at
 * the Center for Technology Licensing at Cornell University as D-7364, developed
 * through research conducted at Cornell University, and its associated copyrights
 * solely for educational, research and non-profit purposes without fee is hereby
 * granted, provided that the user agrees as follows:
 *
 * The permission granted herein is solely for the purpose of compiling the
 * TowCrier source code. No other rights to use TownCrier and its associated
 * copyrights for any other purpose are granted herein, whether commercial or
 * non-commercial.
 *
 * Those desiring to incorporate TownCrier software into commercial products or use
 * TownCrier and its associated copyrights for commercial purposes must contact the
 * Center for Technology Licensing at Cornell University at 395 Pine Tree Road,
 * Suite 310, Ithaca, NY 14850; email: ctl-connect@cornell.edu; Tel: 607-254-4698;
 * FAX: 607-254-5454 for a commercial license.
 *
 * IN NO EVENT SHALL CORNELL UNIVERSITY BE LIABLE TO ANY PARTY FOR DIRECT,
 * INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS,
 * ARISING OUT OF THE USE OF TOWNCRIER AND ITS ASSOCIATED COPYRIGHTS, EVEN IF
 * CORNELL UNIVERSITY MAY HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * THE WORK PROVIDED HEREIN IS ON AN "AS IS" BASIS, AND CORNELL UNIVERSITY HAS NO
 * OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR
 * MODIFICATIONS.  CORNELL UNIVERSITY MAKES NO REPRESENTATIONS AND EXTENDS NO
 * WARRANTIES OF ANY KIND, EITHER IMPLIED OR EXPRESS, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR
 * PURPOSE, OR THAT THE USE OF TOWNCRIER AND ITS ASSOCIATED COPYRIGHTS WILL NOT
 * INFRINGE ANY PATENT, TRADEMARK OR OTHER RIGHTS.
 *
 * TownCrier was developed with funding in part by the National Science Foundation
 * (NSF grants CNS-1314857, CNS-1330599, CNS-1453634, CNS-1518765, CNS-1514261), a
 * Packard Fellowship, a Sloan Fellowship, Google Faculty Research Awards, and a
 * VMWare Research Award.
 */

#ifndef TOWN_CRIER_COMMONS_H
#define TOWN_CRIER_COMMONS_H
#include <stdint.h>
#include <vector>
#include "Log.h"
#include "encoding.h"

// home-made Big-endian long int
// NOTE: no leading zeros. Starts with useful bytes.
// size is the actual bytes that is used
// for example, bytes32 (0xAAFF) is
// AA, FF, 00, 00, .. , 00,
// size = 2

#define ROUND_TO_32(x) ((x + 31) / 32 * 32)

static uint8_t hex2int(char input) {
  if (input >= '0' && input <= '9')
    return input - '0';
  if (input >= 'A' && input <= 'F')
    return input - 'A' + 10;
  if (input >= 'a' && input <= 'f')
    return input - 'a' + 10;
  throw std::invalid_argument("Invalid input string");
}

static char hexmap[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                        '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

inline std::string toHex(const unsigned char *data, int len) {
  std::string s(len * 2, ' ');
  for (int i = 0; i < len; ++i) {
    s[2 * i] = hexmap[(data[i] & 0xF0) >> 4];
    s[2 * i + 1] = hexmap[data[i] & 0x0F];
  }
  return s;
}

inline const char *hex(char *_hex, const unsigned char *buf, size_t len) {
  for (int i = 0; i < len; ++i) {
    _hex[2 * i] = hexmap[(buf[i] & 0xF0) >> 4];
    _hex[2 * i + 1] = hexmap[buf[i] & 0x0F];
  }
  _hex[2 * len] = 0;
  return _hex;
}

#include <cassert>

template<typename T>
T swap_endian(T u) {
  assert(CHAR_BIT == 8);

  union {
    T u;
    unsigned char u8[sizeof(T)];
  } source, dest;

  source.u = u;

  for (size_t k = 0; k < sizeof(T); k++)
    dest.u8[k] = source.u8[sizeof(T) - k - 1];

  return dest.u;
}

inline uint32_t swap_uint32(uint32_t num) {
  return ((num >> 24) & 0xff) | // move byte 3 to byte 0
      ((num << 8) & 0xff0000) | // move byte 1 to byte 2
      ((num >> 8) & 0xff00) | // move byte 2 to byte 1
      ((num << 24) & 0xff000000); // byte 0 to byte 3
}

inline uint64_t swap_uint64(uint64_t num) {
  return ((static_cast<uint64_t>(swap_uint32(num & 0xffffffff))) << 32) |
      (static_cast<uint64_t>(swap_uint32((num >> 32) & 0xffffffff)));
}

template<typename UINT, size_t BUF_LEN>
UINT uint_bytes(const unsigned char *buf, bool big_endian = true) {
  UINT ret;
  memcpy(&ret, buf + BUF_LEN - sizeof(UINT), sizeof(UINT));
  if (big_endian)
    ret = swap_endian<UINT>(ret);

  return ret;
}

#endif