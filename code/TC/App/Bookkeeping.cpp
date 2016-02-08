#include "sqlite3.h"
#include "Log.h"
#include <cstddef>
#include <stdio.h>
#include <sstream>
#include <iostream>
#include "sys_helper.h"

#define CREATE_TABLE \
    "CREATE TABLE IF NOT EXISTS Nonces ( \
            nonce INT UNIQUE ON CONFLICT REPLACE, \
            time DATETIME);"\
    "CREATE TABLE IF NOT EXISTS Scans (\
            block_id INT UNIQUE ON CONFLICT REPLACE,\
            time     DATETIME );"\
    "CREATE TABLE IF NOT EXISTS Transactions ( \
            id INTEGER PRIMARY KEY, \
            request_id INT UNIQUE ON CONFLICT REPLACE, \
            discover_time DATETIME, \
            complete_time DATETIME);" \

int sqlite3_init(sqlite3** db, const char* db_name)
{
    int ret;
    char* error;
    ret = sqlite3_open(db_name, db);
    if (ret)
    {
        LL_CRITICAL("Error openning SQLITE3 database: %s", sqlite3_errmsg(*db));
        sqlite3_close(*db);
        return -1;
    }
    ret = sqlite3_exec(*db, CREATE_TABLE, NULL, NULL, &error);
    if (ret)
    {
        LL_CRITICAL("Error executing SQLite3 statement: %s", error);
        sqlite3_free(error);
        return -1;
    }

    return 0;
}

static int sqlite3_exec_wrapper(sqlite3* db, const char* sql)
{
    int ret;
    char* error;
    if (!db)
    {
        return -1;
    }
    ret = sqlite3_exec(db, sql, NULL, NULL, &error);
    if (ret)
    {
        LL_CRITICAL("Error executing SQLite3 statement: %s", error);
        sqlite3_free(error);
        return -1;
    }

    return 0;   
}

int record_nonce(sqlite3* db, int nonce)
{
    std::stringstream ss;
    ss << "INSERT INTO Nonces (nonce, time) VALUES (" << nonce << ", \"" << current_datetime() << "\");";
    return sqlite3_exec_wrapper(db, ss.str().c_str());
}

int record_scan(sqlite3* db, int blk)
{
    std::stringstream ss;
    ss << "INSERT INTO Scans (block_id, time) VALUES (" << blk << ", \"" << current_datetime() << "\");";
    return sqlite3_exec_wrapper(db, ss.str().c_str());
}

int get_last_nonce(sqlite3* db, int *nonce)
{
    int ret;
    char* error;
    int n_row, n_col;
    char** result;
    if (!db)
    {
        return -1;
    }
    std::string sql = "SELECT MAX(nonce) FROM Nonces;";

    ret = sqlite3_get_table(db, sql.c_str(), &result, &n_row, &n_col, &error);
    if (ret)
    {
        LL_CRITICAL("Error executing SQLite3 statement: %s", error);
        sqlite3_free(error);
        return -1;
    }

    if (n_row != 1 || n_col != 1)
    {
        LL_CRITICAL("Error, please check SQL statement %s", sql.c_str());
        sqlite3_free_table(result);
        return -1;
    }

    *nonce = atoi(result[1]);
    return 0;  
}

int get_last_scan (sqlite3* db, int* blk)
{
    int ret;
    char* error;
    int n_row, n_col;
    char** result;
    if (!db)
    {
        return -1;
    }
    std::string sql = "SELECT MAX(block_id) FROM Scans;";

    ret = sqlite3_get_table(db, sql.c_str(), &result, &n_row, &n_col, &error);
    if (ret)
    {
        LL_CRITICAL("Error executing SQLite3 statement: %s", error);
        sqlite3_free(error);
        return -1;
    }

    if (n_row != 1 || n_col != 1)
    {
        LL_CRITICAL("Error, please check SQL statement %s", sql.c_str());
        sqlite3_free_table(result);
        return -1;
    }

    *blk = atoi(result[1]);
    return 0;  
}