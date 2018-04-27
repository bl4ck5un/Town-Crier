#!/usr/bin/env bash

CONTRACT_DIR=$(cd "../../contracts" && pwd)
NETWORK=privatenet

while getopts ":n:" opt; do
    case ${opt} in
        n)
            NETWORK=$OPTARG
            ;;
        \?)
            echo "Invalid Option: -$OPTARG";
            exit 1
            ;;
    esac
done
shift $((OPTIND -1))

if [[ ! ${NETWORK} == "mainnet" ]]; then
    patch ${CONTRACT_DIR}/TownCrier.sol ${CONTRACT_DIR}/patch-${NETWORK}.diff -o tc.sol || exit 1
else
    cp ${CONTRACT_DIR}/TownCrier.sol tc.sol
fi

cat <<EOF > .compiledContract.js
var compiledContract = $(cat tc.sol ${CONTRACT_DIR}/Application.sol \
        | docker run --rm -i ethereum/solc:stable --optimize --combined-json abi,bin,interface  \
        | sed 's/<stdin>\://g');
EOF

rm -rf tc.sol || exit 1