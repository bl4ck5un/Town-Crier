#include "sqlite3.h"

extern sqlite3* db;

int sqlite3_init(sqlite3** db, const char*dbname="TC.db");
int sqlite3_drop(const char*dbname="TC.db");

int record_nonce(sqlite3* db, int nonce);
int record_scan(sqlite3* db, int blk);

int get_last_nonce(sqlite3* db, int *nonce);
int get_last_scan (sqlite3* db, int* blk);