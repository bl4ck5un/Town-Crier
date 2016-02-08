#include "stdafx.h"
#include "CppUnitTest.h"
#include "sqlite3.h"
#include "Bookkeeping.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{		
	TEST_CLASS(UnitTest1)
	{
	public:
		
		TEST_METHOD(TestBookKeeping) 
		{
            sqlite3* db;
			sqlite3_init(&db, "TEST.db");
            record_nonce(db, 1);
            record_nonce(db, 2);
            record_nonce(db, 8);
            record_nonce(db, 3);
            record_nonce(db, 1);

            int nonce;
            get_last_nonce(db, &nonce);
            Assert::AreEqual(8, nonce);

            record_scan(db, 233);
            record_scan(db, 25);
            record_scan(db, 23);
            record_scan(db, 1);
            record_scan(db, 99);

            int blk_id;
            get_last_scan(db, &blk_id);
            Assert::AreEqual(233, blk_id);

            sqlite3_close(db);
		}

        TEST_METHOD(TestABI)
		{
		    
		}

	};
}