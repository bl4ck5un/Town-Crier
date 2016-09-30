#include <iostream>
#include "Monitor.h"
#include "EthRPC.h"
#include "Log.h"

#ifdef _WIN32
#define _WINSOCKAPI_
#include "windows.h"
#endif


#include "Enclave_u.h"
#include "App.h"
#include "Utils.h"
#include <Constants.h>
#include "RemoteAtt.h"
#include "Constants.h"
#include "Bookkeeping.h"
#include <iomanip>

#include <math.h>
#include <unistd.h>

#define RPC_HOSTNAME    "localhost"
#define RPC_PORT        8545


inline uint64_t swap_uint64(uint64_t X) {
    uint64_t x = X;
    x = (x & 0x00000000FFFFFFFF) << 32 | (x & 0xFFFFFFFF00000000) >> 32;
    x = (x & 0x0000FFFF0000FFFF) << 16 | (x & 0xFFFF0000FFFF0000) >> 16;
    x = (x & 0x00FF00FF00FF00FF) << 8  | (x & 0xFF00FF00FF00FF00) >> 8;
    return x;
}

inline uint32_t swap_uint32( uint32_t num )
{
    return ((num>>24)&0xff) | // move byte 3 to byte 0
                    ((num<<8)&0xff0000) | // move byte 1 to byte 2
                    ((num>>8)&0xff00) | // move byte 2 to byte 1
                    ((num<<24)&0xff000000); // byte 0 to byte 3
}

long g_frequency = 2500000000;

