#include <gtest/gtest.h>

#include "../Init.h"

TEST (SealData, All) {
    sgx_enclave_id_t eid;
    int ret = initialize_enclave(ENCLAVE_FILENAME, &eid);
    ASSERT_EQ(SGX_SUCCESS, ret);
    seal_data_test(eid, &ret);
    ASSERT_EQ(0, ret);
}
