//
// Created by mars on 11/4/16.
//

#include <sgx_tseal.h>
#include "../Enclave_t.h"
#include "../Commons.h"
#include "Debug.h"

extern "C" int seal_data_test();
int seal_data_test()
{
    uint8_t x[1024*10];
    uint32_t len = sgx_calc_sealed_data_size(0, sizeof(x));
    sgx_sealed_data_t *p = (sgx_sealed_data_t *)malloc(len);
    printf_sgx("len:%u\n", len);
    if (sgx_seal_data(0, 0, sizeof(x), x, len, p) != SGX_SUCCESS) return -10;

    long long time1, time2;
    rdtsc(&time1);
    for (int i = 0; i < 1000; i++) {
        if (sgx_seal_data(0, 0, sizeof(x), x, len, p) != SGX_SUCCESS) return -1;
        uint32_t MACtext_length = sgx_get_add_mac_txt_len(p), decrypted_text_length = sgx_get_encrypt_txt_len(p);
        uint8_t y[decrypted_text_length];
        if (sgx_unseal_data(p, 0, &MACtext_length, y, &decrypted_text_length) != SGX_SUCCESS) return -1;
        if (MACtext_length != 0) return -1;
        if (decrypted_text_length != sizeof(x)) return -1;
        if (memcmp(x, y, sizeof(x))) {
            hexdump("x:", x, sizeof(x));
            hexdump("y: ", y, sizeof(x));
            return -5;
        }
    }
    rdtsc(&time2);

    printf_sgx("%lld", time2-time2);

    return 0;
}
