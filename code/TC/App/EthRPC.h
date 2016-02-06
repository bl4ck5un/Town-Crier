#include <iostream>
#include <map>
#include "json/json.h"

#ifndef ETH_RPC_H
#define ETH_RPC_H

#if defined(__cplusplus)
extern "C" {
#endif

int send_transaction(char* tx);
int fetch_request(std::string hostname, unsigned port, long block_num, long tx_i, Json::Value& tx);

#if defined(__cplusplus)
}
#endif

#endif