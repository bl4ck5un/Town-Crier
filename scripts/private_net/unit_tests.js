if (eth.accounts.length < 3)
{
    console.log(eth.accounts.length + ' accounts found. Need 3')
    exit;
}

var minerAddr = eth.accounts[0]
var sellerAddr = eth.accounts[1]
var buyerAddr = eth.accounts[2]
var sgxAddr = "0x89b44e4d3c81ede05d0f5de8d1a68f754d73d997"

var gasCnt = 3e+6
var TC_FEE = 3e+15

var TownCrier = web3.eth.contract(JSON.parse(compiledContract.contracts["TownCrier"].abi));
var App = web3.eth.contract(JSON.parse(compiledContract.contracts["Application"].abi));

function checkWork(){
    if (eth.getBlock("pending").transactions.length > 0) {
        if (eth.mining) return;
        console.log("== Pending transactions! Mining...");
        miner.start(1);
    } else {
        if (!eth.mining) return;
        miner.stop();
        console.log("== No transactions! Mining stopped.");
    }
}

function mineBlocks(num) {
    miner.start(1); miner.start(1); admin.sleepBlocks(num); miner.stop();

    return "One block mined";
}

function setup_log(tc, tradeContract, id) {
    if (id == 0) {
        tc.RequestInfo(function(e,r) {
            if (!e) { console.log('TC RequestInfo: ' + JSON.stringify(r.args)); }
            else {console.log(e)}
        });

        tc.DeliverInfo(function(e,r) {
            if (!e) { console.log('TC ResponseInfo: ' + JSON.stringify(r.args)); }
            else {console.log(e)}
        });

        tc.Cancel(function(e,r) {
            if (!e) { console.log('TC Cancel: ' + JSON.stringify(r.args)); }
            else {console.log(e)}
        });
    }

    if (id == 1) {
        tradeContract.Insure(function(e,r) {
            if (!e) { console.log('App Insure: ' + JSON.stringify(r.args)); }
            else { console.log(e)}
        });
    }

    tradeContract.Request(function(e,r) {
		if (!e) { console.log('App Request: ' + JSON.stringify(r.args)); }
		else {console.log(e)}
	});

    tradeContract.Response(function(e,r) {
		if (!e) { console.log('App Response: ' + JSON.stringify(r.args)); }
		else {console.log(e)}
	});

    tradeContract.Cancel(function(e,r) {
		if (!e) { console.log('App Cancel: ' + JSON.stringify(r.args)); }
		else {console.log(e)}
	});
}

function createTC(tci) {
    unlockAccounts();
    var tc = TownCrier.new({
        from: minerAddr,
        data: "0x" + compiledContract.contracts["TownCrier"].bin,
        gas: gasCnt}, function(e, c) {
            if (!e){
                if (c.address) {
                    console.log('Town Crier created at: ' + c.address)
                }
            }
            else {console.log('Failed to create Town Crier contract: ' + e)}
        });
    mineBlocks(1);
    return tc;
}

function upgrade(contract, newAddr) {
    unlockAccounts();
    contract.upgrade.sendTransaction(newAddr, {
        from: minerAddr,
        gas: gasCnt
    });
    mineBlocks(1);
    return "TC upgraded!";
}


function reset(contract, price, minGas, cancellationGas) {
    unlockAccounts();
    contract.reset.sendTransaction(price, minGas, cancellationGas, {
        from: minerAddr,
        gas: gasCnt
    });
    mineBlocks(1);
    return "TC reset!";
}

function suspend(contract) {
    unlockAccounts();
    contract.suspend.sendTransaction({
        from: minerAddr,
        gas: gasCnt
    });
    mineBlocks(1);
    return "TC suspended!";
}

function restart(contract) {
    unlockAccounts();
    contract.restart.sendTransaction({
        from: minerAddr,
        gas: gasCnt
    });
    mineBlocks(1);
    return "TC restarted!";
}

function withdraw(contract) {
    unlockAccounts();
    contract.withdraw.sendTransaction({
        from: minerAddr,
        gas: gasCnt
    });
    mineBlocks(1);
    return "TC ether withdrew!";
}

