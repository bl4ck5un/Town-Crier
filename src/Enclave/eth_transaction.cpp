//
// Copyright (c) 2016-2018 by Cornell University.  All Rights Reserved.
//
// Permission to use the "TownCrier" software ("TownCrier"), officially
// docketed at the Center for Technology Licensing at Cornell University
// as D-7364, developed through research conducted at Cornell University,
// and its associated copyrights solely for educational, research and
// non-profit purposes without fee is hereby granted, provided that the
// user agrees as follows:
//
// The permission granted herein is solely for the purpose of compiling
// the TownCrier source code. No other rights to use TownCrier and its
// associated copyrights for any other purpose are granted herein,
// whether commercial or non-commercial.
//
// Those desiring to incorporate TownCrier software into commercial
// products or use TownCrier and its associated copyrights for commercial
// purposes must contact the Center for Technology Licensing at Cornell
// University at 395 Pine Tree Road, Suite 310, Ithaca, NY 14850; email:
// ctl-connect@cornell.edu; Tel: 607-254-4698; FAX: 607-254-5454 for a
// commercial license.
//
// IN NO EVENT SHALL CORNELL UNIVERSITY BE LIABLE TO ANY PARTY FOR
// DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES,
// INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF TOWNCRIER AND ITS
// ASSOCIATED COPYRIGHTS, EVEN IF CORNELL UNIVERSITY MAY HAVE BEEN
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// THE WORK PROVIDED HEREIN IS ON AN "AS IS" BASIS, AND CORNELL
// UNIVERSITY HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
// ENHANCEMENTS, OR MODIFICATIONS.  CORNELL UNIVERSITY MAKES NO
// REPRESENTATIONS AND EXTENDS NO WARRANTIES OF ANY KIND, EITHER IMPLIED
// OR EXPRESS, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, OR THAT THE USE
// OF TOWNCRIER AND ITS ASSOCIATED COPYRIGHTS WILL NOT INFRINGE ANY
// PATENT, TRADEMARK OR OTHER RIGHTS.
//
// TownCrier was developed with funding in part by the National Science
// Foundation (NSF grants CNS-1314857, CNS-1330599, CNS-1453634,
// CNS-1518765, CNS-1514261), a Packard Fellowship, a Sloan Fellowship,
// Google Faculty Research Awards, and a VMWare Research Award.
//

#include "eth_transaction.h"

#include <cassert>
#include <cstdlib>
#include <cstdint>
#include <vector>
#include <cinttypes>

#include "mbedtls/bignum.h"

#include "eth_ecdsa.h"
#include "eth_abi.h"
#include "external/keccak.h"
#include "Enclave_t.h"
#include "Constants.h"
#include "log.h"
#include "commons.h"
#include "debug.h"

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

