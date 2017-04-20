#!/bin/bash -e

CPPLINT=./utils/cpplint.py

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
    xargs $CPPLINT
