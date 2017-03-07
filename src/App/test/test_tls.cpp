#include <gtest/gtest.h>
#include <vector>

#include "../utils.h"
#include "../Enclave_u.h"

#include "../EthRPC.h"
#include "../Converter.h"

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
    int ret = 0;
    ssl_self_test(eid, &ret);
    ASSERT_EQ(0, ret);
}
