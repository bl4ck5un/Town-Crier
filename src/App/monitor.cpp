//
// Copyright (c) 2016-2018 by Cornell University.  All Rights Reserved.
//
// Permission to use the "TownCrier" software ("TownCrier"), officially
// docketed at the Center for Technology Licensing at Cornell University
// as D-7364, developed through research conducted at Cornell University,
// and its associated copyrights solely for educational, research and
// non-profit purposes without fee is hereby granted, provided that the
// user agrees as follows:
//
// The permission granted herein is solely for the purpose of compiling
// the TownCrier source code. No other rights to use TownCrier and its
// associated copyrights for any other purpose are granted herein,
// whether commercial or non-commercial.
//
// Those desiring to incorporate TownCrier software into commercial
// products or use TownCrier and its associated copyrights for commercial
// purposes must contact the Center for Technology Licensing at Cornell
// University at 395 Pine Tree Road, Suite 310, Ithaca, NY 14850; email:
// ctl-connect@cornell.edu; Tel: 607-254-4698; FAX: 607-254-5454 for a
// commercial license.
//
// IN NO EVENT SHALL CORNELL UNIVERSITY BE LIABLE TO ANY PARTY FOR
// DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES,
// INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF TOWNCRIER AND ITS
// ASSOCIATED COPYRIGHTS, EVEN IF CORNELL UNIVERSITY MAY HAVE BEEN
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// THE WORK PROVIDED HEREIN IS ON AN "AS IS" BASIS, AND CORNELL
// UNIVERSITY HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
// ENHANCEMENTS, OR MODIFICATIONS.  CORNELL UNIVERSITY MAKES NO
// REPRESENTATIONS AND EXTENDS NO WARRANTIES OF ANY KIND, EITHER IMPLIED
// OR EXPRESS, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, OR THAT THE USE
// OF TOWNCRIER AND ITS ASSOCIATED COPYRIGHTS WILL NOT INFRINGE ANY
// PATENT, TRADEMARK OR OTHER RIGHTS.
//
// TownCrier was developed with funding in part by the National Science
// Foundation (NSF grants CNS-1314857, CNS-1330599, CNS-1453634,
// CNS-1518765, CNS-1514261), a Packard Fellowship, a Sloan Fellowship,
// Google Faculty Research Awards, and a VMWare Research Award.
//

#include "App/monitor.h"

#include <math.h>
#include <unistd.h>
#include <algorithm>
#include <chrono>  // NOLINT
#include <iomanip>
#include <iostream>
#include <string>
#include <thread>  // NOLINT
#include <queue>
#include <memory>
#include <utility>

#include "App/converter.h"
#include "App/Enclave_u.h"
#include "App/eth_rpc.h"
#include "App/request_parser.h"
#include "App/logging.h"

namespace tc {
namespace monitor {
log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("monitor.cpp"));
}
}

// TX_BUF_SIZE is defined in Constants.h
uint8_t resp_buffer[TX_BUF_SIZE] = {0};
size_t resp_data_len = 0;

void Monitor::loop() {
  using tc::monitor::logger;
  // keeps track of the blocks that have been processed
  blocknum_t next_block_num;
  next_block_num = driver->getLastBlock();
#ifdef TC_SGX_IN_RELEASE_MODE
  if (next_block_num < 3706916)
      next_block_num = 3706916;
#endif
  next_block_num++;

  int ret = 0;
  Json::Value transaction;
  // number of retry for the current tx
  unsigned monitor_retry_counter = 0;
  unsigned int sleep_time_sec = 1;

  // handle interrupt nicely
  while (!quit.load()) {
    if (monitor_retry_counter > 0) {
      // doubling the sleeping time
      sleep_time_sec *= 2;
      sleep_time_sec = min(sleep_time_sec, (uint) 32);
      LL_ERROR("retry in %d seconds", sleep_time_sec);
      sleep(sleep_time_sec);
    }

    blocknum_t current_highest_block;

    try {
      current_highest_block = eth_blockNumber();
      LL_DEBUG("chain height=%ld, processed=%ld. %ld behind head...",
               current_highest_block, next_block_num - 1,
               current_highest_block - next_block_num);
    }

    catch (const jsonrpc::JsonRpcException &e) {
      LL_CRITICAL("can't get block height: %s", e.what());
      monitor_retry_counter++;
      continue;
    }

    if (current_highest_block == 0) {
      LL_INFO("geth not synced yet, waiting..");
      monitor_retry_counter++;
      continue;
    }

    // if we've scanned all of them
    if (next_block_num > current_highest_block) {
      // reset error counter after one success
      monitor_retry_counter = 0;
      if (!isSleeping) {
        LL_INFO("Nothing to do. Going to sleep...");
        isSleeping = true;
      }
      sleep(Monitor::nothingToDoSleepSec);
      continue;
    } else {
      // wake up the monitor
      isSleeping = false;
    }

    for (; next_block_num <= current_highest_block; next_block_num++) {
      try {
        /* when TC is run for the first time, this loop will be executed for
         * millions of time very quickly
         * so we need to handle interrupt here too. */
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        if (quit.load()) {
          LL_INFO("Ctrl-C pressed, cleaning up...");
          ret = 1;
          break;
        }
        LL_INFO("processing block %d", next_block_num);

        this->_process_one_block(next_block_num);
        monitor_retry_counter = 0;
      } catch (const jsonrpc::JsonRpcException &ex) {
        LL_ERROR("RPC error: %s", ex.what());
        monitor_retry_counter++;
      } catch (const std::invalid_argument &ex) {
        LL_ERROR("Invalid Argument: %s", ex.what());
        monitor_retry_counter++;
      } catch (const std::exception &ex) {
        LL_ERROR("Unexpected std exception %s", ex.what());
        monitor_retry_counter++;
      } catch (...) {
        LL_ERROR("Unexpected exception!");
        monitor_retry_counter++;
      }
    }
  }  // while (true)
}

