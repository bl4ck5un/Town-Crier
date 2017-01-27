#include "stdint.h"
#include <mbedtls/bignum.h>

#ifndef ENCLAVE_ECDSA_H
#define ENCLAVE_ECDSA_H

#define SECKEY_LEN  32
#define PUBKEY_LEN  64
#define ADDRESS_LEN 20

#if defined(__cplusplus)
extern "C" {
#endif
int sign(uint8_t* data, int in_len, uint8_t* rr, uint8_t *ss, uint8_t* vv);
int ecdsa_keygen(unsigned char *pubkey, unsigned char *address);
int __ecdsa_keygen(const mbedtls_mpi *seckey, unsigned char *pubkey, unsigned char *address);
#if defined(__cplusplus)
}
#endif
#endif