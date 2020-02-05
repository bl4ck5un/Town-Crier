# Town Crier: An Authenticated Data Feed For Smart Contracts

Smart contracts systems today lack trustworthy sources of data, as currently
deployed oracles provide only weak provenance and confidentiality guarantees.
The Town Crier (TC) system leverages trusted hardware (Intel SGX) to provide a
strong guarantee that data comes from an existing, trustworthy source. It also
provides confidentiality, enabling smart contracts to support confidential
queries and manage user credentials.

[![Build Status](https://travis-ci.org/bl4ck5un/Town-Crier.svg?branch=master)](https://travis-ci.org/bl4ck5un/Town-Crier)

Please read on for build instructions for the TC enclave.
If you want to learn how Town Crier works and how to use TC in smart contract systems,
please visit [town-crier.org](http://town-crier.org).

## For Contributors

### Build instructions

The recommended way to build Town Crier is using the docker image we provide:

    git clone https://github.com/bl4ck5un/Town-Crier
    cd Town-Crier
    ./scripts/sgx-enter.sh

    # in docker
    cmake -DCMAKE_INSTALL_PREFIX=/tc /code
    make -j && make install

### Running tests

Continuing the above build instructions

    # in docker
    make tc-test
    cd src/Enclave
    ./tc-test

### Start Town Crier

The main executable is installed at `/tc/bin/tc`.
You'll need a config file to start Town Crier.
For exampel, using the dummy config we provided for testing purposes, you can do,

    # in docker
    /tc/bin/tc -c /tc/conf/config-privatenet-sim
    # omit logging
    2020-02-05 18:12:35,256 [tc.cpp:107] INFO  - Enclave 4733053960194 created
    2020-02-05 18:12:35,264 [tc.cpp:122] INFO  - using wallet address at 0x89B44E4D3C81EDE05D0F5DE8D1A68F754D73D997
    2020-02-05 18:12:35,265 [tc.cpp:123] INFO  - using hybrid pubkey: BLtIrjcmxXNzRKVLNGP+xJnLEIp9EboTe6PH0EO9bX4UmU9gRio/kVUHSbsq5UEfIrf5vueZVqRjwwitUI81V98=
    2020-02-05 18:12:35,265 [tc.cpp:138] INFO  - RPC server started at 8123



LICENSE
-------

The permission granted herein is solely for the purpose of compiling the TownCrier source code.
See the LICENSE file for details.
