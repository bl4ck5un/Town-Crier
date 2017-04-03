#include <assert.h>
#include "stdlib.h"
#include "stdint.h"
#include "Enclave_t.h"
#include "vector"
#include "external/keccak.h"
#include "mbedtls/bignum.h"

#include "eth_ecdsa.h"
#include "Log.h"

#include "eth_abi.h"
#include "commons.h"
#include "Debug.h"

#include "eth_transaction.h"

// uncomment to dump transactions
// #define VERBOSE

#ifdef VERBOSE
#include "Debug.h"
#endif

void rlp_item(const uint8_t *input, const int len, bytes &out) {
  int i;
  size_t len_len;
  if (!input) {
    throw std::invalid_argument("NULL input");
  }
  if (len == 1 && (*input) < 0x80) {
    out.push_back(*input);
    return;
  }
  // longer than 1
  if (len < 56) {
    out.push_back(0x80 + static_cast<uint8_t>(len));
    for (i = 0; i < len; i++) out.push_back(input[i]);
  } else {
    len_len = bytesRequired(len);
    if (len_len > 8) { throw std::invalid_argument("Error: len_len > 8"); }
    out.push_back(0xb7 + static_cast<uint8_t>(len_len));

    for (i = len_len - 1; i >= 0; i--) out.push_back(static_cast<uint8_t>((len >> (8 * i)) & 0xFF));
    for (i = 0; i < len; i++) out.push_back(input[i]);
  }
}

