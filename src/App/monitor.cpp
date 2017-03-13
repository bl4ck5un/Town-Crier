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
#include <iomanip>
#include <thread>

#include "monitor.h"
#include "EthRPC.h"
#include "Log.h"
#include "Enclave_u.h"
#include "request-parser.hxx"
#include "Converter.h"

// TX_BUF_SIZE is defined in Constants.h
uint8_t resp_buffer[TX_BUF_SIZE] = {0};
size_t resp_data_len = 0;

void Monitor::loop() {
  // keeps track of the blocks that have been processed
  blocknum_t next_block_num = 0;
  next_block_num = driver.getLastBlock();
  next_block_num++;

  int ret = 0;
  Json::Value transaction;
  // number of retry for the current tx
  unsigned monitor_retry_counter = 0;
  unsigned int sleep_time_sec = 1;

  while (true) {
    // handle interrupt nicely
    if (quit.load()) {
      LL_INFO("Ctrl-C pressed, cleaning up...");
      ret = 1;
      break;
    }

    if (monitor_retry_counter > 0) {
      // doubling the sleeping time
      sleep_time_sec *= 2;
      sleep_time_sec = min(sleep_time_sec, (uint) 32);
      LL_ERROR("retry in %d seconds", sleep_time_sec);
      sleep(sleep_time_sec);
    }

    try {
      blocknum_t current_highest_block = eth_blockNumber();
      LL_LOG("highest block = %ld", current_highest_block);

      if (current_highest_block < 0) {
        LL_ERROR("eth_blockNumber returns %ld", current_highest_block);
        throw EX_GET_BLOCK_NUM;
      }

      // if we've scanned all of them
      if (next_block_num > current_highest_block) {
        LL_INFO("Highest block is %ld, waiting for block %ld...", current_highest_block, next_block_num);
        throw EX_NOTHING_TO_DO;
      }

      for (; next_block_num <= current_highest_block; next_block_num++) {
        /* when TC is run for the first time, this loop will be executed for millions of time very quickly
         * so we need to handle interrupt here too. */
        std::this_thread::sleep_for(std::chrono::milliseconds(200)); 
        if (quit.load()) {
            LL_INFO("Ctrl-C pressed, cleaning up...");
            ret = 1;
            break;
        }
        LL_LOG("processing block %d", next_block_num);

        Json::Value txn_list;
        string filter_id = eth_new_filter(next_block_num, next_block_num);
        eth_getfilterlogs(filter_id, txn_list);

        if (txn_list.isArray()) {
          if (txn_list.empty()) {
            /* log the empty blocks too */
            TransactionRecord __tr(next_block_num, "no_tx_in_" + std::to_string(next_block_num), "");
            driver.logTransaction(__tr);
          }

          for (auto _current_tx : txn_list) {
            /* process each request */
            const string data_field_name = "data";
            const string tx_hash_field_name = "transactionHash";

            LL_DEBUG("raw_request: %s", _current_tx.toStyledString().c_str());

            if (!(_current_tx.isMember(data_field_name) && _current_tx.isMember(tx_hash_field_name))) {
              LL_ERROR("get bad RPC data, skipping this tx");
              continue;
            }

            Request request(_current_tx[data_field_name].asString());

            LL_INFO("parsed tx: %s", request.toString().c_str());

            /* try to get txn from the database */
            OdbDriver::record_ptr _tx_record = driver.getLogByHash(_current_tx[tx_hash_field_name].asString());
            if (_tx_record) {
              /* if tr has been processed before */
              if (!_tx_record->getResponse().empty() || _tx_record->getNumOfRetrial() > maxRetry) {
                LL_INFO("this request has fulfilled (or can't be fulfilled), skipping");
                continue;
              }
            } else {
              // if no record found, create a new one
              _tx_record.reset(new TransactionRecord(next_block_num,
                                               _current_tx[tx_hash_field_name].asString(),
                                               request.getRawRequest()));
              driver.logTransaction(*_tx_record);
            }

            sgx_status_t ecall_ret;
            long nonce = eth_getTransactionCount();
            LL_LOG("nonce obtained %d\n", nonce);
            // TODO: change nonce to have long type
            ecall_ret = handle_request(eid, &ret, nonce, request.getId(), request.getType(),
                                       request.getData(), request.getDataLen(),
                                       resp_buffer, &resp_data_len);

            if (ecall_ret != SGX_SUCCESS || ret != TC_SUCCESS) {
              // increment the number and skip
              LL_ERROR("%s returned %d, INVALID", "handle_request", ret);
              _tx_record->incrementNumOfRetrial();
              continue;
            } else {
              string resp_txn = bufferToHex(resp_buffer, resp_data_len, true);
              LL_LOG("resp bin %s", resp_txn.c_str());

              string resp_txn_hash = send_transaction(resp_txn);
              LL_INFO("Response sent");

              _tx_record->incrementNumOfRetrial();
              _tx_record->setResponse(resp_txn);
              _tx_record->setResponseTime(std::time(0));
              driver.updateLog(*_tx_record);
            }
          }
        }
        LL_INFO("Done processing block %ld", next_block_num);
        monitor_retry_counter = 0; //! reset retry_counter upon each success
      }
    }
    catch (EX_REASONS e) {
      switch (e) {
        case EX_GET_BLOCK_NUM:
        case EX_CREATE_FILTER:
        case EX_GET_FILTER_LOG:
        case EX_HANDLE_REQ:
        case EX_SEND_TRANSACTION:
          monitor_retry_counter++;
          break;
        case EX_NOTHING_TO_DO:
          LL_ERROR("Nothing to do. Sleep for 5 seconds");
          sleep(5);
          break;
        default:LL_ERROR("Unknown exception: %d", e);
          break;
      }
    }
    catch (const jsonrpc::JsonRpcException &ex) {
      LL_ERROR("RPC error: %s", ex.what());
      monitor_retry_counter++;
    }
    catch (const std::invalid_argument &ex) {
      LL_ERROR("%s", ex.what());
      monitor_retry_counter++;
    }
    catch (const std::exception &ex) {
      LL_ERROR("Unexpected exception %s", ex.what());
      monitor_retry_counter++;
    }
    catch (...) {
      LL_ERROR("Unexpected exception!");
      monitor_retry_counter++;
    }
  } // while (true)
}