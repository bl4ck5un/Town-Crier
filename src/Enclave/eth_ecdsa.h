#include <stdint.h>
#include <sgx_tseal.h>

#include "mbedtls/bignum.h"
#include "macros.h"

#ifndef ENCLAVE_ECDSA_H
#define ENCLAVE_ECDSA_H

#if defined(__cplusplus)
extern "C" {
#endif
int ecdsa_sign(const uint8_t *data, size_t in_len, uint8_t *rr, uint8_t *ss, uint8_t *vv);
int __ecdsa_seckey_to_pubkey(const mbedtls_mpi *seckey, unsigned char *pubkey, unsigned char *address);
int tc_provision_key(const sgx_sealed_data_t *secret, size_t secret_len);
int tc_get_address(unsigned char *pubkey, unsigned char *address);
#if defined(__cplusplus)
}
#endif
#endif