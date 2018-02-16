# Town Crier: An Authenticated Data Feed For Smart Contracts

Smart contracts systems today lack trustworthy sources of data, as currently
deployed oracles provide only weak provenance and confidentiality guarantees.
The Town Crier (TC) system leverages trusted hardware (Intel SGX) to provide a
strong guarantee that data comes from an existing, trustworthy source. It also
provides confidentiality, enabling smart contracts to support confidential
queries and manage user credentials.

Please read on for build instructions for the TC enclave.
If you want to learn how Town Crier works and how to use TC in smart contract systems,
please visit http://town-crier.org.

[![Build Status](https://travis-ci.org/bl4ck5un/Town-Crier.svg?branch=master)](https://travis-ci.org/bl4ck5un/Town-Crier)

Build with Docker
-----------------------

    git clone https://github.com/bl4ck5un/Town-Crier
    cd Town-Crier
    ./scripts/sgx-enter.sh
    cmake ../code
    make && make install
    
To print `MRENCLAVE`, 

    root@08119b795b16:/build# /tc/bin/tc -m
    # omit logging
    E1FBB750A350798ADBECAFEE8A1BFC5B229E3BC007212E92102B55CE30EA0C01
    
Alternatively, `MRENCLAVE` can be extracted manually from the `sgx_metadata` section of the enclave binary.

Launch TC
---------

TC relies on [geth](https://github.com/ethereum/go-ethereum) to communicate with Ethereum blockchain.
By default TC connects to geth via RPC at `http://localhost/8200`. The RPC entrypoint can be changed in the configuration file. After `make install`, a sample configuration is installed at `/tc/conf/config-sim-test`.

    root@3405273772d1:/build# cat /tc/conf/config-sim-test 
    [RPC]
    RPChost = http://localhost:8200

    [init]
    enclave_path = /tc/enclave/enclave.debug.so

    [daemon]
    pid_file = tc.pid

    [status]
    enabled = true
    port = 8123

    [sealed]
    sig_key = blahblah
    hybrid_key = blahblah
    
To launch TC (in the above docker where `tc` is built):

    
    

Status Server
-------------

```
curl -d '{"id": 123, "jsonrpc": "2.0", "method": "attest"}' server.town-crier.org:8123
```

LICENSE
-------

The permission granted herein is solely for the purpose of compiling the TownCrier source code.
See the LICENSE file for details.
