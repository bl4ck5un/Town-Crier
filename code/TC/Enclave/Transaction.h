#include <sgx_tseal.h>
#include "stdint.h"

#ifndef TX_H
#define TX_H

#if defined(__cplusplus)
extern "C" {
#endif

int get_raw_signed_tx(sgx_sealed_data_t* sealed_nonce, int i_len, uint8_t* serialized_tx, int* o_len);

#if defined(__cplusplus)
}
#endif
#endif