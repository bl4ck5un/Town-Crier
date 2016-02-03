#include "mbedtls/platform.h"
#include "mbedtls/ecdsa.h"

#if defined(__cplusplus)
extern "C" {
#endif

void dump_buf( const char *title, unsigned char *buf, size_t len );
void dump_pubkey( const char *title, mbedtls_ecdsa_context *key );
void dump_mpi (const char* title, mbedtls_mpi* X);
void dump_group( const char* title, mbedtls_ecp_group* grp);
void hexdump(const char* title, void const * data, unsigned int len);

#if defined(__cplusplus)
}
#endif