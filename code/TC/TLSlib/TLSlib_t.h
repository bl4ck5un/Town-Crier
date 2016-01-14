#ifndef TLSLIB_T_H__
#define TLSLIB_T_H__

#include <stdint.h>
#include <wchar.h>
#include <stddef.h>
#include "sgx_edger8r.h" /* for sgx_ocall etc. */

#include "mbedtls/net.h"

#define SGX_CAST(type, item) ((type)(item))

#ifdef __cplusplus
extern "C" {
#endif


int ocall_print_string(const char* str);
int ocall_get_current_time(mbedtls_x509_time* now);
void mbedtls_net_init(mbedtls_net_context* ctx);
int mbedtls_net_connect(mbedtls_net_context* ctx, const char* host, const char* port, int proto);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
