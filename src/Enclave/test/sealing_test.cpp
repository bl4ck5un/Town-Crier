#include <sgx_tseal.h>
#include "../Enclave_t.h"
#include "commons.h"
#include "Debug.h"
#include "mbedtls/ecdsa.h"

extern "C" int seal_data_test();
int seal_data_test()
{
    mbedtls_ecdsa_context ctx;
    mbedtls_ecdsa_init(&ctx);
    mbedtls_ecp_group_load(&ctx.grp, MBEDTLS_ECP_DP_SECP256K1);
    mbedtls_mpi_read_string(&ctx.d, 16, "cd244b3015703ddf545595da06ada5516628c5feadbf49dc66049c4b370cc5d8");
    mbedtls_ecp_mul( &ctx.grp, &ctx.Q, &ctx.d, &ctx.grp.G, NULL, NULL );

    uint8_t x[1024*10];
    uint32_t len = sgx_calc_sealed_data_size(0, sizeof(x));
    sgx_sealed_data_t *p = (sgx_sealed_data_t *)malloc(len);
    printf_sgx("len:%u\n", len);
    if (sgx_seal_data(0, 0, sizeof(x), x, len, p) != SGX_SUCCESS) return -10;

    long long time1, time2;
    rdtsc(&time1);
    for (int i = 0; i < 1000; i++) {
        if (sgx_seal_data(0, 0, sizeof(x), x, len, p) != SGX_SUCCESS) return -1;
        uint32_t MACtext_length = sgx_get_add_mac_txt_len(p);
        uint32_t decrypted_text_length = sgx_get_encrypt_txt_len(p);
        uint8_t y[decrypted_text_length];
        if (sgx_unseal_data(p, 0, &MACtext_length, y, &decrypted_text_length) != SGX_SUCCESS) return -1;
        if (MACtext_length != 0) return -1;
        if (decrypted_text_length != sizeof(x)) return -1;
        if (memcmp(x, y, sizeof(x))) {
            hexdump("x:", x, sizeof(x));
          hexdump("y:", y, sizeof(x));
            return -5;
        }
    }
    rdtsc(&time2);
    free(p);
    return 0;
}
