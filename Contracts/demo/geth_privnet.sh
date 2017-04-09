#!/bin/bash
source env.sh

$GETH --datadir=$DATADIR \
  --verbosity=$VERBOSITY \
  --keystore=$KEY_STORE \
  init genesis.json

$GETH --datadir=$DATADIR \
  --identity="00" \
  --networkid=$NETWORKID \
  --port=$LISTEN_PORT \
  --rpc \
  --rpcaddr=$RPC_ADDR \
  --rpcport=$RPC_PORT \
  --rpccorsdomain='*' \
  --verbosity=$VERBOSITY \
  --nodiscover \
  --rpcapi="db,eth,ne,web3,personal" \
  --ipcpath=$IPC_PATH \
  --keystore=$KEY_STORE \
  $@
