#!/bin/bash
source env.sh

$MIST --testnet \
      --datadir $TESTNET_DATADIR \
      --ipcpath $IPC_PATH 
