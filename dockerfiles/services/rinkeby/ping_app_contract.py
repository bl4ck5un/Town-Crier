from web3 import Web3
import logging

logging.basicConfig(format='%(asctime)s %(levelname)-8s [%(filename)s:%(lineno)-4d] %(message)s',
                    datefmt='%d-%m-%Y:%H:%M:%S',
                    level=logging.INFO)


class TCTestingContract:
    def __init__(self, w3_instance, app_contract_address, app_contract_abi):
        self.w3 = w3_instance

        self.logger = logging.getLogger(__name__)
        self.logger.info("Current block height: %d", self.w3.eth.blockNumber)

        self.app_contract = self.w3.eth.contract(address=app_contract_address, abi=app_contract_abi)

        # this is a Rinkeby private key
        # it DOSE NOT hold real Ether
        self.test_wallet_address = "0x0482C4DAA16E43bD23B9919CcfA95AEE4a7d547d"
        self.test_wallet_privatekey = "0x5498462161d13e0bb3a86d8fafbd76759542c648fa9c7ea50153518f3ec5d7fd"

    def test_requests_bitcoin_fees(self):
        # requestType = 2
        # must pay a TC fee of at least 0.0025 Ether (here we're paying 0.003 for extra safety)
        txn = self.app_contract.functions.request(2, []).buildTransaction({
            'chainId': 4,  # Rinkeby = 1
            'gasPrice': Web3.toWei('5', 'gwei'),  # set to a high value to make it fast
            'nonce': self.w3.eth.getTransactionCount(self.test_wallet_address),
            'gas': 300000,  # a rough estimate
            'value': Web3.toWei(0.003, 'Ether'),  # TC usage fee
        })

        import pprint
        pprint.pprint(txn)

        signed_txn = self.w3.eth.account.sign_transaction(txn, private_key=self.test_wallet_privatekey)

        tx_hash = self.w3.eth.sendRawTransaction(signed_txn.rawTransaction)

        receipt = self.w3.eth.waitForTransactionReceipt(tx_hash)
        print("Transaction receipt mined: \n")

        pprint.pprint(dict(receipt))
        print("Was transaction successful?", receipt['status'])
        self.logger.info("Found tx at https://rinkeby.etherscan.io/tx/%s", tx_hash.hex())


if __name__ == '__main__':
    # use a private infura node for fast prototype
    # any provider should work
    from os import environ

    if environ.get('WEB3_INFURA_PROJECT_ID') is None:
        raise Exception("please set WEB3_INFURA_PROJECT_ID")
    else:
        infura_credential = environ.get('WEB3_INFURA_PROJECT_ID')
    w3_instance = Web3(Web3.HTTPProvider("https://rinkeby.infura.io/v3/{}".format(infura_credential)))

    # ABI is copied from https://rinkeby.etherscan.io/address/0x20e63d9683a75ef73e6174298354f8b016878de3#code
    testing_contract = TCTestingContract(w3_instance,
                                         Web3.toChecksumAddress('0x20e63d9683a75ef73e6174298354f8b016878de3'),
                                         r"""[{"constant":false,"inputs":[{"name":"requestType","type":"uint8"},{"name":"requestData","type":"bytes32[]"}],"name":"request","outputs":[],"payable":true,"stateMutability":"payable","type":"function"},{"constant":false,"inputs":[{"name":"requestId","type":"uint64"}],"name":"cancel","outputs":[],"payable":false,"stateMutability":"nonpayable","type":"function"},{"constant":true,"inputs":[],"name":"TC_CONTRACT","outputs":[{"name":"","type":"address"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":false,"inputs":[{"name":"requestId","type":"uint64"},{"name":"error","type":"uint64"},{"name":"respData","type":"bytes32"}],"name":"response","outputs":[],"payable":false,"stateMutability":"nonpayable","type":"function"},{"inputs":[{"name":"tcCont","type":"address"}],"payable":false,"stateMutability":"nonpayable","type":"constructor"},{"payable":true,"stateMutability":"payable","type":"fallback"},{"anonymous":false,"inputs":[{"indexed":false,"name":"requestId","type":"int64"},{"indexed":false,"name":"requester","type":"address"},{"indexed":false,"name":"dataLength","type":"uint256"},{"indexed":false,"name":"data","type":"bytes32[]"}],"name":"Request","type":"event"},{"anonymous":false,"inputs":[{"indexed":false,"name":"requestId","type":"int64"},{"indexed":false,"name":"requester","type":"address"},{"indexed":false,"name":"error","type":"uint64"},{"indexed":false,"name":"data","type":"uint256"}],"name":"Response","type":"event"},{"anonymous":false,"inputs":[{"indexed":false,"name":"requestId","type":"uint64"},{"indexed":false,"name":"requester","type":"address"},{"indexed":false,"name":"success","type":"bool"}],"name":"Cancel","type":"event"}]""")

    testing_contract.test_requests_bitcoin_fees()
