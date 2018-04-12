#!/bin/bash

docker run \
        --name 'geth' \
        -d \
        --rm \
        -v /data/ethereum:/root \
        -v /home/accuser/geth_testnet.toml:/geth_testnet.toml \
        -p 8200:8200 \
        -p 30303:30303 \
        ethereum/client-go:v1.6.0 --config /geth_testnet.toml
