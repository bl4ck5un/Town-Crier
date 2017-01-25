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


using namespace jsonrpc;
using namespace std;

ethRPCClient *c;

/*!
 * Send raw transactions to geth
 * @param raw encoded transaction
 * @return the tx hash
 * See https://github.com/ethereum/wiki/wiki/JSON-RPC#eth_sendrawtransaction
 */
string send_transaction(const std::string &rawTransaction) {
  std::string param(rawTransaction);

  std::string res = c->eth_sendRawTransaction(param);
  LL_CRITICAL("Response recorded in the blockchain.");
  LL_CRITICAL("TX: %s", res.c_str());

  return res;
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

long eth_getTransactionCount() {
  unsigned long ret;
  std::string txn_count = c->eth_getTransactionCount(SGX_ADDRESS, "pending");
  std::stringstream ss;
  ss << std::hex << txn_count;
  ss >> ret;
  return ret;
}

