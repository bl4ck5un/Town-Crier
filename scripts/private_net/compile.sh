#!/usr/bin/env bash

cat <<EOF > .compiledContract.js
var compiledContract = $(cat ../../contracts/TownCrier.sol \
        ../../contracts/Application.sol \
        | docker run --rm -i ethereum/solc:stable --optimize --combined-json abi,bin,interface  \
        | sed 's/<stdin>\://g');
EOF