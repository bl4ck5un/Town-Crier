#!/usr/bin/env bash

cat <<EOF > .compiledContract.js
var compiledContract = $(cat ../../Contracts/TownCrier.sol \
        ../../Contracts/Application.sol \
        | docker run --rm -i ethereum/solc:stable --optimize --combined-json abi,bin,interface  \
        | sed 's/<stdin>\://g');
EOF