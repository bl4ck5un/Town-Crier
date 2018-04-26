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

    git clone https://github.com/bl4ck5un/Town-Crier
    cd Town-Crier
    ./scripts/sgx-enter.sh
    cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/tc /code
    make -j && make install

To print `MRENCLAVE`, 

    root@08119b795b16:/build# /tc/bin/tc -c /tc/conf/config-privatenet-sim -m
    # omit logging
    E1FBB750A350798ADBECAFEE8A1BFC5B229E3BC007212E92102B55CE30EA0C01
    
Alternatively, `MRENCLAVE` can be extracted manually from the `sgx_metadata` section of the enclave binary.


LICENSE
-------

The permission granted herein is solely for the purpose of compiling the TownCrier source code.
See the LICENSE file for details.
