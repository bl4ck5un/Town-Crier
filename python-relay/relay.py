#!/bin/env python

import time
from ethjsonrpc import EthJsonRpc
import pickle
import os
import logging
import requests
import json
import argparse


class TcLog:
    def __init__(self):
        self.last_processed_block = 0
        self.processed_txn_in_next_block = []
        self.n_txn_in_next_block = 0


class Request:
    def __init__(self, txid, data):
        self.txid = txid
        self.data = data


class BaseConfig:
    SGX_WALLET_ADDR = ""
    TC_CONTRACT_ADDR = ""
    TC_CONTRACT_BLOCK_NUM = 0
    PICKLE_FILE = ""


class ConfigSim(BaseConfig):
    SGX_WALLET_ADDR = "0x89b44e4d3c81ede05d0f5de8d1a68f754d73d997"
    TC_CONTRACT_ADDR = "0x18322346bfb90378ceaf16c72cee4496723636b9"
    TC_CONTRACT_BLOCK_NUM = 0
    PICKLE_FILE = 'tc.bin'


class ConfigHwAzure(BaseConfig):
    SGX_WALLET_ADDR = "0x3A8DE03F19C7C4C139B171978F87BFAC9FFE99C0"
    # https://rinkeby.etherscan.io/address/0x9ec1874ff1def6e178126f7069487c2e9e93d0f9
    TC_CONTRACT_ADDR = "0x9eC1874FF1deF6E178126f7069487c2e9e93D0f9"
    TC_CONTRACT_BLOCK_NUM = 2118268
    PICKLE_FILE = '/relay/tc.bin'


class TCMonitor:
    ETH_RPC_ADDRESS = 'localhost'
    ETH_RPC_PORT = 8545

    TC_CORE_RPC_URL = "http://localhost:8123"
    TC_REQUEST_TOPIC = "0x295780EA261767C398D062898E5648587D7B8CA371FFD203BE8B4F9A43454FFA"

    SIM_NET = 'sim'
    TEST_NET = 'rinkeby'

    NUM_OF_RETRY_ON_NETWORK_ERROR = 10

    def __init__(self, network):
        if network == self.SIM_NET:
            self.config = ConfigSim()
        elif network == self.TEST_NET:
            self.config = ConfigHwAzure()
        else:
            raise KeyError("{0} is unknown".format(network))

        print 'pickle_file:', self.config.PICKLE_FILE
        print 'sgx wallet addr:', self.config.SGX_WALLET_ADDR
        print 'tc contract addr:', self.config.TC_CONTRACT_ADDR

        if os.path.exists(self.config.PICKLE_FILE):
            try:
                with open(self.config.PICKLE_FILE, 'rb') as f:
                    self.record = pickle.load(f)
            except Exception as e:
                logging.error("cannot load log {0}".format(e))
                self.record = TcLog()
        else:
            self.record = TcLog()

        self.eth_rpc = EthJsonRpc(self.ETH_RPC_ADDRESS, self.ETH_RPC_PORT)
        logging.info('connected to {0}'.format(self.eth_rpc.web3_clientVersion()))

    def _get_requests_in_block(self, block):
        filter_obj = {"fromBlock": block, "toBlock": block, "address": self.config.TC_CONTRACT_ADDR,
                      "topics": [self.TC_REQUEST_TOPIC]}

        logs = self.eth_rpc.eth_getLogs(filter_obj)

        logging.info("{0} requests find in block {1}".format(len(logs), block))

        requests = []
        for log in logs:
            requests.append(Request(log['transactionHash'], log['data']))

        return requests

    def _update_record_one_request(self, req):
        self.record.processed_txn_in_next_block.append(req)
        with open(self.config.PICKLE_FILE, 'wb') as f:
            pickle.dump(self.record, f)
        logging.info('done update')

    def _update_record_one_block(self):
        self.record.last_processed_block += 1
        self.record.processed_txn_in_next_block = []
        with open(self.config.PICKLE_FILE, 'wb') as f:
            pickle.dump(self.record, f)

    def _process_request(self, req):
        logging.info("processing request {0}".format(req.txid))

        nonce = self.eth_rpc.eth_getTransactionCount(self.config.SGX_WALLET_ADDR)

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
            logging.error('Error: {0}'.format(resp['error']))
        else:
            error_code = resp['result']['error_code']
            response_tx = resp['result']['response']
            if error_code != 0:
                logging.error('Error in tx: {0}'.format(error_code))
            self.eth_rpc.eth_sendRawTransaction(response_tx)
            self._update_record_one_request(req)

    def loop(self):
        next_block = self.config.TC_CONTRACT_BLOCK_NUM
        while True:
            next_block = max(next_block, self.record.last_processed_block + 1)

            if next_block > self.eth_rpc.eth_blockNumber():
                logging.debug("waiting for more blocks")
                time.sleep(2)
                continue

            logging.info("processing block {0}".format(next_block))

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
                            logging.error('exception: {0}'.format(e.message))
                        except Exception as e:
                            logging.error('exception:', e.message)
                        retry += 1

            self._update_record_one_block()


parser = argparse.ArgumentParser(description="Town Crier Ethereum relay")
parser.add_argument('-v', action='store_true', dest='verbose', help='Verbose')
parser.add_argument('-t', action='store_true', dest='testnet', help='Enable testnet')

args = parser.parse_args()
args.parser = parser

logging.root.setLevel('INFO')
network = TCMonitor.SIM_NET

if args.verbose:
    logging.root.setLevel('DEBUG')

if args.testnet:
    network = TCMonitor.TEST_NET

print network

monitor = TCMonitor(network)
monitor.loop()
