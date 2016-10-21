#!/bin/bash

GETH=geth
IPC=~/.ethereum/geth.ipc
datadir=chain0
mkdir -p $datadir

read -p "Create new accounts? [y/N] " yn
case $yn in
	[Yy]*) $GETH --datadir $datadir account new;; 
	*    ) echo "No account created";;
esac

$GETH --datadir=$datadir removedb

coinbase=$($GETH --datadir=$datadir account list | head -n 1 | sed 's/.*{\([a-z0-9]*\)}.*/\1/g') 

localip=127.0.0.1

$GETH --datadir=$datadir init genesis.json
$GETH --datadir=$datadir \
  --identity="00" \
  --networkid="93819023" \
  --etherbase="$coinbase" \
  --port=30000 \
  --rpc \
  --rpcaddr=$localip\
  --rpcport=8200\
  --rpccorsdomain='*' \
  --verbosity="3" \
  --nodiscover \
  --rpcapi="db,eth,ne,web3,personal" \
  --ipcpath=$IPC \
  console 
