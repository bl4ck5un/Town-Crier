#ifndef ENCLAVE_EVENTHANDLER_H
#define ENCLAVE_EVENTHANDLER_H

#include <stddef.h>

#if defined(__cplusplus)
extern "C" {
#endif

int handle_request(int nonce,
                   uint64_t id,
                   uint64_t type,
                   uint8_t *data,
                   int data_len,
                   uint8_t *raw_tx,
                   size_t *raw_tx_len);

#if defined(__cplusplus)
}
#endif



#endif
