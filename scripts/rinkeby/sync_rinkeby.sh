#!/bin/bash

docker run \
	--name 'geth-rink' \
	-d \
	--rm \
	-v /data/ethereum/rinkeby:/root \
	-v $PWD/rinkeby.json:/genesis.json \
	-v $PWD/config_rinkeby.toml:/config.toml \
        -p 8545:8545 \
        -p 30303:30303 \
	ethereum/client-go:v1.6.0 --config /config.toml

