/*!
 * @file: EthRPC.cpp
 * @brief: This file provides wrapper functions around Etherium RPC calls
 *
 * It provides functionality for talking to the blockchain and checking specific state
 * Additionally, the Request object parses the data received from the blockchain
 */

#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <iomanip>
#include <sstream>
#include <jsonrpccpp/client/connectors/httpclient.h>

#include "Enclave_u.h"
#include "Log.h"
#include "EthRPC.h"
#include "Constants.h"

using namespace jsonrpc;

ethRPCClient *c;

/*!
 * Send raw transactions to geth
 * @param raw encoded transaction
 */
void send_transaction(const std::string &rawTransaction) {
  std::string res;
  std::string param(rawTransaction);

  res = c->eth_sendRawTransaction(param);
  LL_CRITICAL("Response recorded in the blockchain.");
  LL_CRITICAL("TX: %s", res.c_str());
}

/*!
 * @remark How to get topic id?
 *  > https://asecuritysite.com/encryption/sha3
 *  > https://github.com/ethereum/wiki/wiki/Ethereum-Contract-ABI
 *  $ sha3(RequestInfo(uint64,uint8,address,uint256,address,bytes32,bytes32,bytes32[]))
 * Returns a filter from block [from] to block [to], writes the value of the filter into [id]
 * Postcondition: [id] is a valid id that can be used with eth_get_filter_logs
 */
string eth_new_filter(int from, int to) {
  if (from < 0 || to < 0) {
    throw invalid_argument("from or to is smaller than 0");
  }

  Json::Value filter_opt;
  filter_opt["address"] = TC_ADDRESS;

  filter_opt["topics"][0] = "0xc8d1123dbf500dfdf606f7ec68a0e5f51c3a0112d0d433a442a9664266a8cc41";
  filter_opt["fromBlock"] = from;
  filter_opt["toBlock"] = to;

  return c->eth_newFilter(filter_opt);
}


/* eth_getfilterlogs [hostname] [port] [filter_id] [result] returns the logged events of [filter_id]
 * Given the [filter_id] writes to [result] an array containing the required data
 */
void eth_getfilterlogs(const string &filter_id, Json::Value &txnContainer) {
    if(filter_id.empty()){
      throw invalid_argument("filter_id is empty");
    }
  txnContainer = c->eth_getFilterLogs(filter_id);
}

/*!
 * @return the highest block number that geth has seen so far
 */
long eth_blockNumber()
{
    unsigned long ret;
    std::string blk = c->eth_blockNumber();
    std::stringstream ss;
    ss << std::hex << blk;
    ss >> ret;
    return ret;
}

inline static unsigned int __hextoi(const string &str) {
  try {
    return static_cast<unsigned int> (std::stoi(str, nullptr, 16));
  }
  catch (std::out_of_range &ex) {
    throw invalid_argument("Exception happen when calling stoi(" + str + ")");
  }
}

inline static unsigned long __hextol(const string &str) {
  return std::stoul(str, nullptr, 16);
}

Request::Request(const std::string &input) {
  LL_LOG("input is %s", input.c_str());
  if (input.size() < Request::REQUEST_MIN_LEN) {
    throw std::invalid_argument("input string is too short");
  }
  // 0x00 - 0x20 bytes : id
  size_t offset = (input.compare(0, 2, "0x") == 0) ? 2 : 0;
  this->id = __hextol(input.substr(offset, ENTRY_LEN));
  offset += ENTRY_LEN;

  // 0x20 - 0x40 bytes : requestType
  this->type = __hextol(input.substr(offset, ENTRY_LEN));
  offset += ENTRY_LEN;

  // 0x40 - 0x60 bytes : requester
  offset += ADDRESS_LEADING_ZERO; //skipping leading zeroes
  hexToBuffer(input.substr(offset, ADDRESS_LEN), this->requester, sizeof this->requester);
  offset += ADDRESS_LEN;

  // 0x60 - 0x80       : fee
  this->fee = __hextol(input.substr(offset, ENTRY_LEN));
  offset += ENTRY_LEN;

  // 0x80 - 0xa0       : cb
  offset += ADDRESS_LEADING_ZERO; //skipping leading zeroes
  hexToBuffer(input.substr(offset, ADDRESS_LEN), this->callback, sizeof this->callback);
  offset += ADDRESS_LEN;

  // 0xa0 - 0xc0       : hash
  hexToBuffer(input.substr(offset, ENTRY_LEN), this->param_hash, sizeof this->param_hash);
  offset += ENTRY_LEN;

  // 0xc0 - 0xe0       : timestamp
  this->timestamp = __hextol(input.substr(offset, ENTRY_LEN));
  offset += ENTRY_LEN;

  // 0xe0 - 0x100       : offset of requestData
  offset += ENTRY_LEN; // skipping offset

  // 0x100 - 0x120      : reqLen (in bytes32)
  this->data_len = __hextoi(input.substr(offset, ENTRY_LEN)) * 32;
  offset += ENTRY_LEN;

  // 0x120 - ...       : reqData
  if (this->data_len > 102400) {
    throw std::invalid_argument("request data is too large");
  }

  this->data = static_cast<uint8_t *>(malloc(this->data_len));
  hexToBuffer(input.substr(offset), this->data, this->data_len);
}

/*Destructor */
Request::~Request() {
    if (this->data != NULL) {
        free(this->data);
    }
}
