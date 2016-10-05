
#include <gtest/gtest.h>

#include "../Init.h"
#include "../Enclave_u.h"


TEST (RLP, All) {
    sgx_enclave_id_t eid;
    int ret = initialize_enclave(ENCLAVE_FILENAME, &eid);
    ASSERT_EQ(SGX_SUCCESS, ret);

    RLP_self_test(eid, &ret);
    ASSERT_EQ(0, ret);
}
