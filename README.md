# Town Crier: An Authenticated Data Feed For Smart Contracts

Smart contracts systems today lack trustworthy sources of data, as currently
deployed oracles provide only weak provenance and confidentiality guarantees.
The Town Crier (TC) system leverages trusted hardware (Intel SGX) to provide a
strong guarantee that data comes from an existing, trustworthy source. It also
provides confidentiality, enabling smart contracts to support confidential
queries and manage user credentials.

Please read on for installation instructions. If you want to learn how Town Crier works,
you can read our [CCS'16 paper](https://www.cs.cornell.edu/~fanz/files/pubs/tc-ccs16-final.pdf).

[![Build Status](https://travis-ci.org/bl4ck5un/Town-Crier.svg?branch=master)](https://travis-ci.org/bl4ck5un/Town-Crier)

# Get Started

## Using Docker (recommended)

    docker pull bl4ck5un/towncrier:latest
    git clone https://github.com/bl4ck5un/Town-Crier
    cd Town-Crier

Alternatively, you can also use `docker run` non-interactively, e.g.:

```shell
# build gtest
docker run -v $(pwd):/tc -w /tc bl4ck5un/towncrier sh -c "scripts/build_gtest.sh"

# make everything
docker run -v $(pwd):/tc -w /tc/src bl4ck5un/towncrier sh -c "mkdir -p build; cd build; cmake ..; make -j; make tests"

# run tests
docker run -v $(pwd):/tc -w /tc/src/build bl4ck5un/towncrier sh -c "./tests"

See `.travis.yml` for an example of how Docker is used to build and test Town
Crier.


Launch TC
---------

A configuration is needed to run tc. We need to specify RPChost in this configuration.

```
cd build
echo "[RPC]" >> config
echo "RPChost = http://localhost:8200" >> config
./tc config
```

Status Server
-------------

```
curl -d '{"id": 123, "jsonrpc": "2.0", "method": "attest"}' server.town-crier.org:8123
```

# LICENSE

The permission granted herein is solely for the purpose of compiling the TownCrier source code.
See the LICENSE file for details.
