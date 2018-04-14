Testing
=======

This document tells you how to test Town Crier on a private Ethereum net.
First of all, a version of geth (>= 1.6) should be installed.

    sudo add-apt-repository -y ppa:ethereum/ethereum
    sudo apt-get update
    sudo apt-get install ethereum solc

1. Spin up `geth` by running `geth_privnet.sh`

2. `geth` will reuse keys stored in `wallet_privnet`. To make sure your geth has picked them up, run the following command within the geth console and compare the output verbatim.

        > eth.accounts
        ["0x8097ff90810b145b61d91e98b9b83aac808f4d6c", "0x46fa35fc1a00b4d316f7147e226e8d2b6bb7ba49", "0xfbab8261601d263fb86475773deda69deb3311a0", "0x6e8d004a31fb98958b00ddcbdefcda0f75df9c08"]


    Also, verify in `genesis.json`, same accounts are used.

3. (Optional) Compile the contracts using `prepare16.sh`.

4. restart geth

5. load `test_script.js` and follow the commens at the end.

        // For privatenet:
        // loadScript("test_script.js");
        // tc = createTC();
        // (or if there is already a tc) tc = TownCrier.at("tc address from receipt")
        // app = createApp(tc.address);
        // (or if there is already an app) app = App.at("app address from receipt");
        // setup_log(tc, app, 0);
        // watch_events(tc);
        // watch_events(app);
        // Request(app, 1, ['FJM273', pad(1492100100, 64)]);
        // Request(app, 2, ['f68d2a32cf17b1312c6db3f236a38c94', '4c9f92f6ec1e2a20a1413d0ac1b867a3', '32884794', pad(1456380265, 64), pad(1, 64), 'Portal']);
        // Request(app, 3, ['GOOG', pad(1262390400,64)]);
        // Request(app, 4, ['1ZE331480394808282']);
        // Request(app, 5, ['bitcoin']);


6. change the address of TC in src/Common/Constants.h

7. rebuild tc
