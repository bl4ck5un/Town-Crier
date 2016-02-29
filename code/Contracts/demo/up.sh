#!/bin/bash

datadir=$HOME/chain0/data/00
mkdir -p $datadir

read -p "Do you need a new account? [y/N] " yn
case $yn in
	[Yy]*) geth --datadir $datadir account new;; 
	*    ) echo "No account created";;
esac

geth --datadir=$datadir removedb


coinbase=$(geth --datadir=$datadir account list | head -n 1 | sed 's/.*{\([a-z0-9]*\)}.*/\1/g') 

localip=10.0.2.15

geth --datadir=$datadir \
  --identity="00" \
  --networkid="93819023" \
  --etherbase="$coinbase" \
  --port=30000 \
  --rpc \
  --rpcaddr=$localip\
  --rpcport=8200\
  --rpccorsdomain='*' \
  --genesis=genesis.json \
  console 