function createApp(tc) {
    unlockAccounts();
    var tradeContract = App.new(
        tc, {
            from: sellerAddr,
            data: "0x" + compiledContract.contracts["Application"].bin,
            gas:gasCnt},
            function(e, c) {
                if (!e) {
                    if (c.address) {
                        console.log('Application created at: ' + c.address)
                    }
                } else {
                    console.log('Failed to create Application contract: ' + e)}
                });
    mineBlocks(1);
    return tradeContract;
}

function request(contract, type, requestData) {
    unlockAccounts();
    contract.request.sendTransaction(type, requestData, {
        from: buyerAddr,
        value: 3e15,
        gas: gasCnt
    });
    mineBlocks(1);
    return "Request sent!";
}

function cancel(contract, id) {
    unlockAccounts();
    contract.cancel.sendTransaction(id, {
        from: buyerAddr,
        gas: gasCnt
    });
    mineBlocks(1);
    return "Request Cancelled.";
}

function setup_log(tc, tradeContract, id) {
if (id == 0) {
    tc.Upgrade(function(e,r) {
        if (!e) {console.log('TC Upgrade: ' + JSON.stringify(r.args)); }
        else {console.log(e)}
    });

    tc.Reset(function(e,r) {
        if (!e) {console.log('TC Reset: ' + JSON.stringify(r.args));}
        else {console.log(e)}
    });

    tc.RequestInfo(function(e,r) {
		if (!e) { console.log('TC RequestInfo: ' + JSON.stringify(r.args)); }
		else {console.log(e)}
	});

	tc.DeliverInfo(function(e,r) {
		if (!e) { console.log('TC ResponseInfo: ' + JSON.stringify(r.args)); }
		else {console.log(e)}
	});

	tc.Cancel(function(e,r) {
		if (!e) { console.log('TC Cancel: ' + JSON.stringify(r.args)); }
		else {console.log(e)}
	});
    }

    if (id == 1) {
        tradeContract.Insure(function(e,r) {
            if (!e) { console.log('App Insure: ' + JSON.stringify(r.args)); }
            else { console.log(e)}
        });
    }

    tradeContract.Request(function(e,r) {
		if (!e) { console.log('App Request: ' + JSON.stringify(r.args)); }
		else {console.log(e)}
	});

    tradeContract.Response(function(e,r) {
		if (!e) { console.log('App Response: ' + JSON.stringify(r.args)); }
		else {console.log(e)}
	});

    tradeContract.Cancel(function(e,r) {
		if (!e) { console.log('App Cancel: ' + JSON.stringify(r.args)); }
		else {console.log(e)}
	});
}

function watch_events(contract) {
    var his = contract.allEvents({fromBlock: 0, toBlock: 'latest'});
    var events;
    his.get(function(error, result) {
        if (!error) {
            console.log(result.length);
            for (var i = 0; i < result.length; ++i) {
                console.log(i + " : " + result[i].event);
            }
            events = result;
        } else {
            console.log("error");
            events = "error";
        }
    });
    return events;
}

function unlockAccounts() {
    for (var i = 0; i < eth.accounts.length; ++i) {
        personal.unlockAccount(eth.accounts[i], '123123');
    }
}

function pad(n, width) {
    m = n.toString(16);
    return '0x' + new Array(width - m.length + 1).join('0') + m;
}

/* =========== The following should be run line-by-line as a demo =========== */
// loadScript("test_script.js");
// tc = createTC();
// (or if there is already a tc) tc = TownCrier.at("tc address from receipt")
// app = createApp(tc.address);
// (or if there is already an app) app = App.at("app address from receipt");
// setup_log(tc, app, 0);
// watch_events(tc);
// watch_events(app);
// request(app, 1, ['FJM273', pad(1492100100, 64)]);
// request(app, 2, ['f68d2a32cf17b1312c6db3f236a38c94', '4c9f92f6ec1e2a20a1413d0ac1b867a3', '32884794', pad(1456380265, 64), pad(1, 64), 'Portal']);
// request(app, 3, ['GOOG', pad(1262390400,64)]);
// request(app, 4, ['1ZE331480394808282']);
// request(app, 5, ['bitcoin']);
