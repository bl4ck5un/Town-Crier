#ifndef TLSLIB_T_H__
#define TLSLIB_T_H__

#include <stdint.h>
#include <wchar.h>
#include <stddef.h>
#include "sgx_edger8r.h" /* for sgx_ocall etc. */

#include "mbedtls/net_v.h"
#include "mbedtls/timing_v.h"

#define SGX_CAST(type, item) ((type)(item))

#ifdef __cplusplus
extern "C" {
#endif



sgx_status_t SGX_CDECL ocall_print_string(int* retval, const char* str);
sgx_status_t SGX_CDECL mbedtls_net_init(mbedtls_net_context* ctx);
sgx_status_t SGX_CDECL mbedtls_net_connect(int* retval, mbedtls_net_context* ctx, const char* host, const char* port, int proto);
sgx_status_t SGX_CDECL mbedtls_net_set_block(int* retval, mbedtls_net_context* ctx);
sgx_status_t SGX_CDECL mbedtls_net_set_nonblock(int* retval, mbedtls_net_context* ctx);
sgx_status_t SGX_CDECL mbedtls_net_usleep(unsigned long int usec);
sgx_status_t SGX_CDECL mbedtls_net_recv(int* retval, mbedtls_net_context* ctx, unsigned char* buf, size_t len);
sgx_status_t SGX_CDECL mbedtls_net_send(int* retval, mbedtls_net_context* ctx, const unsigned char* buf, size_t len);
sgx_status_t SGX_CDECL mbedtls_net_recv_timeout(int* retval, mbedtls_net_context* ctx, unsigned char* buf, size_t len, uint32_t timeout);
sgx_status_t SGX_CDECL mbedtls_net_free(mbedtls_net_context* ctx);
sgx_status_t SGX_CDECL mbedtls_timing_hardclock(unsigned long int* retval);
sgx_status_t SGX_CDECL mbedtls_timing_get_timer(unsigned long int* retval, struct mbedtls_timing_hr_time* val, int reset);
sgx_status_t SGX_CDECL mbedtls_set_alarm(int seconds);
sgx_status_t SGX_CDECL mbedtls_timing_set_delay(mbedtls_timing_delay_context* data, uint32_t int_ms, uint32_t fin_ms);
sgx_status_t SGX_CDECL mbedtls_timing_get_delay(int* retval, mbedtls_timing_delay_context* data);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
