#include <assert.h>
#include "stdlib.h"
#include "stdint.h"
#include "Enclave_t.h"
#include "vector"
#include "keccak.h"
#include "mbedtls/bignum.h"

#include "ECDSA.h"
#include "Log.h"

#include "ABI.h"
#include "Commons.h"

#include "Transaction.h"

// uncomment to dump transactions
// #define VERBOSE

#ifdef VERBOSE
#include "Debug.h"
#endif


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

int set_byte_length (bytes32* d)
{
    mbedtls_mpi tmp;
    mbedtls_mpi_init(&tmp);
    size_t len, i;
    if (mbedtls_mpi_read_binary (&tmp, &(*d)[0], 32) != 0)
    {
        LL_CRITICAL("Error reading mpi from binary!\n");
        return -1;
    }
    len = mbedtls_mpi_bitlen(&tmp);
    len = (len + 7) / 8;

    // d uses len bytes at right
    // need to move to the begining of the d->d
    for (i = 0; i < len; i++)
    {
        (*d)[i] = (*d)[32 - len + i];
        (*d)[32 - len + i] = 0;
    }

    d->set_size(len);
    mbedtls_mpi_free(&tmp);
    return 0;
}

TX::TX(TX::Type p) {
        this->m_type = p;
		this->v = 0;
}

void TX::rlp_list(bytes& out, bool withSig) {
        int i;
        uint8_t len_len, b;
        m_nonce.rlp(out);
        m_gasPrice.rlp(out);
        m_gas.rlp(out);
        if (m_type == MessageCall) {
            m_receiveAddress.rlp(out);
        }
        m_value.rlp(out);
        m_data.rlp(out);
        // v is also different
        if (withSig) {
            rlp_item((const uint8_t*)&v, 1, out);
            r.rlp(out);
            s.rlp(out);
        } 
        int len = out.size();
        // list header
        if (len < 56) {
            out.insert(out.begin(), 0xc0 + static_cast<uint8_t>(len));
        }
        else {
            len_len = bytesRequired(len);
            if (len_len > 4) {
                LL_CRITICAL("Error: string too long\n");
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

#include "Constants.h"

#define ABI_STR "deliver(uint64,bytes32,bytes32)"

int get_raw_signed_tx(int nonce, int nonce_len, 
                      uint64_t request_id, uint8_t request_type,
                      const uint8_t* req_data, int req_len,
                      uint8_t* resp_data, int resp_len,
                      uint8_t* serialized_tx, int* o_len)
{
    if (serialized_tx == NULL || o_len == NULL) {
        LL_CRITICAL("Error: get_raw_tx gets NULL input\n"); 
        return -1;
    }

    bytes out;
    int ret;
    TX tx(TX::MessageCall);

    assert(nonce_len == 32);

    ABI_UInt64 a(request_id);

//    vector<ABI_serializable*> request_data;
//    bytes32* r = (bytes32*)malloc(req_len/32 * sizeof bytes32);
//    for (int i = 0; i < req_len / 32; i++)
//    {
//        memcpy(r[i].b, req_data + i*32, 32);
//        request_data.push_back(new ABI_Bytes32(&r[i]));
//        // FIXME MEMORY LEAKAGE
//    }
//
//    ABI_T_Array c(request_data);

    // calculate the paramHash
    bytes32 param_hash;
    size_t hash_in_len = 1 + req_len;
    uint8_t* in = static_cast<uint8_t*>(malloc(hash_in_len));
    memset(in, 0, hash_in_len);

    in[0] = request_type;
    memcpy(in + 1, req_data, req_len);

    keccak(in, hash_in_len, &param_hash[0], 32);

#ifdef VERBOSE
    hexdump("Hash Input", in, hash_in_len);
    hexdump("Hash Test", param_hash.b, 32);
#endif
    ABI_Bytes32 c(&param_hash);

    bytes32 resp_b32;
    assert (resp_len == 32);
    memcpy(&resp_b32[0], resp_data, resp_len);

    ABI_Bytes32 d(&resp_b32);

    vector<ABI_serializable*> args;
    args.push_back(&a);
    args.push_back(&c);
    args.push_back(&d);
    
    ABI_Generic_Array abi_items(args);
    
    bytes abi_str;


    if (abi_items.encode(abi_str) != 0) {
        LL_CRITICAL("abi_encoded returned non-zero\n");
        return -1;
    }

    uint8_t func_selector[32];
    ret = keccak((unsigned const char*) ABI_STR, strlen(ABI_STR), func_selector, 32);
    if (ret) {LL_CRITICAL("SHA3 returned %d\n", ret); return -1;}

    // insert function selector
    for (int i = 0; i < 4; i++) {abi_str.insert(abi_str.begin(), func_selector[3 - i]);}


    // XXX
    uint8_t hash[32]; 

    bytes nonce_bytes;
    enc_int(nonce_bytes, nonce, 4);
    memcpy(&tx.m_nonce[0], &nonce_bytes[0], nonce_bytes.size());

    set_byte_length(& tx.m_nonce);
	tx.m_gasPrice.fromHex(GASPRICE);
	tx.m_gas.fromHex(GASLIMIT);
	tx.m_receiveAddress.fromHex(TC_ADDRESS);
    
    tx.m_data.clear();
    tx.m_data = abi_str;

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
        LL_CRITICAL("%s\n", ex.what());
        return -1;
    }

    ret = keccak(&out[0], out.size(), hash, 32);
    if (ret != 0)
    {
        LL_CRITICAL("keccak returned %d", ret); return -1;
    }
    ret = sign(hash, 32, &tx.r[0], &tx.s[0], &tx.v);

    if (ret != 0) { LL_CRITICAL("Error: signing returned %d\n", ret); return ret;}
    else {tx.r.set_size(32); tx.s.set_size(32);}

    out.clear();

    tx.rlp_list(out, true);

    if (out.size() > TX_BUF_SIZE) { LL_CRITICAL("Error buffer size (%d) is too small.\n", TX_BUF_SIZE); return -1;}

#ifdef VERBOSE
    hexdump("RLP:", &out[0], out.size());
#endif
    memcpy(serialized_tx, &out[0], out.size());
    *o_len = out.size();
    return 0;
}
