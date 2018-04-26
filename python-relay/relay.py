#!/bin/env python

import time
import pickle
import os
import logging
import requests
import json
import argparse
import sys

from web3 import Web3, HTTPProvider

logging.basicConfig(format='%(asctime)s %(levelname)-8s [%(filename)s:%(lineno)-4d] %(message)s',
                    datefmt='%d-%m-%Y:%H:%M:%S',
                    level=logging.INFO)
logger = logging.getLogger(__name__)


class TcLog:
    def __init__(self):
        self.last_processed_block = 0
        self.processed_txn_in_next_block = []
        self.n_txn_in_next_block = 0

    def __str__(self):
        return "last_processed_block={0}".format(self.last_processed_block)


class Request:
    def __init__(self, txid, data):
        self.txid = txid
        self.data = data


class BaseConfig:
    SGX_WALLET_ADDR = ""
    TC_CONTRACT_ADDR = ""
    TC_CONTRACT_BLOCK_NUM = 0


class ConfigSim(BaseConfig):
    SGX_WALLET_ADDR = Web3.toChecksumAddress("0x89b44e4d3c81ede05d0f5de8d1a68f754d73d997")
    TC_CONTRACT_ADDR = Web3.toChecksumAddress("0x18322346bfb90378ceaf16c72cee4496723636b9")
    TC_CONTRACT_BLOCK_NUM = 0


class ConfigHwAzure(BaseConfig):
    SGX_WALLET_ADDR = Web3.toChecksumAddress("0x3A8DE03F19C7C4C139B171978F87BFAC9FFE99C0")
    # https://rinkeby.etherscan.io/address/0x9ec1874ff1def6e178126f7069487c2e9e93d0f9
    TC_CONTRACT_ADDR = Web3.toChecksumAddress("0x9eC1874FF1deF6E178126f7069487c2e9e93D0f9")
    TC_CONTRACT_BLOCK_NUM = 2118268


class TCMonitor:
    ETH_RPC_ADDRESS = 'http://localhost:8545'

    TC_CORE_RPC_URL = "http://localhost:8123"
    TC_REQUEST_TOPIC = "0x295780EA261767C398D062898E5648587D7B8CA371FFD203BE8B4F9A43454FFA"

    SIM_NET = 'sim'
    TEST_NET = 'rinkeby'

    NUM_OF_RETRY_ON_NETWORK_ERROR = 10

    def __init__(self, network, pickle_file):
        if network == self.SIM_NET:
            self.config = ConfigSim()
        elif network == self.TEST_NET:
            self.config = ConfigHwAzure()
        else:
            raise KeyError("{0} is unknown".format(network))

        self.PICKLE_FILE = pickle_file

        logger.info('pickle_file: {0}'.format(self.PICKLE_FILE))
        logger.info('sgx wallet addr: {0}'.format(self.config.SGX_WALLET_ADDR))
        logger.info('tc contract addr: {0}'.format(self.config.TC_CONTRACT_ADDR))

        if os.path.exists(self.PICKLE_FILE):
            try:
                with open(self.PICKLE_FILE, 'rb') as f:
                    self.record = pickle.load(f)
            except Exception as e:
                logger.error("cannot load log {0}".format(e))
                self.record = TcLog()
        else:
            logging.debug("creating empty log")
            self.record = TcLog()

        # start processing with the block in which tc contract is mined
        self.record.last_processed_block = max(self.record.last_processed_block,
                                               self.config.TC_CONTRACT_BLOCK_NUM)

        self.w3 = Web3(HTTPProvider(self.ETH_RPC_ADDRESS))
        if not self.w3.isConnected():
            logger.info('cannot connect to {0}'.format(self.ETH_RPC_ADDRESS))
            sys.exit(1)
        else:
            logger.info('connected to {0}'.format(self.w3.version.node))

    def _get_requests_in_block(self, block):
        filter_obj = {"fromBlock": block, "toBlock": block, "address": self.config.TC_CONTRACT_ADDR,
                      "topics": [self.TC_REQUEST_TOPIC]}

        logs = self.w3.eth.getLogs(filter_obj)

        logger.info("{0} requests find in block {1}".format(len(logs), block))

        requests = []
        for log in logs:
            requests.append(Request(Web3.toHex(log['transactionHash']), log['data']))

        return requests

    def _update_record_one_request(self, req):
        self.record.processed_txn_in_next_block.append(req)
        with open(self.PICKLE_FILE, 'wb') as f:
            pickle.dump(self.record, f)
        logger.info('done update')

    def _update_record_one_block(self):
        self.record.last_processed_block += 1
        self.record.processed_txn_in_next_block = []
        with open(self.PICKLE_FILE, 'wb') as f:
            pickle.dump(self.record, f)
        logger.info('done processing block {0}'.format(self.record.last_processed_block))

    def _process_request(self, req):
        logger.info("processing request {0}".format(req.txid))

        nonce = self.w3.eth.getTransactionCount(self.config.SGX_WALLET_ADDR)

        params = dict(
            data=req.data,
            txid=req.txid,
            nonce=nonce,
        )

        payload = {
            'method': 'process',
            'params': params,
            'jsonrpc': '2.0',
            'id': 0,
        }

        resp = requests.post(self.TC_CORE_RPC_URL, data=json.dumps(payload),
                             headers={'content-type': 'application/json'}).json()
        if 'error' in resp:
            logger.error('Error: {0}'.format(resp['error']))
        else:
            error_code = resp['result']['error_code']
            response_tx = resp['result']['response']
            if error_code != 0:
                logger.error('Error in tx: {0}'.format(error_code))
            logger.info('response from enclave: {0}'.format(response_tx))
            txid = self.w3.eth.sendRawTransaction(response_tx)
            self._update_record_one_request(req)

            logger.info("response sent {0}".format(Web3.toHex(txid)))

    def loop(self):
        next_block = self.config.TC_CONTRACT_BLOCK_NUM
        while True:
            try:
                next_block = max(next_block, self.record.last_processed_block + 1)
                geth_block = self.w3.eth.blockNumber

                if next_block > geth_block:
                    logger.debug("waiting for block #{0} (geth is at #{1})".format(next_block, geth_block))
                    time.sleep(2)
                    continue

                logger.info("processing block {0}".format(next_block))

                reqs = self._get_requests_in_block(next_block)
                for req in reqs:
                    if req not in self.record.processed_txn_in_next_block:
                        retry = 0
                        while retry < self.NUM_OF_RETRY_ON_NETWORK_ERROR:
                            time.sleep(2 ** retry)
                            try:
                                self._process_request(req)
                                break
                            except requests.RequestException as e:
                                logger.error('exception: {0}'.format(str(e)))
                            except Exception as e:
                                logger.error('exception: {0}'.format(str(e)))
                            retry += 1

                self._update_record_one_block()
            # catch everything (e.g. errors in RPC call with geth) and continue
            except Exception as e:
                logger.error('exception: {0}'.format(str(e)))
                time.sleep(2)


parser = argparse.ArgumentParser(description="Town Crier Ethereum relay")
parser.add_argument('-v', action='store_true', dest='verbose', help='Verbose')
parser.add_argument('-t', action='store_true', dest='testnet', help='Enable testnet')
parser.add_argument('--db', action='store', dest='database', default='/relay/tc.bin',
                    help='where to store the runtime log')

args = parser.parse_args()
args.parser = parser

logger.setLevel('INFO')
network = TCMonitor.SIM_NET

if args.verbose:
    logger.setLevel('DEBUG')

if args.testnet:
    network = TCMonitor.TEST_NET

monitor = TCMonitor(network, args.database)
monitor.loop()
