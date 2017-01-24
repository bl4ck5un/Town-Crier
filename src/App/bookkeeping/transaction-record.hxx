#ifndef TOWN_CRIER_TXRECORD_HXX
#define TOWN_CRIER_TXRECORD_HXX

#include <string>
#include <odb/core.hxx>

#include "../types.hxx"

using namespace std;

#pragma db object
class TransactionRecord {
 public:
  TransactionRecord(blocknum_t blockNumber, string tx, string request, time_t reqTime) :
      blockNumber(blockNumber), tx(tx), request(request), requestTime(reqTime) {
    this->responseTime = 0;
    this->numOfRetrial = 0;
  }

  blocknum_t getBlockNumber() const {
    return blockNumber;
  }
  const string &getTx() const {
    return tx;
  }
  const string &getRequest() const {
    return request;
  }
  time_t getRequestTime() const {
    return requestTime;
  }
  const string &getResponse() const {
    return response;
  }
  time_t getResponseTime() const {
    return responseTime;
  }
  unsigned int getNumOfRetrial() const {
    return numOfRetrial;
  }

  void setResponse(const string &response) {
    TransactionRecord::response = response;
  }
  void setResponseTime(time_t responseTime) {
    TransactionRecord::responseTime = responseTime;
  }
  void setNumOfRetrial(unsigned int numOfRetrial) {
    TransactionRecord::numOfRetrial = numOfRetrial;
  }

 private:
  TransactionRecord() {}
  friend class odb::access;

#pragma db id auto
  unsigned long __id;
  blocknum_t blockNumber;
  string tx;
  string request;
  time_t requestTime;
  string response;
  time_t responseTime;
  unsigned int numOfRetrial;
};

#pragma db view object(TransactionRecord)
struct TransactionStat {
#pragma db column("count(" + TransactionRecord::__id + ")")
  std::size_t count;
#pragma db column("max(" + TransactionRecord::blockNumber + ")")
  blocknum_t lastBlock;
};

#endif  // TOWN_CRIER_TXRECORD_HXX
