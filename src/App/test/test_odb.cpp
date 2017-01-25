#include <gtest/gtest.h>

#include <memory>
#include <odb/database.hxx>
#include <odb/sqlite/database.hxx>
#include <odb/schema-catalog.hxx>

#include "../bookkeeping/transaction-record.hxx"
#include "../bookkeeping/transaction-record-odb.hxx"
#include "../bookkeeping/database.hxx"

using namespace std;
using namespace odb::core;

typedef odb::query<TransactionRecord> Query;
typedef odb::result<TransactionRecord> Result;

string db_name = "test.db";

TEST(OdbTest, basic) {
  OdbDriver driver(db_name, true);
  OdbDriver::record_ptr p = driver.getLogByHash("nonexistentHash");
  ASSERT_FALSE(p);
}

TEST(OdbTest, newDB) {
  try {
    OdbDriver driver(db_name, true);
    TransactionRecord r1(1, "0xaaa1", "0xbbb", time(0));
    TransactionRecord r2(2, "0xaaa2", "0xbbb", time(0));
    TransactionRecord r3(3, "0xaaa3", "0xbbb", time(0));
    TransactionRecord r4(4, "0xaaa4", "0xbbb", time(0));
    TransactionRecord r5(5, "0xaaa5", "0xbbb", time(0));

    r1.setResponse("0xresponse");
    r2.setResponse("0xresponse");

    driver.logTransaction(r1);
    driver.logTransaction(r2);
    driver.logTransaction(r3);
    driver.logTransaction(r4);
    driver.logTransaction(r5);

    OdbDriver::record_ptr r4_ = driver.getLogByHash("0xaaa4");
    OdbDriver::record_ptr r5_ = driver.getLogByHash("0xaaa5");

    r4_->setResponse("0xresponse");
    r5_->setResponse("0xresponse");

    driver.updateLog(*r4_.get());
    driver.updateLog(*r5_.get());

    ASSERT_EQ(5, driver.getLastBlock());
    ASSERT_EQ(4, driver.getNumOfResponse());

  }
  catch (const odb::exception &e) {
    std::remove(db_name.c_str());
    cerr << e.what() << endl;
    FAIL();
  }

  OdbDriver driver(db_name, false);

  vector<TransactionRecord> rc = driver.getAllLogs();
  ASSERT_EQ(5, rc.size());
  for (vector<TransactionRecord>::iterator it = rc.begin(); it != rc.end(); it++) {
    cerr << it->getTxHash() << endl;
  }
}
