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

#include <gtest/gtest.h>
#include <odb/database.hxx>
#include <odb/schema-catalog.hxx>
#include <odb/sqlite/database.hxx>

#include <memory>
#include <vector>

#include "App/bookkeeping/database.h"
#include "App/bookkeeping/transaction-record-odb.hxx"
#include "App/bookkeeping/transaction-record.hxx"

typedef odb::query<TransactionRecord> Query;
typedef odb::result<TransactionRecord> Result;

#define db_name "test.db"

TEST(Odb, basic) {
  OdbDriver driver(db_name, true);
  OdbDriver::record_ptr p = driver.getLogByHash("nonexistentHash");
  ASSERT_FALSE(p);
}

TEST(Odb, newDB) {
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
  } catch (const odb::exception &e) {
    std::remove(db_name);
    cerr << e.what() << endl;
    FAIL();
  }

  OdbDriver driver(db_name, false);

  vector<TransactionRecord> rc = driver.getAllLogs();
  ASSERT_EQ(5, rc.size());
  auto it = rc.begin();
  ASSERT_STREQ("0xaaa1", it->getTxHash().c_str());
  it++;
  ASSERT_STREQ("0xaaa2", it->getTxHash().c_str());
  it++;
  ASSERT_STREQ("0xaaa3", it->getTxHash().c_str());
  it++;
  ASSERT_STREQ("0xaaa4", it->getTxHash().c_str());
  it++;
  ASSERT_STREQ("0xaaa5", it->getTxHash().c_str());
}