int monitor_loop(sgx_enclave_id_t eid, int nonce)
{
#ifdef E2E_BENCHMARK
    // prepare
    long long time1, time2;
    time1 = __rdtsc();
    LL_CRITICAL("Starting..");
    time2 = __rdtsc();
    LL_CRITICAL("print overhead: %llu", time2-time1);

    // benchmark remote att
    time1 = __rdtsc();
    remote_att_init();
    time2 = __rdtsc();
    LL_CRITICAL("remote_att overall %llu", (time2-time1));

    int ret = 0;
    char req[64] = {0};
    req[0] = 'G';
    req[1] = 'O';
    req[2] = 'O';
    req[3] = 'G';
    req[4] = 'L';
    uint8_t raw_tx[TX_BUF_SIZE];
    int raw_tx_len = sizeof ( raw_tx );

    // benchmark request handling
    time1 = __rdtsc();
    LL_CRITICAL("Entering handle_request: %llu", time1);
    handle_request(global_eid, &ret, nonce, 0xFF, 0x01, (uint8_t*) req, sizeof ( req ), raw_tx, &raw_tx_len);
    time2 = __rdtsc();
    LL_CRITICAL("overall: %llu", time2-time1);
    
    if (ret != 0)
    {
        LL_CRITICAL("handle_request returned %d", ret);
        return -1;
    }
    char* tx_str = (char*) malloc(raw_tx_len * 2 + 1);
    char2hex(raw_tx, raw_tx_len, tx_str);

#ifdef VERBOSE
    dump_buf("tx body: ", raw_tx, raw_tx_len);
    printf("tx_str: %s\n", tx_str);
#endif
#ifdef E2E_BENCHMARK
    ret = -1;
#else
    ret = send_transaction(tx_str);
#endif
    if (ret != 0)
    {
        fprintf(stderr, "send_raw_tx returned %d\n", ret);
        return -1;       
    }
    else
    {
        dump_buf("new nonce being dumped: ", nonce, 32);
        // add accounting info
        dump_nonce(nonce);
    }
//    free(tx_str);
    return ret;
#else
    long next_wanted;
    get_last_scan(db, &next_wanted);
    next_wanted++;

    int ret = 0;
    Json::Value transaction;
    unsigned retry_n = 0;
    int sleep_sec;
    std::string filter_id;
    long highest_block;

    // TX_BUF_SIZE is defined in Enclave.edl
    uint8_t raw_tx[TX_BUF_SIZE] = {0};
    int raw_tx_len = 0;

    do
    {
        if (retry_n >= 5)
        {
            LL_CRITICAL("Exit after %d retries", retry_n);
            ret = -1;
            break;
        }

        if (retry_n > 0)
        {
            sleep_sec = 1 << retry_n; // 2^retry_n
            LL_CRITICAL("retry in %d seconds", sleep_sec);
            sleep(sleep_sec);
        }

        try {
            // how many blocks do we have now?
            highest_block = eth_blockNumber(RPC_HOSTNAME, RPC_PORT);

            if (highest_block < 0)
            {
                LL_CRITICAL("eth_blockNumber returns %ld", highest_block);
                throw EX_GET_BLOCK_NUM;
            }

            // if we've scanned all of them
            if (next_wanted > highest_block)
            {
                LL_NOTICE("Highest block is %ld, waiting for block %ld...", highest_block, next_wanted);
                throw EX_NOTHING_TO_DO;
            }

            // fetch up to the latest block
            for (; next_wanted <= highest_block; next_wanted++)
            {
                // create a new filter for next_wanted
                ret = eth_new_filter(RPC_HOSTNAME, RPC_PORT, filter_id, next_wanted, next_wanted);

                if (ret < 0)
                {
                    LL_CRITICAL("Error: can't create new filter!");
                    throw EX_CREATE_FILTER;
                }

                LL_NOTICE("detected block %ld", next_wanted);

                // get events of interest
                ret = eth_getfilterlogs(RPC_HOSTNAME, RPC_PORT, filter_id, transaction);
                if (ret != 0) {
                    LL_CRITICAL("Error: can't get filter logs");
                    throw EX_GET_FILTER_LOG;
                }

                if (transaction.isArray() && transaction.size() > 0)
                {
                    for (int i = 0; i < transaction.size(); i++)
                    {
                        std::vector<uint8_t> data;
                        const char* data_c = transaction[i]["data"].asCString();
                        fromHex(data_c, data);
#ifdef VERBOSE
                        hexdump("TX:", &data[0], data.size());
#endif
                        Request request(&data[0]);
                        LL_NOTICE("find an request (id=%lu)", request.id);

#ifdef VERBOSE
                        hexdump("req_data:", req_data, req_len * 32);
#endif

                        get_last_nonce(db, &nonce);

                        handle_request(eid, &ret, nonce,
                                       request.id,
                                       request.type,
                                       request.data,
                                       request.data_len,
                                       raw_tx,
                                       &raw_tx_len);

                        if (ret != 0)
                        {
                            LL_CRITICAL("%s returned %d", "handle_request", ret);
                            throw EX_HANDLE_REQ;
                        }

                        char* tx_str = static_cast<char*>( malloc(raw_tx_len * 2 + 1));
                        char2hex(raw_tx, raw_tx_len, tx_str);
#ifdef VERBOSE
                        dump_buf("tx body: ", raw_tx, raw_tx_len);
#endif
                        std::cout << "TX BINARY " << tx_str << std::endl;

                        ret = send_transaction(RPC_HOSTNAME, RPC_PORT, tx_str);
                        if (ret != 0)
                        {
                            LL_CRITICAL("send_raw_tx returned %d", ret);
                            throw EX_SEND_TRANSACTION;
                        }

                        LL_NOTICE("Response sent");
                        LL_LOG("new nonce being dumped: %d", nonce);
                        nonce++;
                        record_nonce(db, nonce);
                    }
                }
                LL_NOTICE("Done processing block %ld", next_wanted);
                record_scan(db, next_wanted);
                retry_n = 0;
            } // for (next_wanted <= highest_block)
        }
        catch (EX_REASONS e) {
            switch (e) {
                case EX_GET_BLOCK_NUM:
                case EX_CREATE_FILTER:
                case EX_GET_FILTER_LOG:
                case EX_HANDLE_REQ:
                case EX_SEND_TRANSACTION:
                    retry_n++;
                    break;
                case EX_NOTHING_TO_DO:
                    LL_CRITICAL("Nothing to do. Sleep for 5 seconds");
                    sleep(5);
                    break;
                default:
                    LL_CRITICAL("Unknown exception: %d", e);
                    return -1;
            }
        }
        catch (std::exception ex) {
            LL_CRITICAL("Unexpected exception %s", ex.what());
        }
        catch (...) {
            LL_CRITICAL("Unexpected exception!");
            retry_n++;
        }
    } while (true); // this loop never ends;
#endif
}


/*
int demo_test_loop(sgx_enclave_id_t eid)
{
    int ret = 0;
    // TX_BUF_SIZE is defined in Enclave.edl
    uint8_t raw_tx[TX_BUF_SIZE] = {0};
    int raw_tx_len = 0;

    int nonce = 6;

    uint64_t id = 0;
    uint8_t request_type = TYPE_STEAM_EX;
    uint32_t req_len = 6;
    size_t req_len_bytes = req_len * 32;

    uint8_t* req_data = static_cast<uint8_t*>(malloc(req_len * 32));
    memset(req_data, 0xF0, req_len_bytes);
                        
    handle_request(eid, &ret, nonce, id, request_type, req_data, req_len * 32, raw_tx, &raw_tx_len);
    if (ret)
    {
       LL_CRITICAL("%s returned %d", "handle_request", ret);
       return -1;
    }

    char* tx_str = static_cast<char*>( malloc(raw_tx_len * 2 + 1));
    char2hex(raw_tx, raw_tx_len, tx_str);
    dump_buf("tx body: ", raw_tx, raw_tx_len);
    printf("tx_str: %s\n", tx_str);
//    ret = send_transaction(tx_str);
    ret = -1;
    if (ret != 0)
    {
        fprintf(stderr, "send_raw_tx returned %d\n", ret);
        return -1;       
    }
    else
    {
        record_nonce(db, ++nonce);
    }
    return 0;
}
*/
