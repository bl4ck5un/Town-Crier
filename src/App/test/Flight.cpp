#include <gtest/gtest.h>

#include "../Init.h"
#include "../Enclave_u.h"

#include "../EthRPC.h"
#include "Converter.h"
#include <vector>

TEST(Scraper, flight){
    sgx_enclave_id_t eid;
    int ret = initialize_enclave(ENCLAVE_FILENAME, &eid);
    ASSERT_EQ(SGX_SUCCESS, ret);

    flight_self_test(eid, &ret);
    ASSERT_EQ(0, ret);
}
