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

#include "eth_transaction.h"

#include "macros.h"
#include "Constants.h"

extern "C" int RLP_self_test();

typedef struct {
  uint64_t nonce;
  uint64_t gasprice;
  uint64_t gaslimit;
  const char *to;
  uint64_t value;
  const char *data_hex;

  const char *expected_output;
} test_case_t;

test_case_t cases[]{
#include "rlp_test_cases"
};

int RLP_self_test() {

  NO_THROW_RET(

      int n_error = 0;
      int n_passed = 0;

      for (auto test : cases) {
        bytes b;
        b.from_hex(test.data_hex);
        Transaction t(Transaction::Type::MessageCall,
                      test.nonce,
                      test.gasprice,
                      test.gaslimit,
                      test.to,
                      test.value,
                      b);

        bytes out;
        t.rlpEncode(out, false);

        if (string(test.expected_output) != to_hex(out.data(), out.size())) {
          LL_DEBUG("wanted %s", test.expected_output);
          LL_DEBUG("got %s", to_hex(out.data(), out.size()).c_str());

          n_error += 1;
        }
        else {
          n_passed += 1;
        }
      }

      if (n_error > 0) {
        LL_CRITICAL("%d tests failed", n_error);
        return -1;
      }
      else {
        LL_INFO("%d tests passed", n_passed);
      }

      return 0;
  )
}
