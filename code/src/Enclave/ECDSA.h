#include "stdint.h"

#ifndef ENCLAVE_ECDSA_H
#define ENCLAVE_ECDSA_H

#if defined(__cplusplus)
extern "C" {
#endif
int sign(uint8_t* data, int in_len, uint8_t* rr, uint8_t *ss, uint8_t* vv);
#if defined(__cplusplus)
}
#endif
#endif