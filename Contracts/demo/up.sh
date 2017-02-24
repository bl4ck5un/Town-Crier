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

localip=127.0.0.1

$GETH --datadir=$datadir init genesis.json
$GETH --datadir=$datadir \
  --identity="00" \
  --networkid="93819023" \
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
