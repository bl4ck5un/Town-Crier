#include <gtest/gtest.h>

#include "../utils.h"
#include "../Enclave_u.h"

#include "../EthRPC.h"
#include "../Converter.h"
#include <vector>

TEST(RegEx, regex){
    sgx_enclave_id_t eid;
    int ret = initialize_enclave(ENCLAVE_FILENAME, &eid);
    ASSERT_EQ(SGX_SUCCESS, ret);

   //Note: Test is on outdated flights
   // flight_self_test(eid, &ret);
   // ASSERT_EQ(0, ret);
    regex_self_test(eid, &ret);
    ASSERT_EQ(0, ret);

}
