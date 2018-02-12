#!/bin/bash

docker run -it \
    -p 8200:8200 \
    -p 8545:8545 \
    -v ~/data/geth-tc/privnet:/root/.ethereum \
    -v "$(pwd)/../Contracts/demo:/tc" \
    ethereum/client-go \
    --identity="00" \
    --networkid="26282927" \
    --ipcdisable \
    --rpc \
    --rpcaddr="0.0.0.0" \
    --rpcport="8200" \
    --rpcapi="db,eth,ne,web3,personal,crypto-js" \
    --verbosity=3 \
    --nodiscover \
    --keystore="/tc/wallet_privnet" \
    $@
