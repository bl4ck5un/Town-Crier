#include <sgx_tseal.h>
#include "stdint.h"
#include "commons.h"
#include "encoding.h"

#ifndef TX_H
#define TX_H


class TX {
public:
    enum Type {
        NullTransaction,
        ContractCreation,
        MessageCall
    };
    Type    m_type;
    bytes32 m_nonce;
    bytes32 m_value;
    bytes32 m_receiveAddress;
    bytes32 m_gasPrice;
    bytes32 m_gas;
    bytes   m_data;
    bytes32 r;
    bytes32 s;
    uint8_t v;
    TX(Type p);
    void rlp_list(bytes& out, bool withSig=true);
};


#if defined(__cplusplus)
extern "C" {
#endif

void rlp_item(const uint8_t*, const int, bytes&);
int get_raw_signed_tx(int nonce, int nonce_len,
                      uint64_t request_id, uint8_t request_type,
                      const uint8_t* req_data, int req_len,
                      bytes resp_data,
                      uint8_t *serialized_tx, size_t *o_len);

#if defined(__cplusplus)
}
#endif
#endif
