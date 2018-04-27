var tcDevWallet = "0x8f108aab17e3b90f6855a73349511f5944b7e146"
var sgxAddr = "0x3A8DE03F19C7C4C139B171978F87BFAC9FFE99C0"

var tcContractAddr = "0x9eC1874FF1deF6E178126f7069487c2e9e93D0f9";
var appContractAddr = "0xdE34AfC49b8A15bEb76A6E942bD687143C1574B6";

var TownCrier = web3.eth.contract(JSON.parse(compiledContract.contracts["TownCrier"].abi));
var App = web3.eth.contract(JSON.parse(compiledContract.contracts["Application"].abi));

var gasCnt = 3e+6
var TC_FEE = 3e+15

function createTC() {
    unlockAccounts();
    var tc = TownCrier.new({
        from: tcDevWallet,
        data: "0x" + compiledContract.contracts["TownCrier"].bin,
        gas: gasCnt
    }, function (e, c) {
        if (!e) {
            if (c.address) {
                console.log('Town Crier created at: ' + c.address)
            }
        }
        else {
            console.log('Failed to create Town Crier contract: ' + e)
        }
    });
    return tc;
}

function createApp(tc) {
    unlockAccounts();
    var tradeContract = App.new(
        tc, {
            from: tcDevWallet,
            data: "0x" + compiledContract.contracts["Application"].bin,
            gas: gasCnt
        },
        function (e, c) {
            if (!e) {
                if (c.address) {
                    console.log('Application created at: ' + c.address)
                }
            } else {
                console.log('Failed to create Application contract: ' + e)
            }
        });
    return tradeContract;
}

function request(contract, type, requestData) {
    unlockAccounts();
    contract.request.sendTransaction(type, requestData, {
        from: tcDevWallet,
        value: 3e15,
        gas: gasCnt
    });
    return "Request sent!";
}

function cancel(contract, id) {
    unlockAccounts();
    contract.cancel.sendTransaction(id, {
        from: tcDevWallet,
        gas: gasCnt
    });
    return "Request Cancelled.";
}

function setup_log(tc) {
    tc.Upgrade(function (e, r) {
        if (!e) {
            console.log('TC Upgrade: ' + JSON.stringify(r.args));
        }
        else {
            console.log(e)
        }
    });

    tc.Reset(function (e, r) {
        if (!e) {
            console.log('TC Reset: ' + JSON.stringify(r.args));
        }
        else {
            console.log(e)
        }
    });

    tc.RequestInfo(function (e, r) {
        if (!e) {
            console.log('TC RequestInfo: ' + JSON.stringify(r.args));
        }
        else {
            console.log(e)
        }
    });

    tc.DeliverInfo(function (e, r) {
        if (!e) {
            console.log('TC ResponseInfo: ' + JSON.stringify(r.args));
        }
        else {
            console.log(e)
        }
    });

    tc.Cancel(function (e, r) {
        if (!e) {
            console.log('TC Cancel: ' + JSON.stringify(r.args));
        }
        else {
            console.log(e)
        }
    });
}

function watch_events(contract) {
    var his = contract.allEvents({fromBlock: 0, toBlock: 'latest'});
    var events;
    his.get(function (error, result) {
        if (!error) {
            console.log(result.length);
            for (var i = 0; i < result.length; ++i) {
                console.log(i + " : " + result[i].event);
                console.log(JSON.stringify(result[i]));
            }
            events = "done";
        } else {
            console.log("error");
            events = "error";
        }
    });
    return events;
}

function unlockAccounts() {
    personal.unlockAccount(tcDevWallet);
}

function pad(n, width) {
    m = n.toString(16);
    return '0x' + new Array(width - m.length + 1).join('0') + m;
}

tc = TownCrier.at(tcContractAddr);
app = App.at(appContractAddr);

function help() {

    var helpMsg = "/* =========== The following should be run line-by-line as a demo =========== */ \n\
// tc = createTC(); \n\
// (or if there is already a tc) tc = TownCrier.at('tc address from receipt') \n\
// app = createApp(tc.address); \n\
// (or if there is already an app) app = App.at('app address from receipt'); \n\
// setup_log(tc); \n\
// watch_events(tc); \n\
// watch_events(app); \n\
// request(app, 1, ['FJM273', pad(1492100100, 64)]); \n\
// request(app, 2, ['f68d2a32cf17b1312c6db3f236a38c94', '4c9f92f6ec1e2a20a1413d0ac1b867a3', '32884794', pad(1456380265, 64), pad(1, 64), 'Portal']); \n\
// request(app, 3, ['GOOG', pad(1262390400,64)]); \n\
// request(app, 4, ['1ZE331480394808282']); \n\
// request(app, 5, ['bitcoin']);";


    console.log(helpMsg);
}

help();
