//
// Created by fanz on 6/11/16.
//

#include "gtest/gtest.h"

#include "../utils.h"

TEST(Keygen, keygen) {
  sgx_enclave_id_t eid;
  int ret = initialize_enclave(ENCLAVE_FILENAME, &eid);

  unsigned char pubkey[64];
  unsigned char address[20];
  sgx_status_t st;
  st = keygen_test(eid, &ret);
  EXPECT_EQ(0, ret);
}
