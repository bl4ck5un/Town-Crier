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

#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "App/converter.h"
#include "App/request_parser.h"
#include "App/utils.h"

TEST(RequestHandling, hex_and_unhex) {
  uint8_t b[4];
  uint8_t b_ref[4] = {0xaa, 0xbb, 0xcc, 0xdd};

  string hex = bufferToHex(b_ref, sizeof b_ref, false);
  ASSERT_EQ(0, hex.compare("AABBCCDD"));

  string hexString = "0x00aabbccdd";
  uint8_t buffer[5];
  uint8_t buffer_ref[5]{00, 0xaa, 0xbb, 0xcc, 0xdd};
  hexToBuffer(hexString, buffer, sizeof buffer);
  ASSERT_EQ(0, memcmp(buffer, buffer_ref, sizeof buffer));

  vector<uint8_t> bufferVec;
  hexToBuffer(hexString, &bufferVec);
  for (auto i = 0; i < bufferVec.size(); i++) {
    ASSERT_EQ(buffer_ref[i], bufferVec.at(i));
  }

  memset(b, 0x8a, 4);
  hexToBuffer("", b, sizeof b);
  for (int i = 0; i < 4; i++) {
    ASSERT_EQ(0x8a, b[i]);
  }
}

#define RAW_DATA                                                     \
  "0000000000000000000000000000000000000000000000000000000002340abc" \
  "0000000000000000000000000000000000000000000000000000000000000002" \
  "00000000000000000000FF00000000000000000000000000000000000FFFFFFF" \
  "0000000000000000000000000000000000000000000000000000000002340abc" \
  "000000000000000000000000CCCCAAAA11111111BBBBAAAACCCC555512312312" \
  "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB" \
  "00000000000000000000000000000000000000000000000000000000000BA9C0" \
  "00000000000000000000000000000000000000000000000000000000000000E0" \
  "0000000000000000000000000000000000000000000000000000000000000006" \
  "0000000000000000000000000000000000000000000000000000000000000001" \
  "0000000000000000000000000000000000000000000000000000000000000002" \
  "0000000000000000000000000000000000000000000000000000000000000003" \
  "0000000000000000000000000000000000000000000000000000000000000004" \
  "0000000000000000000000000000000000000000000000000000000000000001" \
  "0000000000000000000000000000000000000000000000000000000000000006"

// Byte code of ABI encoding:
// 0x00 - 0x20 bytes : id
// 0x20 - 0x40 bytes : requestType
// 0x40 - 0x60 bytes : requester
// 0x60 - 0x80       : fee
// 0x80 - 0xa0       : cb
// 0xa0 - 0xc0       : hash
// 0xc0 - 0xe0       : timestamp
// 0xe0 - 0x100       : offset of requestData
// 0x100 - 0x120      : reqLen (in bytes32)
// 0x120 - ...       : reqData

TEST(RequestHandling, parsing) {
  std::string raw = std::string(RAW_DATA);
  tc::RequestParser r(raw, "hash");

  EXPECT_EQ(0x2340abc, r.getId());
  EXPECT_EQ(2, r.getType());

  bool eq = bufferToHex(r.getRequester(), r.getRequesterLen(), false) ==
            "000000000000000000000000000000000FFFFFFF";
  ASSERT_EQ(true, eq);

  EXPECT_EQ(0x2340abc, r.getFee());

  ASSERT_EQ(0, bufferToHex(r.getCallback(), r.getCallbackLen(), false)
                   .compare("CCCCAAAA11111111BBBBAAAACCCC555512312312"));
  ASSERT_EQ(0, bufferToHex(r.getParamHash(), r.getParamHashLen(), false)
                   .compare("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABBBBBBBBBBBBBBBBBBB"
                            "BBBBBBBBBBBBBB"));

  EXPECT_EQ(0xBA9C0, r.getTimestamp());
  EXPECT_EQ(6 * 32, r.getDataLen());

  r.dumpData();

  EXPECT_EQ(1, r.getData()[0x20 - 1]);
  EXPECT_EQ(2, r.getData()[0x40 - 1]);
  EXPECT_EQ(3, r.getData()[0x60 - 1]);
  EXPECT_EQ(4, r.getData()[0x80 - 1]);
  EXPECT_EQ(1, r.getData()[0xa0 - 1]);
  EXPECT_EQ(6, r.getData()[0xc0 - 1]);
}
