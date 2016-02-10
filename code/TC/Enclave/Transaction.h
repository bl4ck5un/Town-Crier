#include <sgx_tseal.h>
#include "stdint.h"
#include "Commons.h"

#ifndef TX_H
#define TX_H

void rlp_item(const uint8_t*, const int, bytes&);

#if defined(__cplusplus)
extern "C" {
#endif

int get_raw_signed_tx(uint8_t* nonce, int nonce_len, 
                      uint64_t request_id, uint8_t request_type,
                      const uint8_t* req_data, int req_len,
                      uint8_t* resp_data, int resp_len,
                      uint8_t* serialized_tx, int* o_len);

#if defined(__cplusplus)
}
#endif
#endif