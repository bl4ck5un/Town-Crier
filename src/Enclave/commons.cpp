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

#include "commons.h"

static uint8_t hex2int(char input) {
  if (input >= '0' && input <= '9')
    return static_cast<uint8_t>(input - '0');
  if (input >= 'A' && input <= 'F')
    return static_cast<uint8_t>(input - 'A' + 10);
  if (input >= 'a' && input <= 'f')
    return static_cast<uint8_t>(input - 'a' + 10);
  throw std::invalid_argument("Invalid input string");
}

static char hexmap[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                        '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};


std::vector<uint8_t> tc::enclave::from_hex(const char *src) {
  std::vector<uint8_t> out;
  if (strlen(src) % 2 != 0)
    throw invalid_argument("input is not of even length");
  if (strncmp(src, "0x", 2) == 0) src += 2;
  while (*src && src[1]) {
    out.push_back(hex2int(*src) * 16 + hex2int(src[1]));
    src += 2;
  }

  return out;
}

// This function assumes src to be a zero terminated sanitized string with
// an even number of [0-9a-f] characters, and target to be sufficiently large
void tc::enclave::from_hex(const char* src, char* target)
{
  while(*src && src[1])
  {
    *(target++) = hex2int(*src)*HEX_BASE + hex2int(src[1]);
    src += 2;
  }
}

std::string to_hex(const unsigned char *data, size_t len) {
  std::string s(len * 2, ' ');
  for (int i = 0; i < len; ++i) {
    s[2 * i] = hexmap[(data[i] & 0xF0) >> 4];
    s[2 * i + 1] = hexmap[data[i] & 0x0F];
  }
  return s;
}
