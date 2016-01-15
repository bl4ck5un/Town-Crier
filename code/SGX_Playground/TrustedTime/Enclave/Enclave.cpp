#include "Enclave_t.h"

#include "sgx_trts.h"
#include "sgx_tae_service.h"
#include "sgx_error.h"

#include "stdio.h"


void printf(const char *fmt, ...)
{
    char buf[BUFSIZ] = {'\0'};
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, BUFSIZ, fmt, ap);
    va_end(ap);
    ocall_print_string(buf);
}

void create_session (void) {
     sgx_status_t st;
    int i = 0;
    do {
        if (i > 0) {
            printf("retry try..\n");
            i++;
        }
        st = sgx_create_pse_session();
    } while (st == SGX_ERROR_BUSY);

    if (st != SGX_SUCCESS ) {
        printf("create_pse_session returned %d\n", st);
    }
    else {
        printf("pse_session created\n");
    }
}

void showtime (void)
{
    sgx_status_t st;
    int i = 0;
    sgx_time_t time;
    sgx_time_source_nonce_t nonce;
    st = sgx_get_trusted_time (&time, &nonce);

    if (st != SGX_SUCCESS) {
        printf("get_trusted_time returned %d\n", st);
    }

    printf("time: %ld\n", time);
    printf("nonce: ");
    for (i = 0; i < 32; i++)
        printf("%#x ", nonce[i]);
    printf("\n");
}