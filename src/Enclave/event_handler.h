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

inline uint32_t swap_uint32( uint32_t num )
{
    return ((num>>24)&0xff) | // move byte 3 to byte 0
                    ((num<<8)&0xff0000) | // move byte 1 to byte 2
                    ((num>>8)&0xff00) | // move byte 2 to byte 1
                    ((num<<24)&0xff000000); // byte 0 to byte 3
}

inline uint64_t swap_uint64( uint64_t num)
{
	return ((static_cast<uint64_t>(swap_uint32(num & 0xffffffff))) << 32) |
		    (static_cast<uint64_t>(swap_uint32((num >> 32) & 0xffffffff)));
}

#endif
