#include <gtest/gtest.h>

#include "../request-parser.hxx"
#include "Converter.h"

TEST (RequestHandling, hex_and_unhex) {
  uint8_t b[4];
  uint8_t b_ref[4] = {0xaa, 0xbb, 0xcc, 0xdd};

  string hex = bufferToHex(b_ref, sizeof b_ref, false);
  ASSERT_EQ(0, hex.compare("AABBCCDD"));

  string hexString = "0x00aabbccdd";
  uint8_t buffer[5];
  uint8_t buffer_ref[5]{00, 0xaa, 0xbb, 0xcc, 0xdd};
  hexToBuffer(hexString, buffer, sizeof buffer);
  ASSERT_EQ(0, memcmp(buffer, buffer_ref, sizeof buffer));

  memset(b, 0x8a, 4);
  hexToBuffer("", b, sizeof b);
  for (int i = 0; i < 4; i++) {
    ASSERT_EQ(0x8a, b[i]);
  }
}


#define RAW_DATA \
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

TEST (RequestHandling, parsing) {
  std::string raw = std::string(RAW_DATA);
  Request r(raw);

  EXPECT_EQ(0x2340abc, r.getId());
  EXPECT_EQ(2, r.getType());

  bool eq = bufferToHex(r.getRequester(), r.getRequesterLen(), false) == "000000000000000000000000000000000FFFFFFF";
  ASSERT_EQ(true, eq);

  EXPECT_EQ(0x2340abc, r.getFee());

  ASSERT_EQ(0,
            bufferToHex(r.getCallback(),
                        r.getCallbackLen(),
                        false).compare("CCCCAAAA11111111BBBBAAAACCCC555512312312"));
  ASSERT_EQ(0, bufferToHex(r.getParamHash(),
                           r.getParamHashLen(),
                           false).compare("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"));

  EXPECT_EQ(0xBA9C0, r.getTimestamp());
  EXPECT_EQ(6 * 32, r.getDataLen());

  EXPECT_EQ(1, r.getData()[0x20 - 1]);
  EXPECT_EQ(2, r.getData()[0x40 - 1]);
  EXPECT_EQ(3, r.getData()[0x60 - 1]);
  EXPECT_EQ(4, r.getData()[0x80 - 1]);
  EXPECT_EQ(1, r.getData()[0xa0 - 1]);
  EXPECT_EQ(6, r.getData()[0xc0 - 1]);
}