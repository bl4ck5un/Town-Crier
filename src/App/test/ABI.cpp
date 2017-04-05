#include <gtest/gtest.h>

#include "utils.h"
#include "Enclave_u.h"

class AbiTestSuite : public ::testing::Test {
 protected:
  sgx_enclave_id_t eid;
  virtual void SetUp() {
    initialize_enclave(ENCLAVE_FILENAME, &eid);
  }

  virtual void TearDown() {
    sgx_destroy_enclave(eid);
  }
};

TEST_F (AbiTestSuite, All) {
  int ret;
  ABI_self_test(eid, &ret);
  ASSERT_EQ(0, ret);
}
