#ifndef ETH_RPC_H
#define ETH_RPC_H

#include <string>
#include "ethrpcclient.h"
#include "types.hxx"
#include <jsonrpccpp/client/connectors/httpclient.h>

using namespace std;

string send_transaction(const ::std::string &rawTransaction);
void eth_getfilterlogs(const string &filter_id, Json::Value &tx);
blocknum_t eth_blockNumber();
long eth_getTransactionCount();
string eth_new_filter(unsigned long from, unsigned long to);

#endif