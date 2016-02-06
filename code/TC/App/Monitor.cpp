#include <iostream>
#include "Monitor.h"
#include "EthRPC.h"
#include "Log.h"
#define _WINSOCKAPI_
#include "windows.h"
#include <map>
#include <string>
#include "jsonrpc.h"

#define BLOCK_ID_MAX    5
#define TX_ID_MAX       5

int monitor_loop()
{
    unsigned block_idx = 1;
    unsigned tx_idx = 0;

    int ret = 0;
    Json::Value transaction;
    int retry_n = 0;
    int sleep_sec;
    bool loop = true;

#if (defined(BLOCK_ID_MAX) && defined(TX_ID_MAX))
    while (block_idx < BLOCK_ID_MAX && tx_idx < TX_ID_MAX)
#else
    while (loop)
#endif
    {
        LL_NOTICE("Scanning %d:%d", block_idx, tx_idx);

        if (retry_n >= 5)
        {
            LL_CRITICAL("Exit after %d retries", retry_n);
            ret = -1;
            break;
        }
        try 
        {
            ret = fetch_request("localhost", 8200, block_idx, tx_idx, transaction);
            if (ret != 0) goto on_error;
            std::cout << transaction << std::endl;
            tx_idx += 1;
            continue;
        }

        catch (std::invalid_argument& ex)
        {
            LL_CRITICAL("%s", ex.what());
            goto on_error;
        }

        catch (std::out_of_range& ex)
        {
            block_idx ++;
            tx_idx = 0;
            continue;
        }
        catch (std::exception& ex)
        {
            LL_CRITICAL("%s", ex.what());
            goto on_error;
        }
on_error:
        sleep_sec = (int) pow(2, retry_n);
        LL_CRITICAL("retry in %d seconds", sleep_sec);
        Sleep(sleep_sec * 1000);
        retry_n ++;
        continue;
    }

    return ret;
}