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
#include <iostream>
#include <map>
#include "json/json.h"
#include "types.hxx"

using namespace jsonrpc;
using namespace std;

ethRPCClient *rpc_client;

/*!
 * Send raw transactions to geth
 * @param raw encoded transaction
 * @return the tx hash
 * See https://github.com/ethereum/wiki/wiki/JSON-RPC#eth_sendrawtransaction
 */
string send_transaction(const std::string &rawTransaction) {
  std::string param(rawTransaction);

  std::string res = rpc_client->eth_sendRawTransaction(param);
  LL_INFO("Response recorded in the blockchain (txHash = %s)", res.c_str());

  return res;
}

/*!
 * @remark How to get topic id?
 *  > https://asecuritysite.com/encryption/sha3
 *  > https://github.com/ethereum/wiki/wiki/Ethereum-Contract-ABI
 *  $ sha3(RequestInfo(uint64,uint8,address,uint256,address,bytes32,uint256,bytes32[]))
 * Returns a filter from block [from] to block [to], writes the value of the filter into [id]
 * Postcondition: [id] is a valid id that can be used with eth_get_filter_logs
 */
string eth_new_filter(blocknum_t from, blocknum_t to) {
  if (from < 0 || to < 0) {
    throw invalid_argument("from or to is smaller than 0");
  }

  Json::Value filter_opt;
  filter_opt["address"] = TC_ADDRESS;

  filter_opt["topics"][0] = "0x295780EA261767C398D062898E5648587D7B8CA371FFD203BE8B4F9A43454FFA";
  filter_opt["fromBlock"] = static_cast<Json::Value::UInt64 >(from);
  filter_opt["toBlock"] = static_cast<Json::Value::UInt64 >(to);

  return rpc_client->eth_newFilter(filter_opt);
}


/* eth_getfilterlogs [hostname] [port] [filter_id] [result] returns the logged events of [filter_id]
 * Given the [filter_id] writes to [result] an array containing the required data
 */
void eth_getfilterlogs(const string &filter_id, Json::Value &txnContainer) {
    if(filter_id.empty()){
      throw invalid_argument("filter_id is empty");
    }
  txnContainer = rpc_client->eth_getFilterLogs(filter_id);
}

/*!
 * @return the highest block number that geth has seen so far
 */
blocknum_t eth_blockNumber()
{
    unsigned long ret;
    std::string blk = rpc_client->eth_blockNumber();
    std::stringstream ss;
    ss << std::hex << blk;
    ss >> ret;
    return ret;
}

long eth_getTransactionCount() {
  unsigned long ret;
  std::string txn_count = rpc_client->eth_getTransactionCount(SGX_ADDRESS, "pending");
  std::stringstream ss;
  ss << std::hex << txn_count;
  ss >> ret;
  return ret;
}

