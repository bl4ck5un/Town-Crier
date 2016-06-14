//
// Created by fanz on 6/11/16.
//

#include <gtest/gtest.h>

#include "../Init.h"
#include "../Enclave_u.h"

#include "../EthRPC.h"
#include "Converter.h"
#include <vector>

TEST (RequestHandling, hex_to_bytes) {
    int ret;
    uint8_t b[4];
    uint8_t b_ref[4] = {0xaa, 0xbb, 0xcc, 0xdd};
    hex_to_bytes("0xaabbccdd", b);

    for (int i = 0; i < 4; i++) {
        ASSERT_EQ(b_ref[i], b[i]);
    }

    memset(b, 0x8a, 4);
    ret = hex_to_bytes("", b);
    ASSERT_EQ(0, ret);
    for (int i = 0; i < 4; i++) {
        ASSERT_EQ(0x8a, b[i]);
    }

    ret = hex_to_bytes(NULL, NULL);
    ASSERT_EQ(-1, ret);

    ret = hex_to_bytes("0xa", NULL);
    ASSERT_EQ(-1, ret);

    long len = calc_b_size("");
    ASSERT_EQ(0, len);

    len = calc_b_size("0x");
    ASSERT_EQ(0, len);
}

// Byte code of ABI encoding:
// 0x00 - 0x20 bytes : id
// 0x20 - 0x40 bytes : requestType
// 0x40 - 0x60 bytes : requester
// 0x60 - 0x80       : fee
// 0x80 - 0xa0       : cb
// 0xa0 - 0xc0       : hash
// 0xc0 - 0xe0       : offset of requestData
// 0xe0 - 0x100      : reqLen (in bytes32)
// 0x100 - ...       : reqData

#define RAW_DATA \
    "0000000000000000000000000000000000000000000000000000000002340abc" \
    "0000000000000000000000000000000000000000000000000000000000000002" \
    "00000000000000000000FF00000000000000000000000000000000000FFFFFFF" \
    "0000000000000000000000000000000000000000000000000000000002340abc" \
    "00000000000000000000EE00000000000000000000000000000000000EEEEEEE" \
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB" \
    "00000000000000000000000000000000000000000000000000000000000000E0" \
    "0000000000000000000000000000000000000000000000000000000000000006" \
    "0000000000000000000000000000000000000000000000000000000000000001" \
    "0000000000000000000000000000000000000000000000000000000000000002" \
    "0000000000000000000000000000000000000000000000000000000000000003" \
    "0000000000000000000000000000000000000000000000000000000000000004" \
    "0000000000000000000000000000000000000000000000000000000000000001" \
    "0000000000000000000000000000000000000000000000000000000000000006"

TEST (RequestHandling, parsing) {
    int ret;
    uint8_t* raw_data = (uint8_t*) malloc(calc_b_size(RAW_DATA));
    hex_to_bytes(RAW_DATA, raw_data);
    Request r(raw_data);

    EXPECT_EQ(0x2340abc, r.id);
    EXPECT_EQ(2, r.type);
    EXPECT_EQ(0x2340abc, r.fee);
    EXPECT_EQ(4 * 32, r.data_len);

    EXPECT_EQ(1, r.data[0x20 - 1]);
    EXPECT_EQ(2, r.data[0x40 - 1]);
    EXPECT_EQ(3, r.data[0x60 - 1]);
    EXPECT_EQ(4, r.data[0x80 - 1]);
    EXPECT_EQ(1, r.data[0xa0 - 1]);
    EXPECT_EQ(6, r.data[0xc0 - 1]);
}


TEST (RequestHandling, general) {
    sgx_enclave_id_t eid;
    int ret = initialize_enclave(ENCLAVE_FILENAME, &eid);
    ASSERT_EQ(SGX_SUCCESS, ret);

    uint8_t* raw_data = (uint8_t*) malloc(calc_b_size(RAW_DATA));
    hex_to_bytes(RAW_DATA, raw_data);
    Request r(raw_data);
    uint8_t raw_tx[2048];
    int raw_tx_len;
    handle_request(eid, &ret, 0, r.id, r.type, r.data, r.data_len, raw_tx, &raw_tx_len);
    ASSERT_EQ(0, ret);
}