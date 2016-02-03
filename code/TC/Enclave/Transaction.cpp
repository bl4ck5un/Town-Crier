#include "stdlib.h"
#include "stdint.h"
#include "Enclave.h"
#include "Enclave_t.h"
#include "vector"
#include "math.h"
#include "stdio.h"
#include "keccak.h"
#include "sgx_tseal.h"
#include "mbedtls/bignum.h"

#include "seal.h"

#include "Debug.h"
#include "ECDSA.h"

typedef struct _b32 {uint8_t b[32]; unsigned size;} bytes32;
typedef struct _b20 {uint8_t b[20]; unsigned size;} bytes20;
typedef uint8_t byte;
typedef std::vector<uint8_t> bytes;

extern "C" {
    int rlp_item(const uint8_t*, const int, bytes&);
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

int set_byte_length (bytes32* d)
{
    mbedtls_mpi tmp;
    mbedtls_mpi_init(&tmp);
    size_t len, i;
    if (mbedtls_mpi_read_binary (&tmp, d->b, 32) != 0)
    {
        printf("Error reading mpi from binary!\n");
        return -1;
    }
    len = mbedtls_mpi_bitlen(&tmp);
    len = (len + 7) / 8;

    // d uses len bytes at right
    // need to move to the begining of the d->d
    for (i = 0; i < len; i++)
    {
        d->b[i] = d->b[32 - len + i];
    }
    

    d->size = len;

    mbedtls_mpi_free(&tmp);
    return 0;
}

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
        memset(&this->m_nonce, 0, sizeof bytes32);
        memset(&this->m_value, 0, sizeof bytes32);
        memset(&this->m_receiveAddress, 0, sizeof bytes20);
        memset(&this->m_gasPrice, 0, sizeof bytes32);
        memset(&this->m_gas, 0, sizeof bytes32);
        memset(&this->r, 0, sizeof bytes32);
        memset(&this->s, 0, sizeof bytes32);
        memset(&this->v, 0, sizeof bytes);
    }
    void rlp_list(bytes& out, bool withSig=true) {
        int i;
        uint8_t len_len, b;

        rlp_item(&m_nonce, out);
        rlp_item(&m_gasPrice, out);
        rlp_item(&m_gas, out);
        if (m_type == MessageCall) {
            rlp_item(&m_receiveAddress, out);
        }
        rlp_item(&m_value, out);
        rlp_item((const uint8_t*)&m_data[0], m_data.size(), out);
        // v is also different

        if (withSig) {
            rlp_item((const uint8_t*)&v, 1, out);
            rlp_item(&r, out);
            rlp_item(&s, out);
        }
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

#define NONCE       "0x06"          
#define GASPRICE    "0x0BA43B7400"  //50000000000
#define GASLIMIT    "0x015F90"      // 90000
#define TO_ADDR     "0x762f3a1b5502276bd14ecc1cab7e5e8b5cb27197"
#define VALUE       "0x00"
#define DATA        "0x60fe47b1000000000000000000000000000000000000000000000000000000000000001b"
//#define SIG_V       "0x1c"
//#define SIG_R       "0x5e1d3a76fbf824220eafc8c79ad578ad2b67d01b0c2425eb1f1347e8f50882ab"
//#define SIG_S       "0x5bd428537f05f9830e93792f90ea6a3e2d1ee84952dd96edbae9f658f831ab13"

static int inc_nonce_by_one(uint8_t* nonce)
{
    int ret;
    mbedtls_mpi p;
    mbedtls_mpi_init(&p);

    if (nonce)
    {
        MBEDTLS_MPI_CHK (mbedtls_mpi_read_binary(&p, nonce, 32));
        MBEDTLS_MPI_CHK (mbedtls_mpi_add_int(&p, &p, 1));
        MBEDTLS_MPI_CHK (mbedtls_mpi_write_binary(&p, nonce, 32));
    }
    else
    { printf("NULL input\n!"); return -1; }
cleanup:
    mbedtls_mpi_free(&p);
    return ret;
}


int get_raw_signed_tx(uint8_t* nonce, int i_len, uint8_t* serialized_tx, int* o_len)
{
    if (serialized_tx == nullptr || o_len == nullptr) {printf("Error: get_raw_tx gets NULL input\n"); return -1;}
    sgx_status_t st;
    bytes out;
    int ret;
    size_t nonce_len;
    (i_len);


    TX tx(TX::MessageCall);
    uint8_t hash[32]; 

    if (nonce)
    {
        memcpy(tx.m_nonce.b, nonce, 32);
    }
    else
    {
        memset(tx.m_nonce.b, 0x0, 32);
    }

    set_byte_length(& tx.m_nonce);
    from32(GASPRICE, &tx.m_gasPrice);
    from32(GASLIMIT, &tx.m_gas);
    from20(TO_ADDR, &tx.m_receiveAddress);
    from32(VALUE, &tx.m_value);
    fromHex(DATA, tx.m_data);

    tx.rlp_list(out, false);

#ifdef TX_DEBUG
    dump_buf("rlp w/o sig: ", &out[0], out.size());
#endif

    ret = keccak(&out[0], out.size(), hash, 32);
    ret = sign(hash, 32, tx.r.b, tx.s.b, &tx.v);

    if (ret != 0) {printf("Error: signing returned %d\n", ret); return ret;}
    else {tx.r.size = 32; tx.s.size = 32;}

    out.clear();

    tx.rlp_list(out, true);

    if (out.size() > 2048) {printf("Error buffer size (%d) is too small.\n", *o_len); return -1;}

#ifdef TX_DEBUG
    dump_buf("rlp w/ sig:\n", &out[0], out.size());
    printf("ref:\n%s\n", "f88903850ba43b740083015f9094762f3a1b5502276bd14ecc1cab7e5e8b5cb2719780a460fe47b1000000000000000000000000000000000000000000000000000000000000001b1ba0ca83f314028eed155e31df4ad05041365cbd2e8ac0bc1c3090765736e01d2110a0442535abfc10b8f6c66363885a46c78c4a087bf601d1e4d638c9346a746254d7");
#endif
    memcpy(serialized_tx, &out[0], out.size());
    *o_len = out.size();

    return inc_nonce_by_one(nonce);
}