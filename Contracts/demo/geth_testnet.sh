#!/bin/bash
source env.sh

$GETH --testnet \
      --fast \
      --datadir $TESTNET_DATADIR \
      --ipcpath $IPC_PATH \
      --rpcapi "db,eth,ne,web3,personal,crypto-js" \
      --port $LISTEN_PORT \
      --rpc \
      --rpcaddr $RPC_ADDR \
      --rpcport $RPC_PORT \
      --rpccorsdomain '*' \
      console
