#!/bin/bash

source env.sh

$MIST --ipcpath $IPC_PATH \
    --node-networkid $NETWORKID \
    --node-datadir $DATADIR \
    --node-keystore $KEY_STORE \
    --loglevel $MIST_LOG_LVL
