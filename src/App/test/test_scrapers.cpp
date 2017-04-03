#include <gtest/gtest.h>

#include "../utils.h"
#include "../Enclave_u.h"

#include "../EthRPC.h"
#include "../Converter.h"
#include <vector>

class ScraperTest: public ::testing::Test
{
protected:
    sgx_enclave_id_t eid;
public:
    ScraperTest() {
    }

    ~ScraperTest() {
    }

    virtual void SetUp()
    {
        initialize_enclave(ENCLAVE_FILENAME, &eid);
    }

    virtual void TearDown()
    {
        sgx_destroy_enclave(eid);
    }
};

TEST_F(ScraperTest, yahoo) {
    int ocall_status, ret;
    ocall_status = yahoo_self_test(eid, &ret);
    ASSERT_EQ(0, ocall_status);
    ASSERT_EQ(0, ret);
}

TEST_F(ScraperTest, coinmarkget) {
    int ocall_status, ret;
    ocall_status = coin_self_test(eid, &ret);
    ASSERT_EQ(0, ocall_status);
    ASSERT_EQ(0, ret);
}

TEST_F(ScraperTest, google) {
    int ocall_status, ret;
    ocall_status = google_self_test(eid, &ret);
    ASSERT_EQ(0, ocall_status);
    ASSERT_EQ(0, ret);
}

TEST_F(ScraperTest, bloomberg) {
    int ocall_status, ret;
    ocall_status = bloomberg_self_test(eid, &ret);
    ASSERT_EQ(0, ocall_status);
    ASSERT_EQ(0, ret);
}
TEST_F(ScraperTest, flight){
    int ocall_status, ret;
    ocall_status = flight_self_test(eid, &ret);
    ASSERT_EQ(0, ocall_status);
    ASSERT_EQ(0, ret);
}