//! best effort to process one block
//! \param blocknum
void Monitor::_process_one_block(blocknum_t blocknum) {
  using tc::monitor::logger;
  int ret = 0;
  Json::Value txn_list;
  string filter_id = eth_new_filter(blocknum, blocknum);
  eth_getfilterlogs(filter_id, &txn_list);

  LL_DEBUG("block %d => get %zu tx", blocknum, txn_list.size());

  if (txn_list.empty()) {
    /* log the empty blocks too */
    TransactionRecord _dummy_tr(blocknum, "no_tx_in_" + std::to_string(blocknum), "");
    _dummy_tr.setResponse("no_tx_in_" + std::to_string(blocknum));
    driver->logTransaction(_dummy_tr);
  }

  for (auto _current_tx : txn_list) {
    auto request = std::unique_ptr<tc::RequestParser>(new tc::RequestParser());
    try {
      LL_DEBUG("raw_request: %s", _current_tx.toStyledString().c_str());

      if (!(_current_tx.isMember(DATA_FIELD_NAME) && _current_tx.isMember(TX_HASH_FIELD_NAME))) {
        LL_ERROR("get bad RPC data, skipping this tx");
        continue;
      }

      string _current_tx_hash = _current_tx[TX_HASH_FIELD_NAME].asString();
      request->valueOf(_current_tx[DATA_FIELD_NAME].asString(), _current_tx_hash);

      LL_INFO("parsed request: %s", request->toString().c_str());

      /* try to get txn from the database */
      if (driver->isProcessed(_current_tx_hash, maxRetry)) {
        LL_INFO("request %s has been fulfilled (or can't be fulfilled), skipping", _current_tx_hash.c_str());
        continue;
      }

      LL_DEBUG("request %s has not been fulfilled", _current_tx_hash.c_str());

      // if no record found, create a new one
      if (!driver->isLogged(_current_tx_hash)) {
        TransactionRecord _log_record(blocknum, _current_tx_hash, request->getRawRequest());
        driver->logTransaction(_log_record);
        LL_INFO("request %s logged", _current_tx_hash.c_str());
      }

      OdbDriver::record_ptr log_entry;
      log_entry = driver->getLogByHash(_current_tx_hash);

      sgx_status_t ecall_ret;
      uint64_t nonce = eth_getTransactionCount();

      LL_INFO("nonce obtained %d", nonce);

      // TODO(FAN): change nonce to some long type
      ecall_ret = handle_request(eid, &ret, nonce, request->getId(),
                                 request->getType(), request->getData(),
                                 request->getDataLen(), resp_buffer, &resp_data_len);

      if (ecall_ret != SGX_SUCCESS || ret != TC_SUCCESS) {
        // increment the number and skip
        LL_ERROR("handle_request returned %d", ret);
        log_entry->incrementNumOfRetrial();
        failed_requests.push(std::move(request));
        continue;
      } else {
        string resp_txn = bufferToHex(resp_buffer, resp_data_len, true);
        LL_DEBUG("response tx: %s", resp_txn.c_str());

        if (send_tx) {
          string resp_txn_hash = send_transaction(resp_txn);
          log_entry->setResponse(resp_txn_hash);
          log_entry->setResponseTime(std::time(0));
        } else {
          LL_INFO("will NOT send tx");
        }

        log_entry->incrementNumOfRetrial();
        driver->updateLog(*log_entry);
      }
    }
    catch (const RequestParserException &ex) {
      LL_CRITICAL("bad request");
    }
    catch (const jsonrpc::JsonRpcException& e) {
      LL_CRITICAL("json rpc error: %s", e.what());
    }
    catch (const std::exception &e) {
      LL_CRITICAL("error happen while processing ", e.what());
      LL_DEBUG("before pushing to failed_queue");
      LL_DEBUG("after pushing to failed_queue");
      LL_CRITICAL("%s pushed to failed queue", request->toString());
      failed_requests.push(std::move(request));
    }
  }

  LL_LOG("going over the failed tx");
  while (!failed_requests.empty()) {
    LL_CRITICAL("redoing work for %s", failed_requests.front()->getTransactionHash().c_str());
    failed_requests.pop();
  }

  LL_INFO("Done processing block %ld", blocknum);
}

