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

  OdbDriver(string filename, bool isOverwrite = false) {
    if (filename.empty()) {
      throw invalid_argument("invalid filename");
    }
    int flag = isOverwrite ? SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE : SQLITE_OPEN_READWRITE;
    db = OdbDriver::db_ptr(new odb::sqlite::database(filename, flag));

    if (flag & SQLITE_OPEN_CREATE) {
      connection_ptr c(db->connection());
      c->execute("PRAGMA foreign_keys=OFF");
      transaction t(db->begin());
      schema_catalog::create_schema(*db);
      t.commit();
      c->execute("PRAGMA foreign_keys=ON");
    }
  }

  void logTransaction(TransactionRecord tr) {
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

    if (r.empty()) return unfulfilled_tx;

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

  void updateLog(const TransactionRecord &tr) {
    transaction t(db->begin());
    record_ptr rc(db->query_one<TransactionRecord>(query_record::tx_hash == tr.getTxHash()));

    if (rc.get() != 0) {
      rc->setResponse(tr.getResponse());
      rc->setResponseTime(tr.getResponseTime());
      rc->setNumOfRetrial(tr.getNumOfRetrial());
      db->update(*rc);
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
