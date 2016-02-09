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

#define BLOCK_ID_MAX    5
#define TX_ID_MAX       5
#define RPC_HOSTNAME    "localhost"
#define RPC_PORT        8200

#define SIMULATE

int monitor_loop()
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
    unsigned next_wanted = 1;
    unsigned tx_idx = 0;

    int ret = 0;
    Json::Value transaction;
    int retry_n = 0;
    int sleep_sec;
    bool loop = true;

    unsigned long highest_block;
    
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
            LL_NOTICE("Scanning %d:%d", next_wanted, tx_idx);

            if (retry_n >= 5)
            {
                LL_CRITICAL("Exit after %d retries", retry_n);
                ret = -1;
                break;
            }
            try 
            {
                ret = fetch_request(RPC_HOSTNAME, RPC_PORT, next_wanted, tx_idx, transaction);
                if (ret != 0) goto retry;

                // reset retry counter
                retry_n = 0;

                // no such as contract
                if (transaction.isNull())
                {            
                    next_wanted ++;
                    tx_idx = 0;
                    continue;
                }

                if (!transaction["to"].isNull())
                {
                    std::cout << "To: " << transaction["to"] << std::endl;
                    // parse the data

                }

                tx_idx += 1;
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