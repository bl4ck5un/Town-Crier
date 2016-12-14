MIST=~/mist/Mist
IPC=~/.ethereum/geth.ipc
datadir=
#datadir=./chain0
#networkid="93819023"

$MIST --testnet \
      --ipcpath $IPC 
#    --node-networkid $networkid \
#   --node-datadir $datadir
