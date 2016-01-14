#ifndef ENCLAVE_U_H__
#define ENCLAVE_U_H__

#include <stdint.h>
#include <wchar.h>
#include <stddef.h>
#include <string.h>
#include "sgx_edger8r.h" /* for sgx_status_t etc. */

#include "mbedtls/net_v.h"
#include "mbedtls/timing_v.h"

#define SGX_CAST(type, item) ((type)(item))

#ifdef __cplusplus
extern "C" {
#endif

int SGX_UBRIDGE(SGX_NOCONVENTION, ocall_print_string, (const char* str));
void SGX_UBRIDGE(SGX_NOCONVENTION, mbedtls_net_init, (mbedtls_net_context* ctx));
int SGX_UBRIDGE(SGX_NOCONVENTION, mbedtls_net_connect, (mbedtls_net_context* ctx, const char* host, const char* port, int proto));
int SGX_UBRIDGE(SGX_NOCONVENTION, mbedtls_net_set_block, (mbedtls_net_context* ctx));
int SGX_UBRIDGE(SGX_NOCONVENTION, mbedtls_net_set_nonblock, (mbedtls_net_context* ctx));
void SGX_UBRIDGE(SGX_NOCONVENTION, mbedtls_net_usleep, (unsigned long int usec));
int SGX_UBRIDGE(SGX_NOCONVENTION, mbedtls_net_recv, (mbedtls_net_context* ctx, unsigned char* buf, size_t len));
int SGX_UBRIDGE(SGX_NOCONVENTION, mbedtls_net_send, (mbedtls_net_context* ctx, const unsigned char* buf, size_t len));
int SGX_UBRIDGE(SGX_NOCONVENTION, mbedtls_net_recv_timeout, (mbedtls_net_context* ctx, unsigned char* buf, size_t len, uint32_t timeout));
void SGX_UBRIDGE(SGX_NOCONVENTION, mbedtls_net_free, (mbedtls_net_context* ctx));
unsigned long int SGX_UBRIDGE(SGX_NOCONVENTION, mbedtls_timing_hardclock, ());
unsigned long int SGX_UBRIDGE(SGX_NOCONVENTION, mbedtls_timing_get_timer, (struct mbedtls_timing_hr_time* val, int reset));
void SGX_UBRIDGE(SGX_NOCONVENTION, mbedtls_set_alarm, (int seconds));
void SGX_UBRIDGE(SGX_NOCONVENTION, mbedtls_timing_set_delay, (mbedtls_timing_delay_context* data, uint32_t int_ms, uint32_t fin_ms));
int SGX_UBRIDGE(SGX_NOCONVENTION, mbedtls_timing_get_delay, (mbedtls_timing_delay_context* data));

sgx_status_t ecall_connect(sgx_enclave_id_t eid, int* retval);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
