#include <iostream>
#include <map>
#include "json/json.h"

#include "Converter.h"
#include "vector"

#ifndef ETH_RPC_H
#define ETH_RPC_H

using namespace std;

class Request {
// RequestInfo(uint64 id, uint8 requestType, address requester, uint fee, address callbackAddr, byte32 hash, bytes32[] requestData);
// Byte code of ABI encoding:
// 0x00 - 0x20 bytes : id
// 0x20 - 0x40 bytes : requestType
// 0x40 - 0x60 bytes : requester
// 0x60 - 0x80       : fee
// 0x80 - 0xa0       : cb
// 0xa0 - 0xc0       : hash
// 0xc0 - 0xe0       : offset of requestData
// 0xe0 - 0x100      : reqLen (in bytes32)
// 0x100 - ...       : reqData
public:
    uint64_t id;
    uint64_t type;
    uint8_t requester[20];
    uint32_t fee;
    uint8_t callback[20];
    uint8_t param_hash[32];
    uint32_t data_len;
    uint8_t* data;
public:
    Request(uint8_t* input);
    ~Request();
};

#if defined(__cplusplus)
extern "C" {
#endif

int send_transaction(std::string hostname, unsigned port, char* raw);
int eth_getfilterlogs(std::string hostname, unsigned port, std::string filter_id, Json::Value& tx);
long eth_blockNumber(std::string hostname, unsigned port);
int eth_new_filter(std::string hostname, unsigned port, std::string& id, int from, int to);

#if defined(__cplusplus)
}
#endif

#endif