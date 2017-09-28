from ethereum.transactions import Transaction
from ethereum.transactions import UnsignedTransaction
import rlp
import binascii

nonce = 0
gasprice = 0x0BA43B7400
gaslimit = 0x2DC6C0
to = '0x18322346bfb90378ceaf16c72cee4496723636b9'
value = 0
data_hex = '487a6e32000000000000000000000000000000000000000000000000000000000000000112782fc26e16afcc619e7b7ce654ae7059990a5082316d8ffb2987e1e66940ca00000000000000000000000000000000000000000000000000000000000000ee0000000000000000000000000000000000000000000000000000000000000000'


def generate_one_test_case(nonce, gasprice, gaslimit, to, value, data_hex):
    data = binascii.unhexlify(data_hex)
    t = UnsignedTransaction(nonce, gasprice, gaslimit, to, value, data);
    binary = rlp.encode(t)

    format_str = \
    """
    {
    %d,
    %d,
    %d,
    "%s",
    %d,
    "%s",
    "%s",
    },
    """

    print format_str % (nonce, gasprice, gaslimit, to, value, data_hex, binascii.hexlify(binary))


import random
import os

for i in range(100):
    nonce = random.randint(1e3, 1e5)
    data_len = random.randint(50, 200)
    if data_len % 2:
        data_len += 1
    data_hex = binascii.hexlify(os.urandom(data_len))

    value = random.randint(0, 5)

    generate_one_test_case(nonce, gasprice, gaslimit, to, value, data_hex)


