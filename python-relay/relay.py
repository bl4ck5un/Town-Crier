"""
Town Crier front end.

Usage:
  relay.py [options]

Options:
  -h --help                 Show this screen.
  --version                 Show version.
  --catchup                 Catch up until the current block first.
  --dryrun                  Process one request then exit.
"""

import logging

import grpc
from web3 import Web3

import tc_pb2_grpc

logging.basicConfig(format='%(asctime)s %(levelname)-8s [%(filename)s:%(lineno)-4d] %(message)s',
                    datefmt='%d-%m-%Y:%H:%M:%S',
                    level=logging.INFO)


class ConfigRinkeby:
    # THIS KEY is for DEBUG/EVALUATION/TESTING only
    # DO NOT (!!!) use it in production. Assume NO security.
    SGX_WALLET_ADDR = Web3.toChecksumAddress("0x89b44e4d3c81ede05d0f5de8d1a68f754d73d997")

    # JSON RPC endpoint
    TC_CORE_RPC_URL = "localhost:8123"

    # https://rinkeby.etherscan.io/address/0xc41c9c6be928f3abde1c3b327a70c5a5abb35c5f
    TC_CONTRACT_ADDR = Web3.toChecksumAddress("0xc41c9c6be928f3abde1c3b327a70c5a5abb35c5f")
    TC_ABI = r"""[{"constant":false,"inputs":[{"name":"newAddr","type":"address"}],"name":"upgrade","outputs":[],"payable":false,"stateMutability":"nonpayable","type":"function"},{"constant":false,"inputs":[],"name":"restart","outputs":[],"payable":false,"stateMutability":"nonpayable","type":"function"},{"constant":true,"inputs":[],"name":"newVersion","outputs":[{"name":"","type":"int256"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":true,"inputs":[],"name":"SGX_ADDRESS","outputs":[{"name":"","type":"address"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":true,"inputs":[],"name":"DELIVERED_FEE_FLAG","outputs":[{"name":"","type":"uint256"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":false,"inputs":[],"name":"withdraw","outputs":[],"payable":false,"stateMutability":"nonpayable","type":"function"},{"constant":true,"inputs":[],"name":"killswitch","outputs":[{"name":"","type":"bool"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":false,"inputs":[{"name":"requestId","type":"uint64"},{"name":"paramsHash","type":"bytes32"},{"name":"error","type":"uint64"},{"name":"respData","type":"bytes32"}],"name":"deliver","outputs":[],"payable":false,"stateMutability":"nonpayable","type":"function"},{"constant":false,"inputs":[{"name":"requestId","type":"uint64"}],"name":"cancel","outputs":[{"name":"","type":"int256"}],"payable":false,"stateMutability":"nonpayable","type":"function"},{"constant":false,"inputs":[{"name":"requestType","type":"uint8"},{"name":"callbackAddr","type":"address"},{"name":"callbackFID","type":"bytes4"},{"name":"timestamp","type":"uint256"},{"name":"requestData","type":"bytes32[]"}],"name":"request","outputs":[{"name":"","type":"int256"}],"payable":true,"stateMutability":"payable","type":"function"},{"constant":true,"inputs":[],"name":"FAIL_FLAG","outputs":[{"name":"","type":"int256"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":true,"inputs":[],"name":"requestCnt","outputs":[{"name":"","type":"uint64"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":true,"inputs":[],"name":"MIN_FEE","outputs":[{"name":"","type":"uint256"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":true,"inputs":[],"name":"CANCELLATION_FEE","outputs":[{"name":"","type":"uint256"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":true,"inputs":[{"name":"","type":"uint256"}],"name":"requests","outputs":[{"name":"requester","type":"address"},{"name":"fee","type":"uint256"},{"name":"callbackAddr","type":"address"},{"name":"callbackFID","type":"bytes4"},{"name":"paramsHash","type":"bytes32"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":true,"inputs":[],"name":"GAS_PRICE","outputs":[{"name":"","type":"uint256"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":true,"inputs":[],"name":"unrespondedCnt","outputs":[{"name":"","type":"uint64"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":false,"inputs":[{"name":"price","type":"uint256"},{"name":"minGas","type":"uint256"},{"name":"cancellationGas","type":"uint256"}],"name":"reset","outputs":[],"payable":false,"stateMutability":"nonpayable","type":"function"},{"constant":true,"inputs":[],"name":"CANCELLED_FEE_FLAG","outputs":[{"name":"","type":"uint256"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":true,"inputs":[],"name":"SUCCESS_FLAG","outputs":[{"name":"","type":"int256"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":false,"inputs":[],"name":"suspend","outputs":[],"payable":false,"stateMutability":"nonpayable","type":"function"},{"constant":true,"inputs":[],"name":"externalCallFlag","outputs":[{"name":"","type":"bool"}],"payable":false,"stateMutability":"view","type":"function"},{"inputs":[],"payable":false,"stateMutability":"nonpayable","type":"constructor"},{"payable":false,"stateMutability":"nonpayable","type":"fallback"},{"anonymous":false,"inputs":[{"indexed":false,"name":"newAddr","type":"address"}],"name":"Upgrade","type":"event"},{"anonymous":false,"inputs":[{"indexed":false,"name":"gas_price","type":"uint256"},{"indexed":false,"name":"min_fee","type":"uint256"},{"indexed":false,"name":"cancellation_fee","type":"uint256"}],"name":"Reset","type":"event"},{"anonymous":false,"inputs":[{"indexed":false,"name":"id","type":"uint64"},{"indexed":false,"name":"requestType","type":"uint8"},{"indexed":false,"name":"requester","type":"address"},{"indexed":false,"name":"fee","type":"uint256"},{"indexed":false,"name":"callbackAddr","type":"address"},{"indexed":false,"name":"paramsHash","type":"bytes32"},{"indexed":false,"name":"timestamp","type":"uint256"},{"indexed":false,"name":"requestData","type":"bytes32[]"}],"name":"RequestInfo","type":"event"},{"anonymous":false,"inputs":[{"indexed":false,"name":"requestId","type":"uint64"},{"indexed":false,"name":"fee","type":"uint256"},{"indexed":false,"name":"gasPrice","type":"uint256"},{"indexed":false,"name":"gasLeft","type":"uint256"},{"indexed":false,"name":"callbackGas","type":"uint256"},{"indexed":false,"name":"paramsHash","type":"bytes32"},{"indexed":false,"name":"error","type":"uint64"},{"indexed":false,"name":"respData","type":"bytes32"}],"name":"DeliverInfo","type":"event"},{"anonymous":false,"inputs":[{"indexed":false,"name":"requestId","type":"uint64"},{"indexed":false,"name":"canceller","type":"address"},{"indexed":false,"name":"requester","type":"address"},{"indexed":false,"name":"fee","type":"uint256"},{"indexed":false,"name":"flag","type":"int256"}],"name":"Cancel","type":"event"}]"""
    # topic can be found here: https://rinkeby.etherscan.io/address/0xc41c9c6be928f3abde1c3b327a70c5a5abb35c5f#events
    TC_REQUEST_TOPIC = "0x295780EA261767C398D062898E5648587D7B8CA371FFD203BE8B4F9A43454FFA"

    # w3 provider
    def __init__(self):
        from os import environ
        if environ.get('WEB3_INFURA_PROJECT_ID') is None:
            raise Exception("please set WEB3_INFURA_PROJECT_ID")
        else:
            infura_credential = environ.get('WEB3_INFURA_PROJECT_ID')
            self.w3 = Web3(Web3.WebsocketProvider("wss://rinkeby.infura.io/ws/v3/{}".format(infura_credential)))


