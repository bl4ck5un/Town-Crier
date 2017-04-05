#include <gtest/gtest.h>

#include "utils.h"
#include "Enclave_u.h"

class TLSSuiteTest : public ::testing::Test {
 protected:
  sgx_enclave_id_t eid;
  virtual void SetUp() {
    initialize_enclave(ENCLAVE_FILENAME, &eid);
  }

  virtual void TearDown() {
    sgx_destroy_enclave(eid);
  }
};

TEST_F (TLSSuiteTest, SSLSuite) {
  int ret = 0;
  ssl_self_test(eid, &ret);
  EXPECT_EQ(0, ret);
}
