#include "stdlib.h"
#include "stdint.h"
#include "Enclave.h"
#include "Enclave_t.h"
#include "vector"
#include "keccak.h"
#include "mbedtls/bignum.h"

#include "ECDSA.h"
#include "Log.h"

#ifdef VERBOSE
#include "Debug.h"
#endif

// home-made Big-endian long int
// NOTE: no leading zeros. Starts with useful bytes.
// size is the actual bytes that is used
// for example, bytes32 (0xFFFF) is
// FF, FF, 00, 00, .. , 00,
// size = 2
typedef struct _b32 {uint8_t b[32]; unsigned size;} bytes32;
typedef struct _b20 {uint8_t b[20]; unsigned size;} bytes20;
typedef uint8_t byte;
typedef std::vector<uint8_t> bytes;

extern "C" {
    void rlp_item(const uint8_t*, const int, bytes&);
}

inline static uint8_t bytesRequired(int _i)
{
    uint8_t i = 0;
    for (; _i != 0; ++i, _i >>= 8) {}
    return i;
}

void rlp_item(const uint8_t* input, const int len, bytes& out){
    int i;
    size_t len_len;
    if (!input) {
        throw std::invalid_argument("NULL input");
    }
    if (len == 1 && (*input) < 0x80 ) {
        out.push_back(*input);
        return;
    }
    // longer than 1
    if (len < 56) {
        out.push_back(0x80 + static_cast<uint8_t>(len));
        for (i = 0; i < len; i++) out.push_back(input[i]);
    }
    else {
        len_len = bytesRequired(len);
        if (len_len > 8) {throw std::invalid_argument("Error: len_len > 8");}
        out.push_back(0xb7 + static_cast<uint8_t>(len_len));

        for (i=len_len-1; i >=0; i--) out.push_back( static_cast<uint8_t>((len >> (8*i)) & 0xFF));
        for (i = 0; i < len; i++) out.push_back(input[i]);
    }
}

void rlp_item(const bytes32* b, bytes& out) {
    rlp_item((const uint8_t*)b->b, b->size, out);
}

void rlp_item(const bytes20* b, bytes& out) {
    rlp_item((const uint8_t*)b->b, b->size, out);
}

static uint8_t char2int(char input)
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
/*
    convert a hex string to a byte array
    [in]  src 
    [out] target 
    [out] len: how many bytes get written to the target
*/
void fromHex(const char* src, uint8_t* target, unsigned* len)
{
    *len = 0;
    if ((strlen(src) % 2) != 0) 
        { LL_CRITICAL("Error: input is not of even len\n"); *len=0;}
    if (strncmp(src, "0x", 2) == 0) src += 2;
    while(*src && src[1])
    {
        try { *(target++) = char2int(*src)*16 + char2int(src[1]); } 
        catch (std::invalid_argument e)
            { printf("Error: can't convert %s to bytes\n", src); }
        src += 2; 
        *len = (*len)+1;
    }
    if (*len == 1 && *(target - *len) == 0) *len = 0;
}

