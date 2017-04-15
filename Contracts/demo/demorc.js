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
var TCsource = 'pragma solidity ^0.4.9;contract TownCrier { struct Request { address requester; uint fee; address callbackAddr; bytes4 callbackFID; bytes32 paramsHash; } event RequestInfo(uint64 id, uint8 requestType, address requester, uint fee, address callbackAddr, bytes32 paramsHash, uint timestamp, bytes32[] requestData); event DeliverInfo(uint64 requestId, uint fee, uint gasPrice, uint gasLeft, uint callbackGas, bytes32 paramsHash, uint64 error, bytes32 respData); event Cancel(uint64 requestId, address canceller, address requester, uint fee, int flag); address constant SGX_ADDRESS = 0x89B44e4d3c81EDE05D0f5de8d1a68F754D73d997; uint public constant GAS_PRICE = 5 * 10**10; uint public constant MIN_FEE = 30000 * GAS_PRICE; uint public constant CANCELLATION_FEE = 25000 * GAS_PRICE; uint constant CANCELLED_FEE_FLAG = 1; uint constant DELIVERED_FEE_FLAG = 0; uint64 requestCnt; Request[2**64] requests; function () {} function TownCrier() public { requestCnt = 1; requests[0].requester = msg.sender; } function request(uint8 requestType, address callbackAddr, bytes4 callbackFID, uint timestamp, bytes32[] requestData) public payable returns (uint64) { if (msg.value < MIN_FEE) { if (!msg.sender.send(msg.value)) { throw; } return 0; } else { uint64 requestId = requestCnt; requestCnt++; bytes32 paramsHash = sha3(requestType, requestData); requests[requestId].requester = msg.sender; requests[requestId].fee = msg.value; requests[requestId].callbackAddr = callbackAddr; requests[requestId].callbackFID = callbackFID; requests[requestId].paramsHash = paramsHash; RequestInfo(requestId, requestType, msg.sender, msg.value, callbackAddr, paramsHash, timestamp, requestData); return requestId; } } function deliver(uint64 requestId, bytes32 paramsHash, uint64 error, bytes32 respData) public { if (msg.sender != SGX_ADDRESS || requestId <= 0 || requests[requestId].requester == 0 || requests[requestId].fee == DELIVERED_FEE_FLAG) { return; } uint fee = requests[requestId].fee; if (requests[requestId].paramsHash != paramsHash) { return; } else if (fee == CANCELLED_FEE_FLAG) { SGX_ADDRESS.send(CANCELLATION_FEE); requests[requestId].fee = DELIVERED_FEE_FLAG; return; } requests[requestId].fee = DELIVERED_FEE_FLAG; if (error < 2) { SGX_ADDRESS.send(fee); } else { requests[requestId].requester.send(fee); } uint callbackGas = (fee - MIN_FEE) / tx.gasprice; DeliverInfo(requestId, fee, tx.gasprice, msg.gas, callbackGas, paramsHash, error, respData); if (callbackGas > msg.gas - 5000) { callbackGas = msg.gas - 5000; } requests[requestId].callbackAddr.call.gas(callbackGas)(requests[requestId].callbackFID, requestId, error, respData); } function cancel(uint64 requestId) public returns (bool) { uint fee = requests[requestId].fee; if (requests[requestId].requester == msg.sender && fee >= CANCELLATION_FEE) { requests[requestId].fee = CANCELLED_FEE_FLAG; if (!msg.sender.send(fee - CANCELLATION_FEE)) { Cancel(requestId, msg.sender, requests[requestId].requester, fee - CANCELLATION_FEE, -2); throw; } Cancel(requestId, msg.sender, requests[requestId].requester, requests[requestId].fee, 1); return true; } else { Cancel(requestId, msg.sender, requests[requestId].requester, fee, -1); return false; } }}'
var APPsource = 'pragma solidity ^0.4.9;contract TownCrier { function request(uint8 requestType, address callbackAddr, bytes4 callbackFID, uint timestamp, bytes32[] requestData) public payable returns (uint64); function cancel(uint64 requestId) public returns (bool);}contract Application { event Request(int64 requestId, address requester, uint dataLength, bytes32[] data); event Response(int64 requestId, address requester, uint64 error, uint data); event Cancel(uint64 requestId, address requester, bool success); uint constant MIN_GAS = 30000 + 20000; uint constant GAS_PRICE = 5 * 10 ** 10; uint constant TC_FEE = MIN_GAS * GAS_PRICE; uint constant CANCELLATION_FEE = 25000 * GAS_PRICE; bytes4 constant TC_CALLBACK_FID = bytes4(sha3("response(uint64,uint64,bytes32)")); TownCrier public TC_CONTRACT; address owner; address[2**64] requesters; uint[2**64] fee; function() public payable {} function Application(TownCrier tcCont) public { TC_CONTRACT = tcCont; owner = msg.sender; } function request(uint8 requestType, bytes32[] requestData) public payable { if (msg.value < TC_FEE) { if (!msg.sender.send(msg.value)) { throw; } Request(-1, msg.sender, requestData.length, requestData); return; } uint64 requestId = TC_CONTRACT.request.value(msg.value)(requestType, this, TC_CALLBACK_FID, 0, requestData); if (requestId == 0) { if (!msg.sender.send(msg.value)) { throw; } Request(-2, msg.sender, requestData.length, requestData); return; } requesters[requestId] = msg.sender; fee[requestId] = msg.value; Request(int64(requestId), msg.sender, requestData.length, requestData); } function response(uint64 requestId, uint64 error, bytes32 respData) public { if (msg.sender != address(TC_CONTRACT)) { Response(-1, msg.sender, 0, 0); return; } address requester = requesters[requestId]; requesters[requestId] = 0; if (error < 2) { Response(int64(requestId), requester, error, uint(respData)); } else { requester.send(fee[requestId]); Response(int64(requestId), msg.sender, error, 0); } } function cancel(uint64 requestId) public { if (requestId == 0 || requesters[requestId] != msg.sender) { Cancel(requestId, msg.sender, false); return; } bool tcCancel = TC_CONTRACT.cancel(requestId); if (tcCancel) { requesters[requestId] = 0; if (!msg.sender.send(fee[requestId] - CANCELLATION_FEE)) { Cancel(requestId, msg.sender, false); throw; } Cancel(requestId, msg.sender, true); } else { Cancel (requestId, msg.sender, false); } }}contract FlightInsurance { struct Policy{ address requester; uint64 tc_id; bytes32[] data; uint fee; uint premium; } event Insure(int64 policyId, address requester, uint length, bytes32[] data, uint premium, uint timestamp); event Request(uint64 policyId, address requester, int64 requestId); event Response(int64 policyId, address requester, uint64 requestid, uint64 error, uint data); event Cancel(uint64 policyId, address requester, bool success, uint timestamp); uint constant MIN_GAS = 30000 + 20000; uint constant GAS_PRICE = 5 * 10 ** 10; uint constant TC_FEE = MIN_GAS * GAS_PRICE; uint constant ETHER_TO_WEI = 10 ** 18; uint constant PAYOUT_RATE = 5; uint constant PAYOUT_DELAY = 30; uint constant DAY_EPOCH = 24 * 60 * 60; bytes4 constant TC_CALLBACK_FID = bytes4(sha3("response(uint64,uint64,bytes32)")); TownCrier public TC_CONTRACT; address owner; uint64 policyCnt; Policy[2**64] policies; uint64[2**64] id_map; function() public payable {} function FlightInsurance(TownCrier tcCont) public payable{ TC_CONTRACT = tcCont; owner = msg.sender; policyCnt = 1; } function insure(bytes32[] flightInfo, uint payment) public payable{ payment = payment * ETHER_TO_WEI; if (msg.value < payment + TC_FEE || block.timestamp + DAY_EPOCH > uint(flightInfo[1])) { if (!msg.sender.send(msg.value)) { throw; } Insure(-1, msg.sender, flightInfo.length, flightInfo, payment, uint(flightInfo[1])); return; } uint64 policyId = policyCnt; ++policyCnt; policies[policyId].requester = msg.sender; policies[policyId].tc_id = 0; policies[policyId].data = flightInfo; policies[policyId].fee = msg.value - payment; policies[policyId].premium = payment; Insure(int64(policyId), msg.sender, flightInfo.length, flightInfo, payment, block.timestamp); } function request(uint64 policyId) public { if (policyId == 0 || policies[policyId].requester != msg.sender || policies[policyId].tc_id != 0 || policies[policyId].fee == 0) { Request(policyId, msg.sender, -1); return; } policies[policyId].tc_id = TC_CONTRACT.request.value(policies[policyId].fee)(1, this, TC_CALLBACK_FID, 0, policies[policyId].data); if (policies[policyId].tc_id == 0) { policies[policyId].fee = 0; if (!msg.sender.send(policies[policyId].premium + policies[policyId].fee)) { Request(policyId, msg.sender, -2); throw; } Request(policyId, msg.sender, 0); } else { id_map[policies[policyId].tc_id] = policyId; Request(policyId, msg.sender, int64(policies[policyId].tc_id)); } } function response(uint64 requestId, uint64 error, bytes32 respData) public { if (msg.sender != address(TC_CONTRACT)) { Response(-1, msg.sender, 0, 0, 0); return; } uint64 policyId = id_map[requestId]; address requester = policies[policyId].requester; uint premium = policies[policyId].premium; uint delay = uint(respData); policies[policyId].fee = 0; if (error == 0){ if (delay >= PAYOUT_DELAY) { requester.send(premium * PAYOUT_RATE); } else { owner.send(premium); } } else if (error == 1) { requester.send(premium); } else { requester.send(premium + policies[policyId].fee); } Response(int64(policyId), msg.sender, requestId, error, delay); } function cancel(uint64 policyId) public { if (policyId == 0 || policies[policyId].requester != msg.sender || policies[policyId].fee == 0 || policies[policyId].tc_id != 0 || block.timestamp + DAY_EPOCH > uint(policies[policyId].data[1])) { Cancel(policyId, msg.sender, false, uint(policies[policyId].data[1])); return; } policies[policyId].fee = 0; if (!msg.sender.send(policies[policyId].fee + policies[policyId].premium)) { Cancel(policyId, msg.sender, false, 0); throw; } Cancel(policyId, msg.sender, true, block.timestamp); }}'
var TCcontract = eth.compile.solidity(TCsource)
var APPcontracts = eth.compile.solidity(APPsource)
var TownCrier = eth.contract(TCcontract["<stdin>:TownCrier"].info.abiDefinition)
//var SteamTrade = eth.contract(APPcontracts["<stdin>:SteamTrade"].info.abiDefinition)
var FlightIns = eth.contract(APPcontracts["<stdin>:FlightInsurance"].info.abiDefinition)
var App = eth.contract(APPcontracts["<stdin>:Application"].info.abiDefinition)

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

