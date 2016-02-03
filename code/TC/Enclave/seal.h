#include "stdint.h"
#include <sgx_tseal.h>

#ifndef SEAL_H
#define SEAL_H

typedef struct nonce {
    uint8_t b[32];
} nonce_t;

#if defined(__cplusplus)
extern "C" {
#endif

int create_nonce(int init, uint8_t* sealed_nonce, int sealed_len);
int inc_nonce_by_one(uint8_t* sealed_nonce, int sealed_len);

#if defined(__cplusplus)
}
#endif
#endif