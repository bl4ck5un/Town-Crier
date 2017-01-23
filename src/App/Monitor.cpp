/*!
 * @file Monitor.cpp
 * @brief Main event loops of Town Crier
 *
 * the main monitor loop that receives requests from the Town Crier
 * contract, relays the request to the Enclave, and then relays the response from the
 * Enclave back to the Town Crier contract. In addition, Monitor.cpp can be run in various modes
 * usefull to debugging and running benchmarks it contains various flags:
 *      VERBOSE: Debugging flag that allows outputs various variable values
 *      E2E_BENCHMARK: Run a benchmark test on the enclave, to determing the
 *                     time it tries to run an handle_request enclave call
 *                     Note: The monitor loops terminates after one call to Handle Request
 *
 * @bug: no known bugs
 */

#include <iostream>
#include <unistd.h>

#include "Monitor.h"
#include "EthRPC.h"
#include "Log.h"

#include "Enclave_u.h"
#include <Constants.h>
#include "Bookkeeping.h"
#include <iomanip>

/*!
 * @param eid
 * @param nonce
 * Monitor_loop is the main loop that will relay all request from the TC contract
 * to enclave {@param eid}. Then, relay back the results to RPC_HOSTNAME in RPC_PORT
 * When the monitor recieved a request from a block, relay message, the for some reason the
 * request fails, retry up to 5 times.  If the request fails 5 times, the monitor_loop exists
 * If a request succeeds, the result will sent back to the blockchain
 */

void processOne(long nextBlockIndex, int nonce, sgx_enclave_id_t eid);

// TX_BUF_SIZE is defined in Enclave.edl
uint8_t raw_tx[TX_BUF_SIZE] = {0};
int raw_tx_len = 0;

void monitor_loop(sgx_enclave_id_t eid, int nonce) {
  long nextBlockIndex = 0; //!< @next_wanted keeps track of the blocks that have been processed
  get_last_scan(db, &nextBlockIndex);
  nextBlockIndex++;

  int ret = 0;
  Json::Value transaction;
  unsigned retryCounter = 0;       //Number of retries
  unsigned int sleepTimeInSec = 1;
  std::string filterID;
  long highestBlock;

  while (true) {
    if (retryCounter >= 8) {
      LL_CRITICAL("Exit after %d retries", retryCounter);
      ret = -1;
      break;
    }

    if (retryCounter > 0) {
      //Increase timeout everytime we retry
      sleepTimeInSec *= 2;
      LL_CRITICAL("retry in %d seconds", sleepTimeInSec);
      sleep(sleepTimeInSec);
    }

    try {
      highestBlock = eth_blockNumber();
      LL_LOG("highest block = %d", highestBlock);

      if (highestBlock < 0) {
        LL_CRITICAL("eth_blockNumber returns %ld", highestBlock);
        throw runtime_error("eth_blockNumber returns " + highestBlock);
      }

      // if we've scanned all of them
      if (nextBlockIndex > highestBlock) {
        LL_NOTICE("Highest block is %ld, waiting for block %ld...", highestBlock, nextBlockIndex);
        throw EX_NOTHING_TO_DO;
      }

      // fetch up to the latest block
      for (; nextBlockIndex <= highestBlock; nextBlockIndex++) {
        processOne(nextBlockIndex, nonce, eid);
        retryCounter = 0; //! reset retryCounter upon each success
      }
    }
    catch (EX_REASONS e) {
      switch (e) {
        case EX_GET_BLOCK_NUM:
        case EX_CREATE_FILTER:
        case EX_GET_FILTER_LOG:
        case EX_HANDLE_REQ:
        case EX_SEND_TRANSACTION:retryCounter++;
          break;
        case EX_NOTHING_TO_DO:LL_CRITICAL("Nothing to do. Sleep for 5 seconds");
          sleep(5);
          break;
        default:LL_CRITICAL("Unknown exception: %d", e);
          break;
      }
    }
    catch (const jsonrpc::JsonRpcException &ex) {
      LL_CRITICAL("RPC error: %s", ex.what());
      retryCounter++;
    }
    catch (const std::invalid_argument &ex) {
      LL_CRITICAL("%s", ex.what());
      retryCounter++;
    }
    catch (const std::exception &ex) {
      LL_CRITICAL("Unexpected exception %s", ex.what());
      retryCounter++;
    }
    catch (...) {
      LL_CRITICAL("Unexpected exception!");
      retryCounter++;
    }
  } // while (true)
}

void processOne(long nextBlockIndex, int nonce, sgx_enclave_id_t eid) {
  LL_LOG("processing block %d", nextBlockIndex);
  Json::Value txnContainer;

  string filterId = eth_new_filter(nextBlockIndex, nextBlockIndex);

  LL_NOTICE("detected block %ld", nextBlockIndex);

  eth_getfilterlogs(filterId, txnContainer);

  LL_LOG("get filter logs returned");

  if (txnContainer.isArray()) {
    for (int i = 0; i < txnContainer.size(); i++) {
      Request request(txnContainer[i]["data"].asString());
      LL_NOTICE("find an request (id=%lu)", request.id);
      LL_NOTICE("find an request (type=%lu)", request.type);

#ifdef VERBOSE
      std::cout << "REQDATA BINARY " << data_c << std::endl;
#endif

      get_last_nonce(db, &nonce);

      int ret = 0;
      sgx_status_t ecall_status;
      //Enclave call to parse the given request
      ecall_status = handle_request(eid, &ret, nonce,
                                    request.id,
                                    request.type,
                                    request.data,
                                    request.data_len,
                                    raw_tx,
                                    &raw_tx_len);

      if (ecall_status != SGX_SUCCESS || ret != 0) {
        LL_CRITICAL("%s returned %d, INVALID", "handle_request", ret);
        throw runtime_error("ecall returned " + ret);
      }
      //TODO: get rid of malloc
      // TODO: use bufferToHex instead
//      char *tx_str = static_cast<char *>( malloc(raw_tx_len * 2 + 1));
//      char2hex(raw_tx, raw_tx_len, tx_str);
      string txn = bufferToHex(raw_tx, raw_tx_len, true);
#ifdef VERBOSE
      std::cout << "REQDATA BINARY " << data_c << std::endl;
#endif
      std::cout << "TX BINARY " << txn << std::endl;

      send_transaction(txn);

      LL_NOTICE("Response sent");
      LL_LOG("new nonce being dumped: %d", nonce);
      nonce++;
      record_nonce(db, nonce);
    }
  }
  LL_NOTICE("Done processing block %ld", nextBlockIndex);
  record_scan(db, nextBlockIndex);
}