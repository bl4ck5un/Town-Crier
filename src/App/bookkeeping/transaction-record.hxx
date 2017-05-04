#ifndef TOWN_CRIER_TXRECORD_HXX
#define TOWN_CRIER_TXRECORD_HXX

#include <string>
#include <odb/core.hxx>

#include "App/types.h"

using namespace std;

#pragma db object
class TransactionRecord {
 public:
  TransactionRecord(blocknum_t blocknumber, string tx, string request) :
      block_number(blocknumber), tx_hash(tx), request(request) {
    this->request_time = time(0);
    this->n_retrial = 0;
    this->response_time = 0;
  }
  TransactionRecord(blocknum_t blockNumber, string tx, string request, time_t reqTime) :
      block_number(blockNumber), tx_hash(tx), request(request), request_time(reqTime) {
    this->response_time = 0;
    this->n_retrial = 0;
  }

  blocknum_t getBlockNumber() const {
    return block_number;
  }
  const string &getTxHash() const {
    return tx_hash;
  }
  const string &getRequest() const {
    return request;
  }
  time_t getRequestTime() const {
    return request_time;
  }
  const string &getResponse() const {
    return response;
  }
  time_t getResponseTime() const {
    return response_time;
  }
  unsigned int getNumOfRetrial() const {
    return n_retrial;
  }

  void setResponse(const string &response) {
    TransactionRecord::response = response;
  }
  void setResponseTime(time_t responseTime) {
    TransactionRecord::response_time = responseTime;
  }
  void setNumOfRetrial(unsigned int numOfRetrial) {
    TransactionRecord::n_retrial = numOfRetrial;
  }
  void incrementNumOfRetrial() {
    n_retrial++;
  }

 private:
  TransactionRecord() {}
  friend class odb::access;

#pragma db id auto
  unsigned long __id;
  blocknum_t block_number;
#pragma db unique
  string tx_hash;
  string request;
  time_t request_time;
  string response;
  time_t response_time;
  unsigned int n_retrial;
};

#pragma db view object(TransactionRecord)
struct TransactionStat {
#pragma db column("count(" + TransactionRecord::__id + ")")
  std::size_t count;
#pragma db column("max(" + TransactionRecord::block_number + ")")
  blocknum_t lastBlock;
};

#endif  // TOWN_CRIER_TXRECORD_HXX
