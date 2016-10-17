MIST=~/mist/Mist
IPC=~/.ethereum/geth.ipc
datadir=./chain0
networkid="93819023"

$MIST --ipcpath $IPC \
    --node-networkid $networkid \
    --node-datadir $datadir
