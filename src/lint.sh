#!/bin/bash -ex

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

pushd $DIR
CPPLINT=./utils/cpplint.py

OPTS=--linelength=120

if [[ ! -x $CPPLINT ]]
then
    curl https://raw.githubusercontent.com/google/styleguide/gh-pages/cpplint/cpplint.py -o $CPPLINT
    chmod u+x $CPPLINT
fi

find App -type f | grep -v Enclave_u | \
    grep -v external | \
    grep -v transaction-record |\
    grep -v abstractstatusserver.h |\
    grep -v ethrpcclient.h |\
    xargs $CPPLINT $OPTS

popd
