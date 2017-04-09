//
// Copyright (c) 2016-2017 by Cornell University.  All Rights Reserved.
//
// Permission to use the "TownCrier" software ("TownCrier"), officially docketed at
// the Center for Technology Licensing at Cornell University as D-7364, developed
// through research conducted at Cornell University, and its associated copyrights
// solely for educational, research and non-profit purposes without fee is hereby
// granted, provided that the user agrees as follows:
//
// The permission granted herein is solely for the purpose of compiling the
// TowCrier source code. No other rights to use TownCrier and its associated
// copyrights for any other purpose are granted herein, whether commercial or
// non-commercial.
//
// Those desiring to incorporate TownCrier software into commercial products or use
// TownCrier and its associated copyrights for commercial purposes must contact the
// Center for Technology Licensing at Cornell University at 395 Pine Tree Road,
// Suite 310, Ithaca, NY 14850; email: ctl-connect@cornell.edu; Tel: 607-254-4698;
// FAX: 607-254-5454 for a commercial license.
//
// IN NO EVENT SHALL CORNELL UNIVERSITY BE LIABLE TO ANY PARTY FOR DIRECT,
// INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS,
// ARISING OUT OF THE USE OF TOWNCRIER AND ITS ASSOCIATED COPYRIGHTS, EVEN IF
// CORNELL UNIVERSITY MAY HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// THE WORK PROVIDED HEREIN IS ON AN "AS IS" BASIS, AND CORNELL UNIVERSITY HAS NO
// OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR
// MODIFICATIONS.  CORNELL UNIVERSITY MAKES NO REPRESENTATIONS AND EXTENDS NO
// WARRANTIES OF ANY KIND, EITHER IMPLIED OR EXPRESS, INCLUDING, BUT NOT LIMITED
// TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR
// PURPOSE, OR THAT THE USE OF TOWNCRIER AND ITS ASSOCIATED COPYRIGHTS WILL NOT
// INFRINGE ANY PATENT, TRADEMARK OR OTHER RIGHTS.
//
// TownCrier was developed with funding in part by the National Science Foundation
// (NSF grants CNS-1314857, CNS-1330599, CNS-1453634, CNS-1518765, CNS-1514261), a
// Packard Fellowship, a Sloan Fellowship, Google Faculty Research Awards, and a
// VMWare Research Award.
//

#include <stdint.h>
#include <vector>

#include "eth_transaction.h"
#include "Debug.h"
#include "encoding.h"
#include "Constants.h"

extern "C" int transaction_rlp_test();
int transaction_rlp_test() {
  bytes v, t;
  t.push_back('d');
  t.push_back('o');
  t.push_back('g');
  rlp_item(&t[0], 3, v);
  if (v.size() != 4 || v[0] != 0x83) return -1;
  if (v[1] != 'd' || v[2] != 'o' || v[3] != 'g') return -1;
  t.clear();
  v.clear();
  rlp_item(&t[0], 0, v);
  if (v.size() != 1 || v[0] != 0x80) return v[0];

  int nonce = 0;
  int nonce_len = 32;
  uint64_t request_id = 1;
  uint8_t request_type = 2;
  size_t req_len = 192;
  uint8_t req_data[200] =
      {246, 141, 42, 50, 207, 23, 177, 49, 44, 109, 179, 242, 54, 163, 140, 148, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 76, 159, 146, 246, 236, 30, 42, 32, 161, 65, 61, 10, 193, 184, 103, 163, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 31, 92, 131, 160, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 60, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 80, 111, 114, 116, 97,
       108, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  uint8_t resp_data[32] = {0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
                           0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
                           0xdd, 0xdd};

  size_t o_len;
  uint8_t serialized_tx[500];

  uint64_t error_code = 0xee;

  int ret = form_transaction(nonce,
                             nonce_len,
                             request_id,
                             request_type,
                             req_data,
                             req_len,
                             error_code,
                             std::vector<uint8_t>(resp_data, resp_data + 32),
                             serialized_tx,
                             &o_len, true);

  uint8_t ans[] =
      {248, 0xea, 128, 133, 11, 164, 59, 116, 0, 131, 1, 95, 144, 148, 136, 203, 90, 183, 19, 87, 217, 140, 123, 249,
       202, 18, 3, 22, 197, 122, 67, 56, 15, 40, 128, 184,
       0x84, 0x48, 0x7a, 0x6e, 0x32,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 18, 120, 47, 194, 110, 22, 175, 204, 97, 158, 123,
       124, 230, 84, 174, 112, 89, 153, 10, 80, 130, 49, 109, 143, 251, 41, 135, 225, 230, 105, 64, 202,
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00, 0xee,
       0xdd, 0xdd,
       0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
       0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
       0xdd, 0xdd};

  if (ret) return ret;
  if (o_len != sizeof ans + 32 * 2 + 1 + 2) {
    LL_CRITICAL("encoded with wrong length");
    return -1;
  }

  bytes tcAddress;
  tcAddress.from_hex(TC_ADDRESS);

  for (int i = 14; i < 14 + 20; i++) {
    ans[i] = tcAddress.at(i - 14);
  }

  if (memcmp(serialized_tx, ans, sizeof ans)) {
    LL_CRITICAL("memcmp failed");
    print_str_dbg("correct:", ans, sizeof ans);
    print_str_dbg("Ours: ", serialized_tx, sizeof ans);
    return 1;
  }

  return 0;
}
