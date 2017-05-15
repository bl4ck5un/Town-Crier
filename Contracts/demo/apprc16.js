var addr = eth.accounts[0]

var gasCnt = 3e+6
var TC_FEE = 3e+15

loadScript("compiledContract.js")

var App = web3.eth.contract(JSON.parse(compiledContract.contracts["Application"].abi));

function createInstance(contract, address) {
    return contract.at(address);
}

function Request(contract, type, requestData) {
    unlockAccounts();
    contract.request.sendTransaction(type, requestData, {
        from: addr,
        value: 3e15,
        gas: gasCnt
    });
    return "Request sent!";
}

function Cancel(contract, id) {
    unlockAccounts();
    contract.cancel.sendTransaction(id, {
        from: addr,
        gas: gasCnt
    });
    return "Request Cancelled.";
}

function watchEvents(tradeContract) {
    tradeContract.Request(function(e,r) { 
		if (!e) { console.log('App Request: ' + JSON.stringify(r.args)); } 
		else {console.log(e)}
	});
	
    tradeContract.Response(function(e,r) { 
		if (!e) { console.log('App Response: ' + JSON.stringify(r.args)); } 
		else {console.log(e)}
	});
}


function unlockAccounts() {
    for (var i = 0; i < eth.accounts.length; ++i) {
        personal.unlockAccount(eth.accounts[i]);
    }
}

function pad(n, width) {
    m = n.toString(16);
    return '0x' + new Array(width - m.length + 1).join('0') + m;
}

app = createInstance(App, "0x0384dd9bdbd868b128e15c163e02b760deb64dd4");
watchEvents(app);

/* =========== The following should be run line-by-line as a demo =========== */
// loadScript("demorc16.js");
// Request(app, 1, ['FJM273', pad(1492100100, 64)]);
// Request(app, 2, ['f68d2a32cf17b1312c6db3f236a38c94', '4c9f92f6ec1e2a20a1413d0ac1b867a3', '32884794', pad(1456380265, 64), pad(1, 64), 'Portal']);
// Request(app, 3, ['GOOG', pad(1262390400,64)]);
// Request(app, 4, ['1ZE331480394808282']);
// Request(app, 5, ['bitcoin']);
