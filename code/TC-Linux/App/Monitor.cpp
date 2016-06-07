#include <iostream>
#include "Monitor.h"
#include "EthRPC.h"
#include "Log.h"

#ifdef _WIN32
#define _WINSOCKAPI_
#include "windows.h"
#endif


#include "jsonrpc.h"
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

int monitor_loop(sgx_enclave_id_t eid)
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
    int next_wanted;
    get_last_scan(db, &next_wanted);
    next_wanted++;

    int ret = 0;
    Json::Value transaction;
    unsigned retry_n = 0;
    int sleep_sec;
    std::string filter_id;
    unsigned long highest_block;
    int nonce = 0;

    // TX_BUF_SIZE is defined in Enclave.edl
    uint8_t raw_tx[TX_BUF_SIZE] = {0};
    int raw_tx_len = 0;

    do
    {
        if (retry_n >= 5)
        {
            LL_CRITICAL("Exit after %d retries", retry_n);
            ret = -1;
            return -1;
        }

        if (retry_n > 0)
        {
            sleep_sec = static_cast<int>(pow(2, retry_n));
            LL_CRITICAL("retry in %d seconds", sleep_sec);
#ifdef _WIN32
            Sleep(sleep_sec * 1000);
#else
            sleep(sleep_sec);
#endif
        }

        // how many blocks do we have now?
        try
        {
            highest_block = eth_blockNumber(RPC_HOSTNAME, RPC_PORT);
            retry_n = 0;
        }
        catch (std::exception& ex)
        {
            LL_NOTICE("%s", ex.what());
            retry_n++;
            continue;
        }

        // if we've scanned all of them
        if (next_wanted > highest_block)
        {
            LL_NOTICE("waiting for block No.%d...", next_wanted);
#ifdef _WIN32
            Sleep(5000);
#else
            sleep(5);
#endif
            continue;
        }

        // fetch up to the latest block
        while (next_wanted <= highest_block)
        {
            // create a new filter for next_wanted
            try
            {
                ret = eth_new_filter(RPC_HOSTNAME, RPC_PORT, filter_id, next_wanted, next_wanted);
                retry_n = 0;
            }
            catch (std::exception& ex)
            {
                LL_CRITICAL("%s", ex.what());
                retry_n++;
                continue;
            }

            if (ret)
            {
                LL_CRITICAL("Error: can't create new filter!");
                return -1;
            }

            LL_NOTICE("detected block %d", next_wanted);

            try 
            {
                // get events of interest
                ret = eth_getfilterlogs(RPC_HOSTNAME, RPC_PORT, filter_id, transaction);
                if (ret != 0) {
                    retry_n++;
                    continue;
                }

                // reset retry counter
                retry_n = 0;

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
                        // RequestInfo(uint64 id, uint8 requestType, address requester, uint fee, address callbackAddr, uint reqLen, bytes32[] requestData);
                        // 0x00 - 32 bytes : uint64 id
                        // 0x20 - 64 bytes : uint8  requestType
                        // 0x40 - 96 bytes : requester
                        // 0x60 - 128      : fee
                        // 0x80 - 160     : cb
                        // 0xa0 - 192     : hash
                        // 0xc0 - 224     : offset
                        // 0xe0- 256      : reqLen
                        // 0x100-         : reqData
                        uint8_t* start = &data[0];
                        uint64_t request_id;
                        uint8_t request_type;
                        uint32_t req_len;

                        // get id
                        memcpy(&request_id,             start + 32 - sizeof ( uint64_t ), sizeof ( uint64_t ));
                        request_id = swap_uint64(request_id);

                        // get type
                        memcpy(&request_type,   start + 64 - sizeof ( uint8_t ), sizeof ( uint8_t ));

                        // get req_len
                        // note that req_len has the unit of bytes32
                        memcpy(&req_len,        start + 0xe0 + 32 - sizeof ( uint32_t ), sizeof ( uint32_t ));
                        req_len = swap_uint32(req_len);

                        // get req_data
                        uint8_t* req_data = static_cast<uint8_t*>(malloc(req_len * 32));
                        memcpy(req_data, start + 0x100, req_len * 32);

                        LL_NOTICE("find an request (id=%llu)", request_id);

#ifdef VERBOSE
                        hexdump("req_data:", req_data, req_len * 32);
#endif
                        
                        get_last_nonce(db, &nonce);

                        handle_request(eid, &ret, nonce, request_id, request_type, req_data, req_len * 32, raw_tx, &raw_tx_len);
                        if (ret != 0)
                        {
                            LL_CRITICAL("%s returned %d", "handle_request", ret);
                            retry_n++;
                            continue;
                        }
                        char* tx_str = static_cast<char*>( malloc(raw_tx_len * 2 + 1));
                        char2hex(raw_tx, raw_tx_len, tx_str);
#ifdef VERBOSE
                        dump_buf("tx body: ", raw_tx, raw_tx_len);
                        printf("tx_str: %s\n", tx_str);
#endif

                        ret = send_transaction(RPC_HOSTNAME, RPC_PORT, tx_str);
                        if (ret != 0)
                        {
                            fprintf(stderr, "send_raw_tx returned %d\n", ret);
                            return -1;       
                        }
                        else
                        {
                            LL_NOTICE("Response sent");
                            LL_LOG("new nonce being dumped: %d", nonce);
                            nonce++;
                            record_nonce(db, nonce);
                        }
                    }
                }
                LL_NOTICE("Done processing block %d", next_wanted);
                record_scan(db, next_wanted);
                next_wanted += 1;
                retry_n = 0;
                continue;
            }
            catch (std::exception& el)
            {
                std::string exp(el.what());
                LL_CRITICAL("%s", el.what());
                if (exp.find("too low"))
                {
                    nonce++;
                    record_nonce(db, nonce);
                }
                if (exp.find("invalid sender"))
                {
                    dump_buf("TX dump", raw_tx, raw_tx_len);
                    return -100;
                }
                retry_n++;
                continue;
            }
            retry_n = 0;
        } // while (next_wanted <= highest_block)
    
    } while (true); // this loop never ends;
#endif
}


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