int set_byte_length(bytes32 *d) {
  mbedtls_mpi tmp;
  mbedtls_mpi_init(&tmp);
  size_t len, i;
  if (mbedtls_mpi_read_binary(&tmp, &(*d)[0], 32) != 0) {
    LL_CRITICAL("Error reading mpi from binary!\n");
    return -1;
  }
  len = mbedtls_mpi_bitlen(&tmp);
  len = (len + 7) / 8;

  // d uses len bytes at right
  // need to move to the begining of the d->d
  for (i = 0; i < len; i++) {
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

void TX::rlp_encode(bytes &out, bool withSig) {
  int i;
  uint8_t len_len, b;
  m_nonce.to_rlp(out);
  m_gasPrice.to_rlp(out);
  m_gas.to_rlp(out);
  if (m_type == MessageCall) {
    m_receiveAddress.to_rlp(out);
  }
  m_value.to_rlp(out);
  m_data.to_rlp(out);
  // v is also different
  if (withSig) {
    rlp_item((const uint8_t *) &v, 1, out);
    r.to_rlp(out);
    s.to_rlp(out);
  }
  int len = out.size();
  // list header
  if (len < 56) {
    out.insert(out.begin(), 0xc0 + static_cast<uint8_t>(len));
  } else {
    len_len = bytesRequired(len);
    if (len_len > 4) {
      LL_CRITICAL("Error: string too long\n");
      return;
    }
    bytes buff;
    buff.push_back(0xf7 + len_len);
    for (i = len_len - 1; i >= 0; i--) {
      b = (len >> (8 * i)) & 0xFF;
      buff.push_back(b);
    }
    out.insert(out.begin(), buff.begin(), buff.end());
  }
}

#include "Constants.h"

#define DELIVER_CALL_SIGNATURE "deliver(uint64,bytes32,uint64,bytes32)"

int form_transaction(int nonce,
                     int nonce_len,
                     uint64_t request_id,
                     uint8_t request_type,
                     const uint8_t *request_data,
                     size_t request_data_len,
                     uint64_t resp_error,
                     bytes resp_data,
                     uint8_t *tx_output_bf,
                     size_t *o_len) {
  if (tx_output_bf == NULL || o_len == NULL) {
    LL_CRITICAL("Error: tx_output_bf or o_len gets NULL input\n");
    return TC_INTERNAL_ERROR;
  }

  bytes out;
  int ret;

  TX tx(TX::MessageCall);

  assert(nonce_len == 32);


  // calculate a _tx_hash of input
  bytes32 param_hash;
  // note that the raw input = request_id || request_data
  size_t __hash_input_len = 1 + request_data_len;
  uint8_t *__hash_input = static_cast<uint8_t *>(malloc(__hash_input_len));
  memset(__hash_input, 0, __hash_input_len);

  __hash_input[0] = request_type;
  memcpy(__hash_input + 1, request_data, request_data_len);

  keccak(__hash_input, __hash_input_len, &param_hash[0], 32);
  free(__hash_input);


  bytes32 resp_b32;
  assert (resp_data.size() == 32);
  memcpy(&resp_b32[0], &resp_data[0], 32);

  // prepare for ABI encoding
  vector<ABI_serializable *> args;
  ABI_UInt64 a(request_id);
  ABI_Bytes32 b(&param_hash);
  ABI_UInt64 c(resp_error);
  ABI_Bytes32 d(&resp_b32);
  args.push_back(&a);
  args.push_back(&b);
  args.push_back(&c);
  args.push_back(&d);
  ABI_Generic_Array _abi_array(args);

  // encoding the function call per ABI
  bytes encoded_abi_call;
  if (_abi_array.encode(encoded_abi_call) != 0) {
    LL_CRITICAL("abi_encoded returned non-zero\n");
    return TC_INTERNAL_ERROR;
  }

  uint8_t func_selector[32];
  ret = keccak((unsigned const char *) DELIVER_CALL_SIGNATURE, strlen(DELIVER_CALL_SIGNATURE), func_selector, 32);
  if (ret) {
    LL_CRITICAL("SHA3 returned %d\n", ret);
    return TC_INTERNAL_ERROR;
  }

  dump_buf("encoded_abi_call", &encoded_abi_call[0], encoded_abi_call.size());

  // insert the function selector to the begining of the encoded abi
  for (int i = 0; i < 4; i++) { encoded_abi_call.insert(encoded_abi_call.begin(), func_selector[3 - i]); }

  // construct a TX

  // 1) encode the nonce
  // TODO: why 4? Look up the nonce size in the yellow book
  bytes nonce_bytes;
  append_as_uint256(nonce_bytes, nonce, 4);
  memcpy(&tx.m_nonce[0], &nonce_bytes[0], nonce_bytes.size());
  set_byte_length(&tx.m_nonce);

  tx.m_gasPrice.from_hex(GASPRICE);
  tx.m_gas.from_hex(GASLIMIT);
  tx.m_receiveAddress.from_hex(TC_ADDRESS);

  tx.m_data.clear();
  tx.m_data = encoded_abi_call;

  try {
    tx.rlp_encode(out, false);
  }
  catch (std::invalid_argument &ex) {
    LL_CRITICAL("%s\n", ex.what());
    return TC_INTERNAL_ERROR;
  }

  // prepare for signing
  uint8_t _tx_hash[32];
  ret = keccak(&out[0], out.size(), _tx_hash, 32);
  if (ret != 0) {
    LL_CRITICAL("keccak returned %d", ret);
    return TC_INTERNAL_ERROR;
  }
  ret = sign(_tx_hash, 32, &tx.r[0], &tx.s[0], &tx.v);

  if (ret != 0) {
    LL_CRITICAL("Error: signing returned %d\n", ret);
    return TC_INTERNAL_ERROR;
  }
  else {
    tx.r.set_size(32);
    tx.s.set_size(32);
  }

//  print_str_dbg("r", &tx.r[0], 32);
//  print_str_dbg("s", &tx.s[0], 32);
//  print_str_dbg("v", &tx.v, 1);

  // RLP encode the final output with signature
  out.clear();
  tx.rlp_encode(out, true);

  if (out.size() > TX_BUF_SIZE) {
    LL_CRITICAL("Error buffer size (%d) is too small.\n", TX_BUF_SIZE);
    return TC_INTERNAL_ERROR;
  }

  memcpy(tx_output_bf, &out[0], out.size());
  *o_len = out.size();
  return TC_SUCCESS;
}
