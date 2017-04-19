#ifndef TOWN_CRIER_ODBDRIVER_HXX
#define TOWN_CRIER_ODBDRIVER_HXX

#include <odb/database.hxx>
#include <odb/sqlite/database.hxx>
#include <odb/schema-catalog.hxx>
#include <odb/transaction.hxx>

#include <memory>
#include <iostream>
#include "../types.hxx"
#include "transaction-record.hxx"
#include "transaction-record-odb.hxx"
#include "Log.h"

#include <string>

using namespace std;
using namespace odb::core;

class OdbDriver {
 private:
  typedef unique_ptr<odb::core::database> db_ptr;
  typedef odb::query<TransactionRecord> query_record;
  typedef odb::query<TransactionStat> query_stat;
  db_ptr db;

 public:
  typedef unique_ptr<TransactionRecord> record_ptr;
  typedef odb::result<TransactionRecord> transaction_record;

  explicit OdbDriver(string filename, bool isOverwrite = false) {
    if (filename.empty()) {
      throw invalid_argument("invalid filename");
    }
    int flag = isOverwrite ? SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE : SQLITE_OPEN_READWRITE;
    db = OdbDriver::db_ptr(new odb::sqlite::database(filename, flag));

    if ((flag & SQLITE_OPEN_CREATE) != 0) {
      connection_ptr c(db->connection());
      c->execute("PRAGMA foreign_keys=OFF");
      transaction t(db->begin());
      schema_catalog::create_schema(*db);
      t.commit();
      c->execute("PRAGMA foreign_keys=ON");
    }
  }

  void logTransaction(TransactionRecord tr) {
    if (isLogged(tr.getTxHash())) {
      return;
}
    transaction t(db->begin());
    db->persist(tr);
    t.commit();
  }

  vector<TransactionRecord> getAllLogs() {
    transaction t(db->begin());
    odb::result<TransactionRecord> r(
        db->query<TransactionRecord>()
    );
    vector<TransactionRecord> tc;
    for (odb::result<TransactionRecord>::iterator it = r.begin(); ! it.equal(r.end()); it++) {
      tc.push_back(*it);
    }

    t.commit();
    return tc;
  }

  vector<TransactionRecord> getUnfulfilledRequest() {
    transaction t(db->begin());
    transaction_record r(db->query<TransactionRecord>(query_record::response == ""));

    vector<TransactionRecord> unfulfilled_tx;

    if (r.empty()) { return unfulfilled_tx;
}

    for (auto it = r.begin(); it != r.end(); it++) {
      unfulfilled_tx.push_back(*it);
    }

    t.commit();
    return unfulfilled_tx;
  }

  record_ptr getLogByHash(const string &txHash) const {
    transaction t(db->begin());
    record_ptr rc(db->query_one<TransactionRecord>(query_record::tx_hash == txHash));
    t.commit();

    return rc;
  }

  bool isLogged(const string &tx_hash) const {
    bool ret = false;
    transaction t(db->begin());
    record_ptr tr(db->query_one<TransactionRecord>(query_record::tx_hash == tx_hash));
    if (!tr) {
      LL_DEBUG("tx %s is not in db", tx_hash.c_str());
      ret = false;
    }
    else {
      ret = true;
    }
    t.commit();

    return ret;
  }

  bool isProcessed(const string &tx_hash, int retryThreshold) const {
    bool ret = false;
    transaction t(db->begin());
    record_ptr tr(db->query_one<TransactionRecord>(query_record::tx_hash == tx_hash));
    if (!tr) {
      LL_DEBUG("tx %s is not processed", tx_hash.c_str());
      ret = false;
    }
    else {
      ret = tr->getNumOfRetrial() >= retryThreshold || ! tr->getResponse().empty();
      LL_DEBUG("tx %s has been tried %d (out of %d) times", tx_hash.c_str(), tr->getNumOfRetrial(), retryThreshold);
      LL_DEBUG("tx %s has been responded with %s", tx_hash.c_str(),
               tr->getResponse().empty() ? "not yet" : tr->getResponse().c_str());
    }
    t.commit();

    return ret;
  }

  void updateLog(TransactionRecord tr) {
    transaction t(db->begin());
    record_ptr rc(db->query_one<TransactionRecord>(query_record::tx_hash == tr.getTxHash()));

    if (rc.get() != nullptr) {
      rc->setResponse(tr.getResponse());
      rc->setResponseTime(tr.getResponseTime());
      rc->setNumOfRetrial(tr.getNumOfRetrial());
      db->update(*rc);
    }
    else {
      db->persist(tr);
    }
    t.commit();
  }

  blocknum_t getLastBlock() const {
    transaction t(db->begin());
    TransactionStat ts(db->query_value<TransactionStat>());
    blocknum_t bn = ts.lastBlock;
    t.commit();

    return bn;
  }

  size_t getNumOfResponse() const {
    transaction t(db->begin());
    TransactionStat ts(db->query_value<TransactionStat>(query_stat::response != ""));
    size_t bn = ts.count;
    t.commit();

    return bn;
  }
};

#endif //TOWN_CRIER_ODBDRIVER_HXX