void Transaction::rlpEncode(bytes &out, bool withSig) {
  int i;
  uint8_t len_len, b;

  m_nonce.dump("nonce");
  m_nonce.to_rlp(out);

  m_gasPrice.dump("gasPrice");
  m_gasPrice.to_rlp(out);

  m_gas.dump("gas");
  m_gas.to_rlp(out);

  if (m_type == MessageCall) {
    m_to.to_rlp(out);
    m_to.dump("receiveAddr");
  }

  m_value.dump("value");
  m_value.to_rlp(out);

  m_data.dump("data");
  m_data.to_rlp(out);
  // v is also different
  if (withSig) {
    rlp_item((const uint8_t *) &v, 1, out);
    r.to_rlp(out);
    s.to_rlp(out);
  }
  size_t len = out.size();
  // list header
  if (len < 56) {
    out.insert(out.begin(), static_cast<uint8_t>(0xc0 + len));
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


#define DELIVER_CALL_SIGNATURE "deliver(uint64,bytes32,uint64,bytes32)"

int form_transaction(int nonce,
                     uint64_t request_id,
                     uint8_t request_type,
                     const uint8_t *request_data,
                     size_t request_data_len,
                     uint64_t resp_error,
                     bytes resp_data,
                     uint8_t * tx_output_bf,
                     size_t * o_len,
                     bool with_sig) {

  LL_INFO("forming transaction for nonce=%d, id=%"PRIu64", " "type=%d, date_len=%zu, err=%"PRIu64,
          nonce, request_id, request_type, request_data_len, resp_error);

  if (tx_output_bf == nullptr || o_len == nullptr) {
    LL_CRITICAL("Error: tx_output_bf or o_len gets NULL input\n");
    return TC_INTERNAL_ERROR;
  }

  bytes out;
  int ret;


  // calculate a _tx_hash of input
  // note that the raw input = request_id || request_data
  size_t __hash_input_len = 1 + request_data_len;
  auto __hash_input = static_cast<uint8_t *>(malloc(__hash_input_len));
  memset(__hash_input, 0, __hash_input_len);

  __hash_input[0] = request_type;
  memcpy(__hash_input + 1, request_data, request_data_len);

  uint8_t __hash_out[32];
  keccak(__hash_input, __hash_input_len, __hash_out, sizeof __hash_out);
  free(__hash_input);

  bytes32 param_hash(__hash_out, sizeof __hash_out);

  bytes32 resp_b32(0);
  if (resp_error) {
    resp_b32.reset();
  } else {
    // TODO: note that only the first 32 bytes of the response are taken
    memcpy(resp_b32.data(), &resp_data[0], 32);
  }

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

  LL_DEBUG("before ABI encoding");

  // encoding the function call per ABI
  bytes encoded_delivery_call;
  if (_abi_array.encode(encoded_delivery_call) != 0) {
    LL_CRITICAL("abi_encoded returned non-zero\n");
    return TC_INTERNAL_ERROR;
  }

  LL_DEBUG("up to ABI encoding");

  // compute function selector as the first 4 bytes
  // of SHA3(DELIVER_CALL_SIGNATURE)
  bytes32 func_selector(0);
  ret = keccak((unsigned const char *) DELIVER_CALL_SIGNATURE, strlen(DELIVER_CALL_SIGNATURE), &func_selector[0], 32);
  if (ret) {
    LL_CRITICAL("SHA3 returned %d\n", ret);
    return TC_INTERNAL_ERROR;
  }

  func_selector.dump("func selector");

  // insert the function selector to the begining of the encoded abi
  encoded_delivery_call.insert(encoded_delivery_call.begin(), func_selector.begin(), func_selector.begin() + 4);

  encoded_delivery_call.dump("encoded data");

  // construct a TX

  // 1) encode the nonce
  Transaction tx(Transaction::MessageCall,
                 nonce,
                 GASPRICE,
                 GASLIMIT,
                 TC_ADDRESS,
                 0,
                 encoded_delivery_call);

  tx.m_data = encoded_delivery_call;

  try {
    LL_DEBUG("before rlpEncode");
    tx.rlpEncode(out, false);
    LL_DEBUG("after rlpEncode");
  }
  catch (const std::invalid_argument &e) {
    LL_CRITICAL("%s", e.what());
    return TC_INTERNAL_ERROR;
  }
  catch (const std::exception &e) {
    LL_CRITICAL("%s", e.what());
    return TC_INTERNAL_ERROR;
  }
  catch (...) {
    LL_CRITICAL("unknown ex");
    return TC_INTERNAL_ERROR;
  }

  out.dump("tx w/o sig");

  // prepare for signing
  uint8_t _tx_hash[32];
  ret = keccak(&out[0], out.size(), _tx_hash, 32);
  if (ret != 0) {
    LL_CRITICAL("keccak returned %d", ret);
    return TC_INTERNAL_ERROR;
  }

  tx.r.resize(32, 0);
  tx.s.resize(32, 0);
  if (with_sig) {
    if ((ret = ecdsa_sign(_tx_hash, 32, &tx.r[0], &tx.s[0], &tx.v)) != 0) {
      LL_CRITICAL("Error: signing returned %d", ret);
      return TC_INTERNAL_ERROR;
    }
  } else {
    // fill in dummy signatures
    memset(&tx.r[0], 0, 32);
    memset(&tx.s[0], 0, 32);
    tx.v = 27;
  }

  // RLP encode the final output with signature
  out.clear();
  tx.rlpEncode(out, true);

  out.dump("final tx");

  if (out.size() > TX_BUF_SIZE) {
    LL_CRITICAL("Error buffer size (%d) is too small.\n", TX_BUF_SIZE);
    return TC_INTERNAL_ERROR;
  }

  memcpy(tx_output_bf, &out[0], out.size());
  *o_len = out.size();

  LL_INFO("finished transaction for nonce=%d, id=%"PRIu64", "
      "type=%d, date_len=%zu, err=%"PRIu64 ", total size=%zuB",
          nonce, request_id, request_type, request_data_len, resp_error, *o_len);
  return TC_SUCCESS;
}
