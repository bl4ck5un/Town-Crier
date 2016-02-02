#include "stdlib.h"
#include "stdint.h"
#include "Enclave.h"
#include "vector"
#include "math.h"

typedef struct _b32 {uint8_t b[32]; unsigned size;} bytes32;
typedef struct _b20 {uint8_t b[20]; unsigned size;} bytes20;
typedef uint8_t byte;
typedef std::vector<uint8_t> bytes;

extern "C" {
    int rlp_item(const uint8_t*, const int, bytes&);
    int test_RLP();
}

inline static unsigned bytesRequired(int _i)
{
    unsigned i = 0;
    for (; _i != 0; ++i, _i >>= 8) {}
    return i;
}

int rlp_item(const uint8_t* input, const int len, bytes& out){
    int i, len_len;
    if (!input) {
        printf("Error: input is NULL\n");
        return -1;
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
        return 0;
    }
}

int rlp_item(const bytes32* b, bytes& out) {
    return rlp_item((const uint8_t*)b->b, b->size, out);
}

int rlp_item(const bytes20* b, bytes& out) {
    return rlp_item((const uint8_t*)b->b, b->size, out);
}

int char2int(char input)
{
  if(input >= '0' && input <= '9')
    return input - '0';
  if(input >= 'A' && input <= 'F')
    return input - 'A' + 10;
  if(input >= 'a' && input <= 'f')
    return input - 'a' + 10;
  throw std::invalid_argument("Invalid input string");
}

// This function assumes src to be a zero terminated sanitized string with
// an even number of [0-9a-f] characters, and target to be sufficiently large
void fromHex(const char* src, uint8_t* target, unsigned* len)
{
    *len = 0;
    if (!strlen(src) % 2) 
        { printf("Error: input is not of even len\n"); *len=-1;}
    if (strncmp(src, "0x", 2) == 0) src += 2;
    while(*src && src[1])
    {
        *(target++) = char2int(*src)*16 + char2int(src[1]);
        src += 2;
        *len = (*len)+1;
    }
    if (*len == 1 && *(target - *len) == 0) *len = 0;
}

void fromHex(const char* src, bytes& out)
{
    if (!strlen(src) % 2) 
        { printf("Error: input is not of even len\n");}
    if (strncmp(src, "0x", 2) == 0) src += 2;
    while(*src && src[1])
    {
        out.push_back(char2int(*src)*16 + char2int(src[1]));
        src += 2;
    }
}

void from32(const char* src, bytes32* d) { fromHex(src, d->b, &d->size); }
void from20(const char* src, bytes20* d) { fromHex(src, d->b, &d->size); }



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
    bytes20 m_receiveAddress;
    bytes32 m_gasPrice;
    bytes32 m_gas;
    bytes   m_data;
    bytes32 r;
    bytes32 s;
    byte    v;

    TX(Type p) {
        this->m_type = p;
    }
    void rlp_list(bytes& out) {
        int i;
        uint8_t len_len, b;

        //rlp_item((const uint8_t*)m_nonce.b, 32, out);
        rlp_item(&m_nonce, out);
        //rlp_item((const uint8_t*)m_gasPrice.b, 32, out);
        rlp_item(&m_gasPrice, out);
        //rlp_item((const uint8_t*)m_gas.b, 32, out);
        rlp_item(&m_gas, out);
        if (m_type == MessageCall) {
            //rlp_item((const uint8_t*) m_receiveAddress.b, 20, out);
            rlp_item(&m_receiveAddress, out);
        }
        //rlp_item((const uint8_t*)m_value.b, 32, out);
        rlp_item(&m_value, out);
        // data is different
        rlp_item((const uint8_t*)&m_data[0], m_data.size(), out);
        // v is also different
        rlp_item((const uint8_t*)&v, 1, out);

        //rlp_item((const uint8_t*)r.b, 32, out);
        rlp_item(&r, out);
        //rlp_item((const uint8_t*)s.b, 32, out);
        rlp_item(&s, out);

        int len = out.size();
        // list header
        if (len < 56) {
            out.insert(out.begin(), 0xc0 + len);
        }
        else {
            len_len = bytesRequired(len);
            if (len_len > 4) {
                printf("Error: string too long\n");
                return;
            }
            bytes buff;
            buff.push_back(0xf7 + len_len);
            for (i=len_len-1; i >=0; i--) {
                b = (len >> (8*i)) & 0xFF;
                buff.push_back( b );
            }
            out.insert(out.begin(), buff.begin(), buff.end());
        }
    }
};

#define NONCE       "0x00"
#define GASPRICE    "0x09184e72a000"
#define GASLIMIT    "0x2710"
#define TO_ADDR     "0x0000000000000000000000000000000000000000"
#define VALUE       "0x00"
#define DATA        "0x7f7465737432000000000000000000000000000000000000000000000000000000600057"
#define SIG_V       "0x1c"
#define SIG_R       "0x5e1d3a76fbf824220eafc8c79ad578ad2b67d01b0c2425eb1f1347e8f50882ab"
#define SIG_S       "0x5bd428537f05f9830e93792f90ea6a3e2d1ee84952dd96edbae9f658f831ab13"
#define REF_JS      "f889808609184e72a00082271094000000000000000000000000000000000000000080a47f74657374320000000000000000000000000000000000000000000000000000006000571ca05e1d3a76fbf824220eafc8c79ad578ad2b67d01b0c2425eb1f1347e8f50882aba05bd428537f05f9830e93792f90ea6a3e2d1ee84952dd96edbae9f658f831ab13"

int test_RLP() {
    bytes out;
    unsigned i = 0;
    TX tx(TX::MessageCall);
    from32(NONCE, &tx.m_nonce);
    from32(GASPRICE, &tx.m_gasPrice);
    from32(GASLIMIT, &tx.m_gas);
    from20(TO_ADDR, &tx.m_receiveAddress);
    from32(VALUE, &tx.m_value);
    fromHex(DATA, tx.m_data);
    tx.v = 28;
    from32(SIG_R, &tx.r);
    from32(SIG_S, &tx.s);


    tx.rlp_list(out);
    printf("My:\n");
    for (i = 0; i < out.size(); i++) {
        printf("%02hhx", out[i]);
    }
    printf("\n");

    printf("Ref:\n%s\n", REF_JS);
    return 0;
}