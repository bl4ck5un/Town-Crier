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
#include <chrono>
#include <thread>

#include "Monitor.h"
#include "EthRPC.h"
#include "Log.h"

#include "Enclave_u.h"
#include "Constants.h"
#include "bookkeeping/database.hxx"
#include <iomanip>
#include "request-parser.hxx"
#include "Converter.h"

uint8_t resp_buffer[TX_BUF_SIZE] = {0};  //! TX_BUF_SIZE is defined in Constants.h
size_t resp_data_len = 0;

void Monitor::loop() {
  blocknum_t next_block_num = 0; //!< @next_wanted keeps track of the blocks that have been processed
  next_block_num = driver.getLastBlock();
  next_block_num++;

  // get nonce
//  size_t nonce = driver.getNumOfResponse();

  int ret = 0;
  Json::Value transaction;
  unsigned retry_counter = 0;       //Number of retries
  unsigned int sleep_time_sec = 1;

  while (true) {
    if (quit.load()) {
      LL_NOTICE("Ctrl-C pressed, cleaning up...");
      ret = 1;
      break;
    }
    if (retry_counter >= kRetryAllowed) {
      LL_CRITICAL("Exit after %d retries", retry_counter);
      ret = -1;
      break;
    }

    if (retry_counter > 0) {
      sleep_time_sec *= 2;
      LL_CRITICAL("retry in %d seconds", sleep_time_sec);
      sleep(sleep_time_sec);
    }

    try {
      long current_highest_block = eth_blockNumber();
      LL_LOG("highest block = %d", current_highest_block);

      if (current_highest_block < 0) {
        LL_CRITICAL("eth_blockNumber returns %ld", current_highest_block);
        throw runtime_error("eth_blockNumber returns " + current_highest_block);
      }

      // if we've scanned all of them
      if (next_block_num > current_highest_block) {
        LL_NOTICE("Highest block is %ld, waiting for block %ld...", current_highest_block, next_block_num);
        throw EX_NOTHING_TO_DO;
      }

      // fetch up to the latest block
      for (; next_block_num <= current_highest_block; next_block_num++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200)); 
        if (quit.load()) {
            LL_NOTICE("Ctrl-C pressed, cleaning up...");
            ret = 1;
            break;
        }
        LL_LOG("processing block %d", next_block_num);

        Json::Value txn_list;
        string filter_id = eth_new_filter(next_block_num, next_block_num);

        LL_NOTICE("detected block %ld", next_block_num);

        eth_getfilterlogs(filter_id, txn_list);

        LL_LOG("get filter logs returned");

        if (txn_list.isArray()) {
          if (txn_list.empty()) {
            //!< log an empty entry to note that we've scanned this block albeit empty
            TransactionRecord __tr(next_block_num, "no_tx_in_" + std::to_string(next_block_num), "");
            driver.logTransaction(__tr);
          }
//          for (int i = 0; i < txn_list.size(); i++) {
          for (auto __tx : txn_list) {
            /*!
             * process each request
             */
            const string dataField = "data";
            const string txHashField = "transactionHash";

            LL_DEBUG("raw_request: %s", __tx.toStyledString().c_str());
            if (!(__tx.isMember(dataField) && __tx.isMember(txHashField))) {
              LL_CRITICAL("get bad RPC data, skipping");
              continue;
            }

            Request request(__tx[dataField].asString());
            LL_NOTICE("%s", request.toString().c_str());

            //!< try to get txn from the database
            OdbDriver::record_ptr __tr = driver.getLogByHash(__tx[txHashField].asString());
            if (__tr) {
              // TODO: 5 is chosen arbitarily
              if (!__tr->getResponse().empty() || __tr->getNumOfRetrial() > 5) {
                LL_NOTICE("this request has fulfilled (or can't be fulfilled), skipping");
                continue;
              }
            } else {
              //!< if no record found, create a new one
              __tr.reset(new TransactionRecord(next_block_num, __tx[txHashField].asString(), request.getRawRequest()));
              driver.logTransaction(*__tr);
            }

            sgx_status_t ecall_ret;
            long nonce = eth_getTransactionCount();
            LL_LOG("nonce obtained %d\n", nonce);
            ecall_ret = handle_request(eid, &ret, nonce, request.getId(), request.getType(),
                                       request.getData(), request.getDataLen(),
                                       resp_buffer, &resp_data_len);

            if (ecall_ret != SGX_SUCCESS || ret != 0) {
              //!< if ecall fails, increment the number and skip
              LL_CRITICAL("%s returned %d, INVALID", "handle_request", ret);
              __tr->incrementNumOfRetrial();
              continue;
            } else {
              //!< if ecall succeeds, try to send transaction and record
              string resp_txn = bufferToHex(resp_buffer, resp_data_len, true);
              std::cout << "TX BINARY " << resp_txn << std::endl;

              string resp_txn_hash = send_transaction(resp_txn);
              LL_NOTICE("Response sent");

              __tr->incrementNumOfRetrial();
              __tr->setResponse(resp_txn);
              __tr->setResponseTime(std::time(0));
              driver.updateLog(*__tr);
            }
          }
        }
        LL_NOTICE("Done processing block %ld", next_block_num);
        retry_counter = 0; //! reset retry_counter upon each success
      }
    }
    catch (EX_REASONS e) {
      switch (e) {
        case EX_GET_BLOCK_NUM:
        case EX_CREATE_FILTER:
        case EX_GET_FILTER_LOG:
        case EX_HANDLE_REQ:
        case EX_SEND_TRANSACTION:retry_counter++;
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
      retry_counter++;
    }
    catch (const std::invalid_argument &ex) {
      LL_CRITICAL("%s", ex.what());
      retry_counter++;
    }
    catch (const std::exception &ex) {
      LL_CRITICAL("Unexpected exception %s", ex.what());
      retry_counter++;
    }
    catch (...) {
      LL_CRITICAL("Unexpected exception!");
      retry_counter++;
    }
  } // while (true)
}

