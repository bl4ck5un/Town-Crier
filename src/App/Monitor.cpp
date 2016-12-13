/* Monitor.cpp is the main monitor loop that recieves requests from the Town Crier
 * contract, relays the request to the Enclave, and then relays the response from the
 * Enclave back to the Town Crier contract. In addition, Monitor.cpp can be run in various modes
 * usefull to debugging and running benchmarks it contains various flags:
 *      VERBOSE: Debugging flag that allows outputs various variable values
 *
 */

#include <iostream>
#include <math.h>
#include <unistd.h>
#include <iomanip>
#include <Constants.h>
#include <syslog.h>

#include "Monitor.h"
#include "EthRPC.h"
#include "Enclave_u.h"
#include "App.h"
#include "Utils.h"
#include "RemoteAtt.h"
#include "Constants.h"
#include "Bookkeeping.h"

#ifdef _WIN32
#define _WINSOCKAPI_
#include "windows.h"
#endif

#define RPC_HOSTNAME    "localhost"
#define RPC_PORT        8545

#define VERBOSE

/* Let x = 0xABCDEFGH then swap(x) returns 0XHGFEDCBA */ 
inline uint64_t swap_uint64(uint64_t X) {
    uint64_t x = X;
    x = (x & 0x00000000FFFFFFFF) << 32 | (x & 0xFFFFFFFF00000000) >> 32;
    x = (x & 0x0000FFFF0000FFFF) << 16 | (x & 0xFFFF0000FFFF0000) >> 16;
    x = (x & 0x00FF00FF00FF00FF) << 8  | (x & 0xFF00FF00FF00FF00) >> 8;
    return x;
}

/* let x = 0xABCD then swap(x) return DCBA */
inline uint32_t swap_uint32( uint32_t num )
{
    return ((num>>24)&0xff) | // move byte 3 to byte 0
                    ((num<<8)&0xff0000) | // move byte 1 to byte 2
                    ((num>>8)&0xff00) | // move byte 2 to byte 1
                    ((num<<24)&0xff000000); // byte 0 to byte 3
}
//Constant that is never used
long g_frequency = 2500000000;

/* Monitor_loop [eid] [nonce] is the main loop that will relay all request from the TC contract
 * to enclave [eid]. Then, relay back the results to RPC_HOSTNAME in RPC_PORT
 * When the monitor recieved a request from a block, relay message, the for some reason the
 * request fails, retry up to 5 times.  If the request fails 5 times, the monitor_loop exists
 * If a request succeds, the result will sent back to the blockchain
 */
int monitor_loop(sgx_enclave_id_t eid, int nonce)
{
    //Case: No Benchmark standard loop
    long next_wanted;               //Next_wanted keeps track of the blocks that have been processed
    get_last_scan(db, &next_wanted);
    next_wanted++;

    int ret = 0;
    Json::Value transaction;
    unsigned retry_n = 0;       //Number of retries
    int sleep_sec;
    std::string filter_id;
    long highest_block;

    // TX_BUF_SIZE is defined in Enclave.edl
    uint8_t raw_tx[TX_BUF_SIZE] = {0};
    int raw_tx_len = 0;

    /* Begin the main system loop */
    while(true){

        if (retry_n >= 50){
            syslog(LOG_CRIT,"Failed after %d retries", retry_n);
            retry_n = 0;
            continue;
        }

        if (retry_n > 0){
            //Increase timeout everytime we retry
            sleep_sec = 1; // 2^retry_n

            syslog(LOG_INFO,"retry in %d seconds", sleep_sec);
            sleep(sleep_sec);
        }

        try{
            // how many blocks do we have now?
            highest_block = eth_blockNumber();

            if (highest_block < 0){
                syslog(LOG_CRIT,"eth_blockNumber returns %ld", highest_block);
                throw EX_GET_BLOCK_NUM;
            }

            // if we've scanned all of them
            if (next_wanted > highest_block){
                syslog(LOG_CRIT,"Highest block is %ld, waiting for block %ld...", highest_block, next_wanted);
                throw EX_NOTHING_TO_DO;
            }

            // fetch up to the latest block
            for (; next_wanted <= highest_block; next_wanted++){
                // create a new filter for next_wanted
                ret = eth_new_filter(filter_id, next_wanted, next_wanted);

                if (ret < 0){
                    syslog(LOG_CRIT,"Error: can't create new filter!");
                    throw EX_CREATE_FILTER;
                }
                syslog(LOG_INFO,"detected block %ld", next_wanted);

                // get events of interest
                ret = eth_getfilterlogs(filter_id, transaction);
                if (ret != 0) {
                    syslog(LOG_CRIT,"Error: can't get filter logs");
                    throw EX_GET_FILTER_LOG;
                }

                if (transaction.isArray() && transaction.size() > 0){
                    //Loop over all transactions in the block
                    for (int i = 0; i < transaction.size(); i++){
                        //Parse the request
                        std::vector<uint8_t> data;
                        const char* data_c = transaction[i]["data"].asCString();
                        fromHex(data_c, data);
                        Request request(&data[0]);//take vector of uint8
                        syslog(LOG_INFO,"find an request (id=%lu)", request.id);
                        syslog(LOG_INFO,"find an request (type=%lu)", request.type);

#ifdef VERBOSE
                        syslog(LOG_INFO, "REQDATA BINARY %s", data_c);
                        //std::cout << "REQDATA BINARY " << data_c << std::endl;
#endif

                        get_last_nonce(db, &nonce);

                        //Enclave call to parse the given request
                        handle_request(eid, &ret, nonce,
                                       request.id,
                                       request.type,
                                       request.data,
                                       request.data_len,
                                       raw_tx,
                                       &raw_tx_len);
                        if (ret != 0){
                            syslog(LOG_CRIT,"%s returned %d, INVALID", "handle_request", ret);
                            throw EX_HANDLE_REQ;
                        }
                        //TODO: get rid of malloc
                        // char* tx_str = static_cast<char*>( malloc(raw_tx_len * 2 + 1));
                        char tx_str[raw_tx_len * 2 + 1];
                        char2hex(raw_tx, raw_tx_len, tx_str);
#ifdef VERBOSE
                        std::cout << "REQDATA BINARY " << data_c << std::endl;
#endif
                        std::cout << "TX BINARY " << tx_str << std::endl;

                        ret = send_transaction(tx_str);
                        if (ret != 0){
                            syslog(LOG_CRIT,"send_raw_tx returned %d", ret);
                            throw EX_SEND_TRANSACTION;
                        }

                        syslog(LOG_INFO,"Response sent");
                        syslog(LOG_INFO,"new nonce being dumped: %d", nonce);
                        nonce++;
                        record_nonce(db, nonce);
                    }
                }
                syslog(LOG_INFO,"Done processing block %ld", next_wanted);
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
                case EX_NOTHING_TO_DO:
                    syslog(LOG_CRIT,"Nothing to do. Sleep for 5 seconds");
                    sleep(5);
                    continue;
                default:
                    syslog(LOG_CRIT,"Unknown exception: %d", e);
                    continue;
            }
        }
        catch (std::exception ex) {
            syslog(LOG_CRIT,"Unexpected exception %s", ex.what());
        }
        catch (...) {
            syslog(LOG_CRIT,"Unexpected exception!");
            retry_n++;
        }
        sleep(60);//Sleep for 60 seconds
    }
}