class TCRelay:
    def __init__(self, config: ConfigRinkeby):
        self.config = config
        self.w3 = config.w3

        self.logger = logging.getLogger(__name__)

        if not self.w3.isConnected():
            self.logger.info('cannot connect to web3')
            import sys
            sys.exit(1)
        else:
            self.logger.info('connected to {0}'.format(self.w3.clientVersion))
            self.logger.info("Current block height: %d", self.w3.eth.blockNumber)

        self.tc_contract = self.w3.eth.contract(address=config.TC_CONTRACT_ADDR, abi=config.TC_ABI)

        # create a grpc client
        channel = grpc.insecure_channel(self.config.TC_CORE_RPC_URL)
        self.stub = tc_pb2_grpc.towncrierStub(channel)

    def handle_request_event(self, log_entry):
        nonce = self.w3.eth.getTransactionCount(self.config.SGX_WALLET_ADDR)

        request = log_entry['args']
        import tc_pb2
        response = self.stub.process(tc_pb2.Request(id=request['id'],
                                                    type=request['requestType'],
                                                    data=b''.join(request['requestData']),  # concat an array of bytes
                                                    nonce=nonce))
        if response.error_code != 0:
            self.logger.error("Enclave returned error %d", response.error_code)

        self.logger.info('response from enclave: %s', response.response_tx.hex())
        return response.response_tx

    def catchup(self, upto_block):
        """
        process all unprocessed transactions
        """

        self.logger.info("catching up to block %d", upto_block)
        # check if there are unanswered requests
        unrespondedCnt = self.tc_contract.functions.unrespondedCnt().call()

        if unrespondedCnt == 0:
            self.logger.info("You've caught up!")
            return

        self.logger.info("There are %d unresponded requests", unrespondedCnt)

        # get all delivered responses -- we don't want to answer them again
        filter_all_responses = self.tc_contract.events.DeliverInfo.createFilter(fromBlock=0, toBlock=upto_block)
        processed_request_ids = set()
        for entry in filter_all_responses.get_all_entries():
            processed_request_ids.add(entry['args']['requestId'])

        # get all requests
        filter_all_requests = self.tc_contract.events.RequestInfo.createFilter(fromBlock=0, toBlock=upto_block)
        for entry in filter_all_requests.get_all_entries():
            id = entry['args']['id']

            # skip processed ones
            if id in processed_request_ids:
                self.logger.info('skipping request %d', id)
                continue

            response = self.handle_request_event(entry)
            self.send_response_check(response)

    def send_response_check(self, response):
        tx_hash = self.w3.eth.sendRawTransaction(response)
        receipt = self.w3.eth.waitForTransactionReceipt(tx_hash)

        self.logger.info("Transaction receipt mined. See https://rinkeby.etherscan.io/tx/%s", tx_hash.hex())

    def wait_for_requests(self, poll_interval=5):
        import time
        # get all requests since last call
        filter_all_requests = self.tc_contract.events.RequestInfo.createFilter(fromBlock='latest')
        self.logger.info("filter for RequestInfo created. ID %s.", filter_all_requests.filter_id)

        while True:
            try:
                self.logger.info('getting all requests in the latest block')
                for entry in filter_all_requests.get_new_entries():
                    response = self.handle_request_event(entry)
                    self.send_response_check(response)
            except grpc.RpcError as rpc_error:
                self.logger.error('RPC failure: %s', rpc_error)
            except ValueError as e:
                try:
                    if e.args[0]['message'] == 'filter not found':
                        # if e is caused by that the filter we're using has been retired, we create a new one.
                        self.logger.warning("filter %s not found. Probably retired.", filter_all_requests.filter_id)
                        filter_all_requests = self.tc_contract.events.RequestInfo.createFilter(fromBlock='latest')
                        self.logger.info("filter for RequestInfo created. ID %s.", filter_all_requests.filter_id)
                except Exception:
                    # if the except e is not due to "filter not found", pass it through (i.e., re-raise)
                    self.logger.error('exception: %s', e)
                    raise e

            except Exception as e:
                self.logger.error('exception: %s', e)

            # sleep for a few seconds (5 seconds by default)
            time.sleep(poll_interval)

    def dry_run(self):
        # get all requests
        self.logger.info('in dryrun mode')
        filter_all_requests = self.tc_contract.events.RequestInfo.createFilter(fromBlock=0)
        for entry in filter_all_requests.get_all_entries():
            try:
                self.handle_request_event(entry)
            except grpc.RpcError as rpc_error:
                self.logger.error('RPC failure: %s', rpc_error)
            # return after processing one request
            return


if __name__ == '__main__':
    from docopt import docopt
    import sys

    args = docopt(__doc__)

    config = ConfigRinkeby()
    relay = TCRelay(config)

    if args['--catchup']:
        curr_block = relay.w3.eth.blockNumber
        relay.catchup(curr_block)

    if args['--dryrun']:
        relay.dry_run()
        sys.exit(0)

    relay.wait_for_requests()
