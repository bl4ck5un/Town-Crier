#include <gtest/gtest.h>

#include "../Init.h"
#include "../Enclave_u.h"

#include "../EthRPC.h"
#include "Converter.h"
#include <vector>

class TLSSuiteTest: public ::testing::Test
{
protected:
    sgx_enclave_id_t eid;
    TLSSuiteTest()
    {
        ;
    }
    ~TLSSuiteTest()
    {
        ;
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

TEST_F (TLSSuiteTest, SSLSuite) {
    int ret;
    ssl_self_test(eid, &ret);
    ASSERT_EQ(0, ret);
}

TEST_F (TLSSuiteTest, Https) {
    //Added by Oscar:
    int ret;
    sgx_status_t ocall_ret;
    ocall_ret = get_page_on_ssl_self_test(eid, &ret);
    ASSERT_EQ(0, ocall_ret);
    ASSERT_EQ(0, ret);
}
