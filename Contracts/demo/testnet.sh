geth --testnet \
     --ipcpath ~/.ethereum/geth.ipc \
     --datadir ~/.ethereum \
     --rpcapi "db,eth,ne,web3,personal,crypto-js" \
     --port 30000 \
     --rpc \
     --rpcaddr 127.0.0.1\
     --rpcport 8200\
     --rpccorsdomain '*' \
    console
