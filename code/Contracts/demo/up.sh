#!/bin/bash

GETH=$HOME/dev/go-ethereum/build/bin/geth
datadir=$HOME/chain0/data/00
mkdir -p $datadir

read -p "Do you need a new account? [y/N] " yn
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
  console 
