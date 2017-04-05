#include <gtest/gtest.h>

#include "utils.h"
#include "Enclave_u.h"

class ScraperTestSuite : public ::testing::Test {
 protected:
  sgx_enclave_id_t eid;
 public:
  virtual void SetUp() {
    initialize_enclave(ENCLAVE_FILENAME, &eid);
  }

  virtual void TearDown() {
    sgx_destroy_enclave(eid);
  }
};

TEST_F(ScraperTestSuite, yahoo) {
  int ocall_status, ret;
  ocall_status = yahoo_self_test(eid, &ret);
  EXPECT_EQ(0, ocall_status);
  EXPECT_EQ(0, ret);
}

TEST_F(ScraperTestSuite, coinmarkget) {
  int ocall_status, ret;
  ocall_status = coin_self_test(eid, &ret);
  EXPECT_EQ(0, ocall_status);
  EXPECT_EQ(0, ret);
}

TEST_F(ScraperTestSuite, google) {
  int ocall_status, ret;
  ocall_status = google_self_test(eid, &ret);
  EXPECT_EQ(0, ocall_status);
  EXPECT_EQ(0, ret);
}

TEST_F(ScraperTestSuite, bloomberg) {
  int ocall_status, ret;
  ocall_status = bloomberg_self_test(eid, &ret);
  EXPECT_EQ(0, ocall_status);
  EXPECT_EQ(0, ret);
}
TEST_F(ScraperTestSuite, flight) {
  int ocall_status, ret;
  ocall_status = flight_self_test(eid, &ret);
  EXPECT_EQ(0, ocall_status);
  EXPECT_EQ(0, ret);
}