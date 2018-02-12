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

#include <stdint.h>
#include "eth_abi.h"
#include "commons.h"
#include "external/keccak.h"
#include "debug.h"
#include <cassert>

extern "C" int ABI_self_test();
int do_ABI_self_test() {
  int ret;
  bytes32 address(0);
  memset(&address[0], 0xBB, 20);

  bytes32 request1(0);
  bytes32 request2(0);
  memset(&request1[0], 0xAA, 32);
  memset(&request2[0], 0xBB, 32);

  ABI_Bytes32 r1(&request1);
  ABI_Bytes32 r2(&request2);

  vector<ABI_Bytes32 *> request_data;
  request_data.push_back(&r1);
  request_data.push_back(&r2);

  ABI_UInt8 a(0xAA);
  ABI_Address b(&address);
  ABI_UInt32 c(0xCCCCCCCC);
//    ABI_T_Array<ABI_Bytes32> d(request_data);

  vector<ABI_serializable *> args;
  args.push_back(&a);
  args.push_back(&b);
  args.push_back(&c);
//    args.push_back(&d);

  ABI_Generic_Array abi_items(args);

  bytes abi_str;
  ret = abi_items.encode(abi_str);

  if (ret) {
    LL_CRITICAL("encode returned %d", ret);
    return ret;
  }

  uint8_t func_selector[4] = {0xFF, 0xEE, 0xDD, 0xCC};

  for (int i = 0; i < 4; i++) {
    abi_str.insert(abi_str.begin(), func_selector[3 - i]);
  }

  unsigned char ref[] = {
      0xFF, 0xEE, 0xDD, 0xCC,
      // enc(0xAA)
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xAA,
      // enc(BBx20)
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBB, 0xBB, 0xBB, 0xBB,
      0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB,
      // enc(0xCCCCCCCC)
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xCC, 0xCC, 0xCC, 0xCC,
  };

  if (memcmp(ref, &abi_str[0], sizeof ref)) {
    LL_CRITICAL("memcmp failed with %d", ret);
    hexdump("Reference:", ref, sizeof ref);
    hexdump("Ours: ", &abi_str[0], sizeof ref);
    return -1;
  }

  return 0;
}

int ABI_self_test() {
  try {
    return do_ABI_self_test();
  }
  catch (const std::exception &e) {
    LL_CRITICAL("exp: %s", e.what());
    return -1;
  }
  catch (...) {
    LL_CRITICAL("unknown exp");
    return -1;
  }
}
