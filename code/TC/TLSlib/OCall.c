#include <stdarg.h>
#include <stdio.h>      /* vsnprintf */
#include "TLSlib.h"
#include "sgx.h"
#include "sgx_trts.h"

extern int ocall_print_string(int* ret, char *str);

int printf(const char *fmt, ...)
{
    int ret;
    va_list ap;
    char buf[BUFSIZ] = {'\0'};
    va_start(ap, fmt);
    vsnprintf(buf, BUFSIZ, fmt, ap);
    va_end(ap);

    ocall_print_string(&ret, buf);
    return ret;
}

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