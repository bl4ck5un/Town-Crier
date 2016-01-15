#include "Enclave_t.h"
#include "Enclave.h"


//int ocall_net_send (void *ctx, const unsigned char *buf, size_t len) {
//	int ret;
//	sgx_status_t st = mbedtls_net_send(&ret, (mbedtls_net_context*) ctx, buf, len);
//
//	if (st != SGX_SUCCESS)
//		abort();
//
//	return ret;
//}
//
//int ocall_net_recv (void *ctx, unsigned char *buf, size_t len) {
//	int ret;
//	sgx_status_t st = mbedtls_net_recv(&ret, (mbedtls_net_context*) ctx, buf, len);
//
//	if (st != SGX_SUCCESS)
//		abort();
//
//	return ret;
//}