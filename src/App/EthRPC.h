#include <iostream>
#include <map>
#include "json/json.h"
#include "ethrpcclient.h"
#include <jsonrpccpp/client/connectors/httpclient.h>

#include "Converter.h"
#include "vector"

#ifndef ETH_RPC_H
#define ETH_RPC_H

using namespace std;

class Request {
// RequestInfo(uint64 id, uint8 requestType, address requester, uint fee, address callbackAddr, bytes32 paramsHash,
// bytes32 timestamp, bytes32[] requestData);

// Byte code of ABI encoding:
// 0x00 - 0x20 bytes : id
// 0x20 - 0x40 bytes : requestType
// 0x40 - 0x60 bytes : requester
// 0x60 - 0x80       : fee
// 0x80 - 0xa0       : cb
// 0xa0 - 0xc0       : hash
// 0xc0 - 0xe0       : timestamp
// 0xe0 - 0x100       : offset of requestData
// 0x100 - 0x120      : reqLen (in bytes32)
// 0x120 - ...       : reqData
 public:
  const static int REQUEST_MIN_LEN = 2 * 120;
  const static int ENTRY_LEN = 2 * 32;
  const static int ADDRESS_LEN = 2 * 20;
  const static int ADDRESS_LEADING_ZERO = 2 * 12;

  unsigned long id;
  unsigned long type;
  uint8_t requester[20];
  unsigned long fee;
  uint8_t callback[20];
  uint8_t param_hash[32];
  unsigned long timestamp;
  unsigned int data_len;
  uint8_t *data;
 public:
  Request(const std::string &input);
  ~Request();
};

void send_transaction(const ::std::string &rawTransaction);
void eth_getfilterlogs(const string &filter_id, Json::Value &tx);
long eth_blockNumber();
string eth_new_filter(int from, int to);

#endif