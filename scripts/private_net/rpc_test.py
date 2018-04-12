#! /bin/env python

#pip install ethjsonrpc

from ethjsonrpc import EthJsonRpc

c = EthJsonRpc('127.0.0.1', 8545)
print c.web3_clientVersion()

accounts = c.eth_accounts()
print c.eth_getBalance(address=accounts[0])
