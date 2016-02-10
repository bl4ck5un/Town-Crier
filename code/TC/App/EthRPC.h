#include <iostream>
#include <map>
#include "json/json.h"

#ifndef ETH_RPC_H
#define ETH_RPC_H

#if defined(__cplusplus)
extern "C" {
#endif

int send_transaction(char* tx);
int eth_getfilterlogs(std::string hostname, unsigned port, long filter_id, Json::Value& tx);
unsigned long eth_blockNumber(std::string hostname, unsigned port);
int eth_new_filter(std::string hostname, unsigned port, int* id, int from, int to);
#if defined(__cplusplus)
}
#endif

#endif