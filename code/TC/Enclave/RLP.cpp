#include "stdlib.h"
#include "stdint.h"
#include "Enclave.h"
#include "vector"
#include "math.h"

using bytes32   = int8_t[32];
using bytes20   = uint8_t[20];
using byte      = uint8_t;
using bytes     = std::vector<uint8_t>;

inline unsigned bytesRequired(uint8_t _i)
{
    unsigned i = 0;
    for (; _i != 0; ++i, _i >>= 8) {}
    return i;
}

int rlp_item(const uint8_t* input, const int len, bytes& out){
    int i, len_len;
    if (!input) {
        printf("Error: input is NULL\n");
        return 0;
    }
    if (len == 1 && (*input) < 0x80 ) {
        out.push_back(*input);
        return 1;
    } else {
        if (len < 56) {
            out.push_back(0x80 + len);
            for (i = 0; i < len; i++) out.push_back(input[i]);
        }
        else {
            len_len = bytesRequired(len);
            out.push_back(0xb7 + len_len);
            if (len_len > 4) {
                printf("Error: string too long\n");
                return -1;
            }
            for (i=len_len-1; i >=0; i--) out.push_back( (uint8_t)(len >> (8*i)) & 0xFF);
            for (i = 0; i < len; i++) out.push_back(input[i]);
        }
        return len;
    }
}


class TX {
protected:
    enum Type
    {
        NullTransaction,
        ContractCreation,
        MessageCall
    };

    Type m_type;
    bytes32 m_nonce;
    bytes32 m_value;
    bytes20 m_receiveAddress;
    bytes32 m_gasPrice;
    bytes32 m_gas;
    bytes m_data;
    bytes32 r;
    bytes32 s;
    byte   v;
public:
    void rlp_list(bytes out) {
        uint8_t len_len, i;

        rlp_item((const uint8_t*)m_nonce, 32, out);
        rlp_item((const uint8_t*)m_gasPrice, 32, out);
        rlp_item((const uint8_t*)m_gas, 32, out);
        if (m_type == MessageCall)
            rlp_item((const uint8_t*) m_receiveAddress, 20, out);
        rlp_item((const uint8_t*)m_value, 32, out);
        rlp_item((const uint8_t*)&m_data[0], m_data.size(), out);
        
        rlp_item((const uint8_t*)&v, 1, out);
        rlp_item((const uint8_t*)r, 32, out);
        rlp_item((const uint8_t*)s, 32, out);

        int len = out.size();
        // list header
        char buf[64];
        uint8_t buf_size = -1;
        if (len < 56) {
            buf[0] = 0xc0 + len;
            out.insert(out.begin(), buf[0]);
        }
        else {
            len_len = bytesRequired(len);
            if (len_len > 4) {
                printf("Error: string too long\n");
                return;
            }
            bytes buff;
            buff.push_back(0xf7 + len_len);
            for (i=len_len-1; i >=0; i--) { buff.push_back( (uint8_t)(len >> (8*i)) & 0xFF);}
            out.insert(out.begin(), buff.begin(), buff.end());
        }
    }
};