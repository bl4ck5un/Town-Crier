#include <gtest/gtest.h>

#include "../utils.h"
#include "../Enclave_u.h"


TEST (ABI, All) {
    sgx_enclave_id_t eid;
    int ret = initialize_enclave(ENCLAVE_FILENAME, &eid);
    ASSERT_EQ(SGX_SUCCESS, ret);

    ABI_self_test(eid, &ret);
    ASSERT_EQ(0, ret);
}