// TODO: watch RequestLog and print it out
// TODO: Not an emergency

function setup_tc() {
    unlockAccounts();
    var tc = TownCrier.new({
        from: minerAddr, 
        data: TCcontract["<stdin>:TownCrier"].code, 
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

function createSteamTrade(apiKey, item, price) {
    unlockAccounts();
  var tradeContract = SteamTrade.new(
          tc.address, apiKey[0], apiKey[1], item, price, {
              from: sellerAddr, 
              data: APPcontracts["<stdin>:SteamTrade"].code, 
              gas: gasCnt}, 
              function(e, c) { 
                  if (!e) {
                      if (c.address) {
                        console.log('SteamTrade created at: ' + c.address)
                      }
                  } 
                  else {console.log('Failed to create SteamTrade contract: ' + e)}
              });
    mineBlocks(1);
    return tradeContract;
}

function createFlightIns(tc) {
    unlockAccounts();
    var tradeContract = FlightIns.new(
            tc, {
                value: 10e+18,
                from: sellerAddr,
                data: APPcontracts["<stdin>:FlightInsurance"].code,
                gas: gasCnt},
                function(e, c) {
                    if (!e) {
                        if (c.address) {
                            console.log('FlightIns created at: ' + c.address)
                        }
                    }
                    else {console.log('Failed to create FligthIns contract: ' + e)}
                });
    mineBlocks(1);
    return tradeContract;
}

function createApp(tc) {
    unlockAccounts();
    var tradeContract = App.new(
        tc, {
            from: sellerAddr,
            data: APPcontracts["<stdin>:Application"].code,
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

function SteamPurchase(contract, steamId, delay) {
  // var timeoutSecs = Math.floor((new Date((new Date()).getTime() + (delay * 1000))).getTime() / 1000);
  // to simplify, delay is the time for SGX to wait before fetching
  // delay = 60, typically
    unlockAccounts();
  contract.purchase.sendTransaction( steamId, delay, {
      from: buyerAddr, 
      value: 1e+18 + TC_FEE, 
      gas: gasCnt
  });
  mineBlocks(1);
  return "Purchased!"
}

function FlightInsure(contract, flightId, time, fee) {
    unlockAccounts();
contract.insure.sendTransaction([flightId, time], fee, {
        from: buyerAddr,
        value: fee * 1e+18 + TC_FEE,
        gas: gasCnt
    });
    mineBlocks(1);
    return "Insured!"
}

function FlightRequest(contract, id) {
    unlockAccounts();
    contract.request.sendTransaction(id, {
        from: buyerAddr,
        gas: gasCnt
    });
    mineBlocks(1);
    return "Request sent!"
}

function FlightCancel(contract, id) {
    unlockAccounts();
    contract.cancel.sendTransaction(id, {
        from: buyerAddr,
        gas: gasCnt
    });
    mineBlocks(1);
    return "Request Cancelled."
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

function TestSteam(contract, steamId, delay) {
    unlockAccounts();
    for (var i = 0; i < 1000; ++i) {
        SteamPurchase(contract, steamId, delay);
    }
}


function check_balance(){
    var before = Number(eth.getBalance(sellerAddr));
    var before_b = Number(eth.getBalance(buyerAddr));

    mineBlocks(1);

    var after = Number(eth.getBalance(sellerAddr));
    var after_b = Number(eth.getBalance(buyerAddr));

    console.log('seller balance before: ' + before*1e-18 + ' ether');
    console.log('seller balance after: ' + after*1e-18 + ' ether');
    console.log('balance delta: ' + (after - before)*1e-18 + ' ether');

    console.log('buyer balance before: ' + before_b*1e-18 + ' ether');
    console.log('buyer balance after: ' + after_b*1e-18 + ' ether');
    console.log('balance delta: ' + (after_b - before_b)*1e-18 + ' ether');

    return "Success!"
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
// loadScript("demorc.js");
// tc = setup_tc();
// app = createApp(tc.address);
// setup_log(tc, app, 0);
// Request(app, 1, ['FJM273', pad(1492100100, 64)]);
// Request(app, 2, ['f68d2a32cf17b1312c6db3f236a38c94', '4c9f92f6ec1e2a20a1413d0ac1b867a3', '32884794', pad(1456380265, 64), pad(1, 64), 'Portal']);
// Request(app, 3, ['GOOG', pad(1262390400,64)]);
// Request(app, 4, ['1ZE331480394808282']);
// Request(app, 5, ['bitcoin']);
// FlightInsure(fi, 'FJM273', pad(1492100100, 64), 1);

// var tradeContract = createSteamTrade(encryptedApiKey, 'Portal', 1e+18);
// TestSteam(tradeContract, buyerSteamId, 60);
// var insContract = createFlightIns();
// FlightInsure(insContract, "fightID", "epochtime", 5);
