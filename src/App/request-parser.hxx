#ifndef TOWN_CRIER_REQUESTPARSER_HXX
#define TOWN_CRIER_REQUESTPARSER_HXX

#include <cstdint>
#include <string>

using namespace std;

namespace tc {
class RequestParser {
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

 private:
  const string raw_request;
  unsigned long id;
  unsigned long type;

 private:
  uint8_t requester[20];
  unsigned long fee;
  uint8_t callback[20];
  uint8_t param_hash[32];
  unsigned long timestamp;
  size_t data_len;
  uint8_t *data;

 public:
  explicit RequestParser(const std::string &input);
  ~RequestParser();
  unsigned long getId() const;
  unsigned long getType() const;
  const uint8_t *getRequester() const;
  unsigned long getFee() const;
  const uint8_t *getCallback() const;
  const uint8_t *getParamHash() const;
  unsigned long getTimestamp() const;
  size_t getDataLen() const;
  uint8_t *getData() const;
  const string &getRawRequest() const;
  const string toString() const;

  size_t getRequesterLen() {
    return sizeof requester;
  }

  size_t getCallbackLen() {
    return sizeof callback;
  }

  size_t getParamHashLen() {
    return sizeof param_hash;
  }
};
}  // namespace tc

#endif //TOWN_CRIER_REQUESTPARSER_HXX
