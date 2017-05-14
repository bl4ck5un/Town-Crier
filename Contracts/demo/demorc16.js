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

var encryptedApiKey = [
    '0xf68d2a32cf17b1312c6db3f236a38c94', 
    '0x4c9f92f6ec1e2a20a1413d0ac1b867a3']

var buyerSteamId = String(32884794);

loadScript("compiledContract.js")

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

function setupTC() {
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

function createApp(tc) {
    unlockAccounts();
    var tradeContract = App.new(
        tc, {
            from: eth.coinbase,
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

function Request(contract, type, requestData) {
    unlockAccounts();
    contract.request.sendTransaction(type, requestData, {
        from: buyerAddr,
        value: 3e15,
        gas: gasCnt
    });
    mineBlocks(1);
    return "Request sent!";
}

function Cancel(contract, id) {
    unlockAccounts();
    contract.cancel.sendTransaction(id, {
        from: buyerAddr,
        gas: gasCnt
    });
    mineBlocks(1);
    return "Request Cancelled.";
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
// For privatenet:
// loadScript("demorc16.js");
// tc = setupTC();
// tc = TownCrier.at("tc address from receipt")
// app = createApp(tc.address);
// app = App.at("app address from receipt");
// setup_log(tc, app, 0);
// Request(app, 1, ['FJM273', pad(1492100100, 64)]);
// Request(app, 2, ['f68d2a32cf17b1312c6db3f236a38c94', '4c9f92f6ec1e2a20a1413d0ac1b867a3', '32884794', pad(1456380265, 64), pad(1, 64), 'Portal']);
// Request(app, 3, ['GOOG', pad(1262390400,64)]);
// Request(app, 4, ['1ZE331480394808282']);
// Request(app, 5, ['bitcoin']);

// For testnet:
// loadScript("demorc16.js");
// tc = TownCrier.at("0xc3847c4de90b83cb3f6b1e004c9e6345e0b9fc27")
// app = createApp(tc.address);
// app = App.at("app address from receipt");
// setup_log(tc, app, 0);
// Request(app, 1, ['FJM273', pad(1492100100, 64)]);
// Request(app, 2, ['f68d2a32cf17b1312c6db3f236a38c94', '4c9f92f6ec1e2a20a1413d0ac1b867a3', '32884794', pad(1456380265, 64), pad(1, 64), 'Portal']);
// Request(app, 3, ['GOOG', pad(1262390400,64)]);
// Request(app, 4, ['1ZE331480394808282']);
// Request(app, 5, ['bitcoin']);

