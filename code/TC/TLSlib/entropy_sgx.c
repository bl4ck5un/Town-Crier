#include "mbedtls/entropy_poll.h"
#include "sgx_trts.h"
#include "mbedtls/platform.h"
#include "tstdio.h"

int mbedtls_hardware_poll(void *data, unsigned char *output, size_t len, size_t *olen )
{
    sgx_status_t st = sgx_read_rand(output, len);
    if (st != SGX_SUCCESS) {
        printf("hardware_poll fails with %d\n", st);
        *olen = -1;
        return -1;
    }
    else {
        *olen = len;
        return 0;
    }
}