#include <stdint.h>
#include <mbedtls/bignum.h>

#include "macros.h"

#ifndef ENCLAVE_ECDSA_H
#define ENCLAVE_ECDSA_H

#if defined(__cplusplus)
extern "C" {
#endif
int sign(uint8_t *data, size_t in_len, uint8_t *rr, uint8_t *ss, uint8_t *vv);
DEPRECATED(int ecdsa_keygen(unsigned char *pubkey, unsigned char *address));
DEPRECATED(int __ecdsa_sec_to_pub(const mbedtls_mpi *seckey, unsigned char *pubkey, unsigned char *address));
#if defined(__cplusplus)
}
#endif
#endif