void fromHex(const char* src, bytes& out)
{
    if (strlen(src) % 2 != 0) 
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
        d->b[32 - len + i] = 0;
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
            out.insert(out.begin(), 0xc0 + static_cast<uint8_t>(len));
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
#define REF_RLP     "f88903850ba43b740083015f9094762f3a1b5502276bd14ecc1cab7e5e8b5cb2719780a460fe47b1000000000000000000000000000000000000000000000000000000000000001b1ba0ca83f314028eed155e31df4ad05041365cbd2e8ac0bc1c3090765736e01d2110a0442535abfc10b8f6c66363885a46c78c4a087bf601d1e4d638c9346a746254d7"

static int inc_nonce_by_one(uint8_t* nonce)
{
    int ret;
    mbedtls_mpi p;
    mbedtls_mpi_init(&p);

    if (nonce)
    {
#pragma warning (push)
#pragma warning (disable: 4127)
        MBEDTLS_MPI_CHK (mbedtls_mpi_read_binary(&p, nonce, 32));
        MBEDTLS_MPI_CHK (mbedtls_mpi_add_int(&p, &p, 1));
        MBEDTLS_MPI_CHK (mbedtls_mpi_write_binary(&p, nonce, 32));
#pragma warning (pop)
    }
    else
    { printf("NULL input\n!"); return -1; }
cleanup:
    mbedtls_mpi_free(&p);
    return ret;
}

#define ABI_STR "deliver(uint64,uint8,bytes,bytes32)"
#define ROUND_TO_32(x) ((x + 31) / 32 * 32)

static uint8_t get_n_th_byte (uint64_t in, int n)
{
    if (n > 8) {printf("n is too big\n"); return 0xFF;}
    return (in >> (8*n)) & 0xff;
}

int enc_int(bytes& out, uint64_t in, int len)
{
    if (len > 32) {printf("Error: too big\n"); return -1; }
    // padding with 0
    for (int i = 0; i < 32 - len; i++)  {out.push_back(0); }
    // push big-endian int
    for (int i = len - 1; i >= 0; i--) {out.push_back(get_n_th_byte(in, i));}
    return 0;
}

class ABI_item
{
public:
    enum Type
    {
        ABI_UINT64,
        ABI_UINT8,
        ABI_BYTES,
        ABI_BYTES32,
    };

    ABI_item(uint64_t* u64) : static_p(u64),    dynamic_p(NULL), p_size(8), type(ABI_UINT64) {}
    ABI_item(uint8_t* u8)   : static_p(u8),     dynamic_p(NULL), p_size(1), type(ABI_UINT8) {}
    ABI_item(bytes32* b32)  : static_p(b32->b), dynamic_p(NULL), p_size(32), type(ABI_BYTES32) {}
    ABI_item(bytes* bytes)  : static_p(NULL),   dynamic_p(bytes), p_size(bytes->size()), type(ABI_BYTES) {}

    size_t get_head_len() const
    {
        // dynamic: head is always 32 bytes
        // static: head is the actual byte size
        return this->get_is_dynamic() ? 32 : ROUND_TO_32(this->p_size);
    }
    size_t get_tail_len() const
    {
        // dynamic: tail is the actual byte size + 32 (len)
        // static: tail is ""
        return this->get_is_dynamic() ? 32 + ROUND_TO_32(this->p_size) : 0;
    }

    int encode_head (bytes& out) const
    {
        uint64_t tmp;
        uint8_t* tmp_p8;
        switch (this->type)
        {
        case ABI_BYTES:
            printf("Error: only ment for static type!\n");
            return -1;
        case ABI_BYTES32:
            tmp_p8 = (uint8_t*) this->static_p;
            for (int i = 0; i < 32; i++) {out.push_back(tmp_p8[i]);}
            return 0;
        case ABI_UINT64:
            tmp = *((uint64_t*) this->static_p);
            return enc_int(out, tmp, this->p_size);
        case ABI_UINT8:
            tmp = *((uint8_t*) this->static_p);
            return enc_int(out, tmp, this->p_size);
        default:
            printf("Error: unsupported type %d\n", this->type);
            return -1;
        }
    }

    int encode_tail (bytes& out) const
    {
        if (!this->get_is_dynamic()) { return 0; }
        if (this->type != ABI_BYTES)
        {
            printf("Error: not implemented yet!\n");
            return -1;
        }

        if (enc_int(out, dynamic_p->size(), 4) != 0) {printf("Error! enc(out, int) return non-zero!\n"); return -1;}
        // padding left
        for (int i = 0; i < ROUND_TO_32(this->p_size) - this->p_size; i++) {out.push_back(0);}
        out.insert(out.end(), dynamic_p->begin(), dynamic_p->end());
        return 0;
    }
    bool get_is_dynamic() const {return this->type == ABI_BYTES;}

private:
    void* static_p;
    bytes* dynamic_p;
    size_t p_size;
    Type type;
};



int abi_encode(std::vector<ABI_item>& args, bytes& output)
{
    int i, j;
    size_t head_len_sum = 0;
    size_t tail_len_sum = 0;
    for (i = 0; i < args.size(); i++) { head_len_sum += args[i].get_head_len();}

    // head encoding
    for (i = 0; i < args.size(); i++)
    {
        if (args[i].get_is_dynamic())
        {
            for (j = 0; j < i; j++)
            {
                tail_len_sum += args[j].get_tail_len();
            }
            if (enc_int(output, head_len_sum + tail_len_sum, sizeof size_t) != 0) {return -1;}
        }
        else
        {
            if (args[i].encode_head(output) != 0) {
                printf("Error\n");
                return -1;
            }
        }
    }

    // tail encoding
    for (i = 0; i < args.size(); i++)
    {
        args[i].encode_tail(output);
    }

    return 0;
}

/*
    The only function we need to call is the following:
    function deliver(uint64 requestId, uint8 requestType, bytes requestData, bytes32 respData);    
*/

int get_raw_signed_tx(uint8_t* nonce, int i_len, uint8_t* serialized_tx, int* o_len)
{
    if (serialized_tx == nullptr || o_len == nullptr) {printf("Error: get_raw_tx gets NULL input\n"); return -1;}
    bytes out;
    int ret;
    
    assert(i_len == 32);

    uint64_t request_id = 0xFFFFFFFF;
    uint8_t request_type = 0xFF;
    bytes request_data;
    bytes32 resp_data;

    for (int i = 0; i < 100; i++)
    {
        request_data.push_back(0xFF);
    }

    memset(resp_data.b, 0xEE, 32);
    resp_data.size = 32;

    ABI_item a(&request_id);
    ABI_item b(&request_type);
    ABI_item c(&request_data);
    ABI_item d(&resp_data);

    std::vector<ABI_item> args;
    args.push_back(a);
    args.push_back(b);
    args.push_back(c);
    args.push_back(d);

    bytes abi_str;

    // 4 bytes placeholder
    abi_str.push_back(0xAA);
    abi_str.push_back(0xAA);
    abi_str.push_back(0xAA);
    abi_str.push_back(0xAA);

    if (abi_encode(args, abi_str) != 0) {
        printf("abi_encoded returned non-zero\n");
        return -1;
    }


    uint8_t func_selector[32];
    ret = keccak((unsigned const char*) ABI_STR, strlen(ABI_STR), func_selector, 32);
    if (ret) {LL_CRITICAL("SHA3 returned %d\n", ret); return -1;}

    for (int i = 0; i < 4; i++) {abi_str[i] = func_selector[i];}
    
    TX tx(TX::MessageCall);
    uint8_t hash[32]; 

    if (nonce)
        memcpy(tx.m_nonce.b, nonce, 32);
    else
        memset(tx.m_nonce.b, 0x0, 32);

    set_byte_length(& tx.m_nonce);
    from32(GASPRICE, &tx.m_gasPrice);
    from32(GASLIMIT, &tx.m_gas);
    from20(TO_ADDR, &tx.m_receiveAddress);
    from32(VALUE, &tx.m_value);
    
    tx.m_data.clear();
    tx.m_data.insert(tx.m_data.begin(), abi_str.begin(), abi_str.end());

#ifdef VERBOSE
    hexdump("ABI:", &abi_str[0], abi_str.size());
    hexdump("NONCE:", tx.m_nonce.b, 32);
    hexdump("gasPrice:", tx.m_gasPrice.b, 32);
    hexdump("gas: ", tx.m_gas.b, 32);
    hexdump("to_addr: ", tx.m_receiveAddress.b, 20);
    hexdump("value: ", tx.m_value.b, 32);
    hexdump("data: ", &tx.m_data[0], tx.m_data.size());
#endif
    try {
        tx.rlp_list(out, false);
    }
    catch (std::invalid_argument& ex) {
        LL_CRITICAL("%s\n", ex.what()); return -1;
    }


    ret = keccak(&out[0], out.size(), hash, 32);
    if (ret != 0)
    {
        LL_CRITICAL("keccak returned %d", ret); return -1;
    }
    ret = sign(hash, 32, tx.r.b, tx.s.b, &tx.v);

    if (ret != 0) { LL_CRITICAL("Error: signing returned %d\n", ret); return ret;}
    else {tx.r.size = 32; tx.s.size = 32;}

    out.clear();

    tx.rlp_list(out, true);

    if (out.size() > 2048) { LL_CRITICAL("Error buffer size (%d) is too small.\n", *o_len); return -1;}

#ifdef VERBOSE
    hexdump("RLP:", &out[0], out.size());
    hexdump("RFL:", REF_RLP, strlen(REF_RLP));
#endif

    memcpy(serialized_tx, &out[0], out.size());
    *o_len = out.size();

    return inc_nonce_by_one(nonce);
}