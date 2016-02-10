#include <iostream>
#include "Monitor.h"
#include "EthRPC.h"
#include "Log.h"
#define _WINSOCKAPI_
#include "windows.h"
#include <map>
#include <string>
#include "jsonrpc.h"
#include "Enclave_u.h"
#include "App.h"
#include <Debug.h>
#include "Utils.h"

#define RPC_HOSTNAME    "localhost"
#define RPC_PORT        8200

//#define SIMULATE

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


int monitor_loop(uint8_t* nonce)
{
#ifdef SIMULATE
    int ret;
    char req[64] = {0};
    req[0] = 'G';
    req[1] = 'O';
    req[2] = 'O';
    req[3] = 'G';
    req[4] = 'L';
    char tx[1024];
    handle_request(global_eid, &ret, 1, 1, req, sizeof req, tx, sizeof tx);
    return ret;
#else
    unsigned next_wanted = 9;

    int ret = 0;
    Json::Value transaction;
    int retry_n = 0;
    int sleep_sec;
    int filter_id;
    unsigned long highest_block;

    // 2048 is defined in Enclave.edl
    uint8_t raw_tx[2048] = {0};
    int raw_tx_len = 0;

    eth_getfilterlogs(RPC_HOSTNAME, RPC_PORT, 0, transaction);

    do
    {
        highest_block = eth_blockNumber(RPC_HOSTNAME, RPC_PORT);
        
        if (next_wanted > highest_block)
        {
            LL_NOTICE("We're too ahead.. Waiting for new blocks...");
            Sleep(5000);
            continue;
        }

        // fetch up to the latest block
        while (next_wanted <= highest_block)
        {
            ret = eth_new_filter(RPC_HOSTNAME, RPC_PORT, &filter_id, next_wanted, next_wanted);
            if (ret)
            {
                LL_CRITICAL("Error: can't create new filter!");
                return -1;
            }
            LL_NOTICE("Scanning %d", next_wanted);

            if (retry_n >= 5)
            {
                LL_CRITICAL("Exit after %d retries", retry_n);
                ret = -1;
                return -1;
            }
            try 
            {
                ret = eth_getfilterlogs(RPC_HOSTNAME, RPC_PORT, filter_id, transaction);
                if (ret != 0) goto retry;

                // reset retry counter
                retry_n = 0;

                if (transaction.isArray() && transaction.size() > 0)
                {
                    for (int i = 0; i < transaction.size(); i++)
                    {
                        std::vector<uint8_t> data;
                        const char* data_c = transaction[i]["data"].asCString();
                        fromHex(data_c, data);
                        // RequestInfo(uint64 id, uint8 requestType, address requester, uint fee, address callbackAddr, uint reqLen, bytes32[] requestData);
                        // 0 - 32 bytes : uint64 id
                        // 32- 64 bytes : uint8  requestType
                        // 64- 96 bytes : request
                        // 96- 128      : free
                        // 128- 160     : cb
                        // 160- 192     : reqLen
                        // 192- 224     : offset
                        // 224- 256     : reqLen
                        // 256-         : reqData
                        uint64_t id;
                        uint8_t* start = &data[0];
                        uint8_t request_type;
                        uint32_t req_len;

                        // get id
                        memcpy(&id,             start + 32 - sizeof uint64_t, sizeof uint64_t);
                        id = swap_uint64(id);
                        // get type
                        memcpy(&request_type,   start + 64 - sizeof uint8_t, sizeof uint8_t);
                        // get req_len
                        memcpy(&req_len,        start + 192 - sizeof uint32_t, sizeof uint32_t);
                        req_len = swap_uint32(req_len);
                        // get req_data
                        uint8_t* req_data = (uint8_t*) malloc(req_len);
                        memcpy(req_data,       start + 256, req_len * 32);
                        hexdump("title", req_data, req_len * 32);
                        
                        handle_request(global_eid, &ret, nonce, id, request_type, req_data, req_len * 32, raw_tx, &raw_tx_len);

                        char* tx_str = (char*) malloc(raw_tx_len * 2 + 1);
                        char2hex(raw_tx, raw_tx_len, tx_str);
//                    #ifdef VERBOSE
                        dump_buf("tx body: ", raw_tx, raw_tx_len);
                        printf("tx_str: %s\n", tx_str);
//                    #endif
                        ret = send_transaction(tx_str);
                        if (ret != 0)
                        {
                            fprintf(stderr, "send_raw_tx returned %d\n", ret);
                            return -1;       
                        }
                        else
                        {
                            dump_buf("new nonce being dumped: ", nonce, 32);
                            dump_nonce(nonce);
                        }
                    }
                }

                next_wanted += 1;
                continue;
            }

            catch (std::invalid_argument& ex)
            {
                // returned error
                LL_CRITICAL("%s", ex.what());
                goto retry;
            }
            catch (std::runtime_error& ex)
            {
                // networking error etc.
                LL_CRITICAL("%s", ex.what());
                goto retry;
            }

            catch (std::exception& el)
            {
                // all others
                LL_CRITICAL("%s", el.what());
                goto retry;
            }
 retry:
            sleep_sec = static_cast<int>(pow(2, retry_n));
            LL_CRITICAL("retry in %d seconds", sleep_sec);
            Sleep(sleep_sec * 1000);
            retry_n ++;
            continue;

        } // while (next_wanted <= highest_block)
    
    } while (true); // this loop never ends;


    return ret;
#endif
}