//
// Copyright (c) 2016-2018 by Cornell University.  All Rights Reserved.
//
// Permission to use the "TownCrier" software ("TownCrier"), officially
// docketed at the Center for Technology Licensing at Cornell University
// as D-7364, developed through research conducted at Cornell University,
// and its associated copyrights solely for educational, research and
// non-profit purposes without fee is hereby granted, provided that the
// user agrees as follows:
//
// The permission granted herein is solely for the purpose of compiling
// the TownCrier source code. No other rights to use TownCrier and its
// associated copyrights for any other purpose are granted herein,
// whether commercial or non-commercial.
//
// Those desiring to incorporate TownCrier software into commercial
// products or use TownCrier and its associated copyrights for commercial
// purposes must contact the Center for Technology Licensing at Cornell
// University at 395 Pine Tree Road, Suite 310, Ithaca, NY 14850; email:
// ctl-connect@cornell.edu; Tel: 607-254-4698; FAX: 607-254-5454 for a
// commercial license.
//
// IN NO EVENT SHALL CORNELL UNIVERSITY BE LIABLE TO ANY PARTY FOR
// DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES,
// INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF TOWNCRIER AND ITS
// ASSOCIATED COPYRIGHTS, EVEN IF CORNELL UNIVERSITY MAY HAVE BEEN
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// THE WORK PROVIDED HEREIN IS ON AN "AS IS" BASIS, AND CORNELL
// UNIVERSITY HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
// ENHANCEMENTS, OR MODIFICATIONS.  CORNELL UNIVERSITY MAKES NO
// REPRESENTATIONS AND EXTENDS NO WARRANTIES OF ANY KIND, EITHER IMPLIED
// OR EXPRESS, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, OR THAT THE USE
// OF TOWNCRIER AND ITS ASSOCIATED COPYRIGHTS WILL NOT INFRINGE ANY
// PATENT, TRADEMARK OR OTHER RIGHTS.
//
// TownCrier was developed with funding in part by the National Science
// Foundation (NSF grants CNS-1314857, CNS-1330599, CNS-1453634,
// CNS-1518765, CNS-1514261), a Packard Fellowship, a Sloan Fellowship,
// Google Faculty Research Awards, and a VMWare Research Award.
//

#ifndef SRC_APP_BOOKKEEPING_DATABASE_H_
#define SRC_APP_BOOKKEEPING_DATABASE_H_

#include <odb/database.hxx>
#include <odb/schema-catalog.hxx>
#include <odb/sqlite/database.hxx>
#include <odb/transaction.hxx>

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "App/types.h"
#include "App/bookkeeping/transaction-record-odb.hxx"
#include "App/bookkeeping/transaction-record.hxx"
#include "App/logging.h"

using odb::core::connection_ptr;
using odb::core::transaction;
using odb::core::schema_catalog;

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
    int flag = isOverwrite ? SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE
                           : SQLITE_OPEN_READWRITE;
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
    odb::result<TransactionRecord> r(db->query<TransactionRecord>());
    vector<TransactionRecord> tc;
    for (odb::result<TransactionRecord>::iterator it = r.begin();
         !it.equal(r.end()); it++) {
      tc.push_back(*it);
    }

    t.commit();
    return tc;
  }

  vector<TransactionRecord> getUnfulfilledRequest() {
    transaction t(db->begin());
    transaction_record r(
        db->query<TransactionRecord>(query_record::response == ""));

    vector<TransactionRecord> unfulfilled_tx;

    if (r.empty()) {
      return unfulfilled_tx;
    }

    for (auto it = r.begin(); it != r.end(); it++) {
      unfulfilled_tx.push_back(*it);
    }

    t.commit();
    return unfulfilled_tx;
  }

  record_ptr getLogByHash(const string &txHash) const {
    transaction t(db->begin());
    record_ptr rc(
        db->query_one<TransactionRecord>(query_record::tx_hash == txHash));
    t.commit();

    return rc;
  }

  bool isLogged(const string &tx_hash) const {
    bool ret = false;
    transaction t(db->begin());
    record_ptr tr(
        db->query_one<TransactionRecord>(query_record::tx_hash == tx_hash));
    if (!tr) {
      ret = false;
    } else {
      ret = true;
    }
    t.commit();

    return ret;
  }

  bool isProcessed(const string &tx_hash, int retryThreshold) const;

  void updateLog(TransactionRecord tr) {
    transaction t(db->begin());
    record_ptr rc(db->query_one<TransactionRecord>(
        query_record::tx_hash == tr.getTxHash()));

    if (rc.get() != nullptr) {
      rc->setResponse(tr.getResponse());
      rc->setResponseTime(tr.getResponseTime());
      rc->setNumOfRetrial(tr.getNumOfRetrial());
      db->update(*rc);
    } else {
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
    TransactionStat ts(
        db->query_value<TransactionStat>(query_stat::response != "" &&
            !query_stat::response.like("no_tx_in_%")));
    size_t bn = ts.count;
    t.commit();

    return bn;
  }
};

#endif  // SRC_APP_BOOKKEEPING_DATABASE_H_
