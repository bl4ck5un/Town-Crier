//
// Created by fanz on 6/11/16.
//

#include <gtest/gtest.h>

#include "../Init.h"
#include "../Enclave_u.h"

#include "../EthRPC.h"
#include "Converter.h"
#include <vector>

TEST (RequestHandling, b_from_hex) {
    char* hex = "0xaabbccdd";
    uint8_t b[4];
    uint8_t b_ref[4] = {0xaa, 0xbb, 0xcc, 0xdd};
    b_from_hex(hex, b);

    for (int i = 0; i < 4; i++) {
        ASSERT_EQ(b_ref[i], b[i]);
    }

    std::vector<uint8_t> b_vector;
    b_from_hex(hex, b_vector);

    for (int i = 0; i < 4; i++) {
        ASSERT_EQ(b_ref[i], b_vector[i]);
    }
}

TEST (RequestHandling, parsing) {
}


TEST (RequestHandling, general) {
    sgx_enclave_id_t eid;
    int ret = initialize_enclave(ENCLAVE_FILENAME, &eid);
    EXPECT_EQ(ret, 0);
}