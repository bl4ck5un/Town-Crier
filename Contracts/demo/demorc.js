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

personal.unlockAccount(minerAddr, '123123')
personal.unlockAccount(sellerAddr, '123123')
personal.unlockAccount(buyerAddr, '123123')
// personal.unlockAccount(sgxAddr)


var encryptedApiKey = [
    '0xf68d2a32cf17b1312c6db3f236a38c94', 
    '0x4c9f92f6ec1e2a20a1413d0ac1b867a3']

var buyerSteamId = String(32884794);
var TCsource = 'pragma solidity ^0.4.9;contract TownCrier { struct Request { address requester; uint fee; address callbackAddr; bytes4 callbackFID; bytes32 paramsHash; } event RequestLog(address self, int16 flag); event RequestInfo(uint64 id, uint8 requestType, address requester, uint fee, address callbackAddr, bytes32 paramsHash, uint timestamp, bytes32[] requestData); event DeliverLog(uint gasLeft, int flag); event DeliverInfo(uint64 requestId, uint fee, uint gasPrice, uint gasLeft, uint callbackGas, bytes32 paramsHash, uint64 error, bytes32 respData); event Cancel(uint64 requestId, address canceller, address requester, uint fee, int flag); address constant SGX_ADDRESS = 0x89B44e4d3c81EDE05D0f5de8d1a68F754D73d997; uint public constant GAS_PRICE = 5 * 10**10; uint public constant MAX_FEE = (31 * 10**10) * GAS_PRICE; uint public constant MIN_FEE = 30000 * GAS_PRICE; uint public constant CANCELLATION_FEE = 25000 * GAS_PRICE; uint constant CANCELLED_FEE_FLAG = 1; uint constant DELIVERED_FEE_FLAG = 0; uint64 requestCnt; Request[2**64] requests; function () {} function TownCrier() public { requestCnt = 1; requests[0].requester = msg.sender; } function request(uint8 requestType, address callbackAddr, bytes4 callbackFID, uint timestamp, bytes32[] requestData) public payable returns (uint64) { RequestLog(this, 0); if (msg.value < MIN_FEE || msg.value > MAX_FEE) { RequestLog(this, -1); if (!msg.sender.send(msg.value)) { RequestLog(this, -2); throw; } return 0; } else { uint64 requestId = requestCnt; requestCnt++; bytes32 paramsHash = sha3(requestType, requestData); requests[requestId].requester = msg.sender; requests[requestId].fee = msg.value; requests[requestId].callbackAddr = callbackAddr; requests[requestId].callbackFID = callbackFID; requests[requestId].paramsHash = paramsHash; RequestInfo(requestId, requestType, msg.sender, msg.value, callbackAddr, paramsHash, timestamp, requestData); RequestLog(this, 1); return requestId; } } function deliver(uint64 requestId, bytes32 paramsHash, uint64 error, bytes32 respData) public { if (msg.sender != SGX_ADDRESS || requestId <= 0 || requests[requestId].requester == 0 || requests[requestId].fee == DELIVERED_FEE_FLAG) { DeliverLog(msg.gas, -1); return; } uint fee = requests[requestId].fee; if (requests[requestId].paramsHash != paramsHash) { DeliverLog(msg.gas, -4); return; } else if (fee == CANCELLED_FEE_FLAG) { DeliverLog(msg.gas, 1); if (!SGX_ADDRESS.send(CANCELLATION_FEE)){ DeliverLog(msg.gas, -8); throw; } requests[requestId].fee = DELIVERED_FEE_FLAG; DeliverLog(msg.gas, int(CANCELLATION_FEE)); return; } DeliverLog(msg.gas, 8); if (error < 2) { if (!SGX_ADDRESS.send(fee)) { DeliverLog(msg.gas, -16); throw; } } else { if (!requests[requestId].requester.send(fee)) { DeliverLog(msg.gas, -32); throw; } } requests[requestId].fee = DELIVERED_FEE_FLAG; DeliverLog(msg.gas, 16); uint callbackGas = (fee - MIN_FEE) / tx.gasprice; DeliverInfo(requestId, fee, tx.gasprice, msg.gas, callbackGas, paramsHash, error, respData); bool deliverSuccess = requests[requestId].callbackAddr.call.gas(callbackGas)(requests[requestId].callbackFID, requestId, error, respData); if (deliverSuccess) { DeliverLog(msg.gas, 32); } else { DeliverLog(msg.gas, -2); } } function cancel(uint64 requestId) public returns (bool) { uint fee = requests[requestId].fee; if (requests[requestId].requester == msg.sender && fee >= CANCELLATION_FEE) { if (!msg.sender.send(fee - CANCELLATION_FEE)) { Cancel(requestId, msg.sender, requests[requestId].requester, fee - CANCELLATION_FEE, -2); throw; } requests[requestId].fee = CANCELLED_FEE_FLAG; Cancel(requestId, msg.sender, requests[requestId].requester, requests[requestId].fee, 1); return true; } else { Cancel(requestId, msg.sender, requests[requestId].requester, fee, -1); return false; } }}'
var APPsource = 'pragma solidity ^0.4.9;contract TownCrier { function request(uint8 requestType, address callbackAddr, bytes4 callbackFID, uint timestamp, bytes32[] requestData) public payable returns (uint64); function cancel(uint64 requestId) public returns (bool);}contract Flight { event Request(int64 requestId, address requester, uint dataLength, bytes32[] data); event Response(int64 requestId, address requester, uint64 error, uint delay); event Cancel(uint64 requestId, address requester, bool success); uint constant MIN_GAS = 30000 + 20000; uint constant GAS_PRICE = 5 * 10 ** 10; uint constant TC_FEE = MIN_GAS * GAS_PRICE; uint constant CANCELLATION_FEE = 25000 * GAS_PRICE; bytes4 constant TC_CALLBACK_FID = bytes4(sha3("response(uint64,uint64,bytes32)")); TownCrier public TC_CONTRACT; address owner; address[2**64] requesters; uint[2**64] fee; function() payable {} function Flight(TownCrier tcCont) public { TC_CONTRACT = tcCont; owner = msg.sender; } function request(bytes32[] requestData) public payable { if (msg.value < TC_FEE) { if (!msg.sender.send(msg.value)) { throw; } Request(-1, msg.sender, requestData.length, requestData); return; } uint64 requestId = TC_CONTRACT.request.value(msg.value)(1, this, TC_CALLBACK_FID, 0, requestData); if (requestId == 0) { if (!msg.sender.send(msg.value)) { throw; } Request(-2, msg.sender, requestData.length, requestData); return; } requesters[requestId] = msg.sender; fee[requestId] = msg.value; Request(int64(requestId), msg.sender, requestData.length, requestData); } function response(uint64 requestId, uint64 error, bytes32 respData) public { if (msg.sender != address(TC_CONTRACT)) { Response(-1, msg.sender, 0, 0); return; } address requester = requesters[requestId]; requesters[requestId] = 0; if (error < 2) { Response(int64(requestId), requester, error, uint(respData)); } else { if (!requester.send(fee[requestId])) { Response(-2, msg.sender, error, 0); throw; } Response(int64(requestId), msg.sender, error, 0); } } function cancel(uint64 requestId) public { if (requestId == 0 || requesters[requestId] != msg.sender) { Cancel(requestId, msg.sender, false); return; } bool tcCancel = TC_CONTRACT.cancel(requestId); if (tcCancel) { if (!msg.sender.send(fee[requestId] - CANCELLATION_FEE)) { Cancel(requestId, msg.sender, false); throw; } Cancel(requestId, msg.sender, true); requesters[requestId] = 0; } else { Cancel (requestId, msg.sender, false); } }}contract FlightIns { event Insure(address beneficiary, uint dataLength, bytes32[] data, int64 requestId); event PaymentLog(int flag); event PaymentInfo(address payee, uint payeeBalance, uint gasRemaining, uint64 requestId, uint64 error, uint256 delay); event FlightCancel(address canceller, address requester, bool success); uint constant TC_FEE = (30000 + 20000) * 5 * 10**10; uint constant FEE = 10**18; uint constant PAYOUT = 2 * 10**19; uint32 constant PAYOUT_DELAY = 30; bytes4 constant TC_CALLBACK_FID = bytes4(sha3("pay(uint64,uint64,bytes32)")); TownCrier public TC_CONTRACT; address owner; address[2**64] requesters; uint[2**64] premium; function() payable {} function FlightIns(TownCrier tcCont) public payable{ TC_CONTRACT = tcCont; owner = msg.sender; } function insure(bytes32[] encryptedFlightInfo, uint payment) public payable{ if (msg.value < payment * FEE + TC_FEE) { if (!msg.sender.send(msg.value)) { throw; } Insure(msg.sender, encryptedFlightInfo.length, encryptedFlightInfo, -1); return; } uint64 requestId = TC_CONTRACT.request.value(TC_FEE)(1, this, TC_CALLBACK_FID, 0, encryptedFlightInfo); if (requestId == 0) { if (!msg.sender.send(msg.value)) { throw; } Insure(msg.sender, encryptedFlightInfo.length, encryptedFlightInfo, -2); return; } requesters[requestId] = msg.sender; premium[requestId] = payment; Insure(msg.sender, encryptedFlightInfo.length, encryptedFlightInfo, int64(requestId)); } function pay(uint64 requestId, uint64 error, bytes32 respData) public { address requester = requesters[requestId]; if (msg.sender != address(TC_CONTRACT)) { PaymentLog(-1); return; } else if (requesters[requestId] == 0) { PaymentLog(-2); return; } PaymentLog(1); uint delay = uint(respData); if (error == 0){ if (delay >= 30 || delay < 0) { if (!requester.send(premium[requestId] * 5 * FEE)) { PaymentLog(-4); throw; } } } else { if (!requester.send(premium[requestId] * FEE)) { PaymentLog(-8); throw; } } PaymentInfo(requester, premium[requestId], msg.gas, requestId, error, delay); requesters[requestId] = 0; PaymentLog(2); } function cancel(uint64 requestId) public returns (bool) { if (requesters[requestId] == msg.sender) { bool tcCancel = TC_CONTRACT.cancel(requestId); if (tcCancel) { FlightCancel(msg.sender, requesters[requestId], true); requesters[requestId] = 0; msg.sender.send(FEE); return true; } } FlightCancel(msg.sender, requesters[requestId], false); return false; }}'
var TCcontract = eth.compile.solidity(TCsource)
var APPcontracts = eth.compile.solidity(APPsource)
var TownCrier = eth.contract(TCcontract["<stdin>:TownCrier"].info.abiDefinition)
//var SteamTrade = eth.contract(APPcontracts["<stdin>:SteamTrade"].info.abiDefinition)
var FlightIns = eth.contract(APPcontracts["<stdin>:FlightIns"].info.abiDefinition)
var Flight = eth.contract(APPcontracts["<stdin>:Flight"].info.abiDefinition)

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


function setup_log(tc, tradeContract) {
	tc.RequestLog(function(e,r) {
		if (!e) { console.log('RequestLog: ' + JSON.stringify(r.args))}
		else { console.log(e)}
	});

	tc.RequestInfo(function(e,r) { 
		if (!e) { console.log('RequestInfo: ' + JSON.stringify(r.args)); } 
		else {console.log(e)}
	});

	tradeContract.UINT(function(e,r) { 
		if (!e) { console.log('UNIT: ' + JSON.stringify(r.args)); } 
		else {console.log(e)}
	});

	tradeContract.Buy(function(e,r) { 
		if (!e) { console.log('Buy: ' + JSON.stringify(r.args)); } 
		else {console.log(e)}
	});
}

// TODO: watch RequestLog and print it out
// TODO: Not an emergency

function setup_tc() {
    var tc = TownCrier.new({
        from: minerAddr, 
        data: TCcontract["<stdin>:TownCrier"].code, 
        gas: gasCnt}, function(e, c) {
            if (!e){
                if (c.address) {
                    console.log("Town Crier created at: " + c.address)
                }
            } 
            else {console.log("Failed to create Town Crier contract: " + e)}
        });
    mineBlocks(1);
    return tc;
}

function createSteamTrade(apiKey, item, price) {
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
                  else {console.log("Failed to create SteamTrade contract: " + e)}
              });
    mineBlocks(1);
    return tradeContract;
}

function createFlightIns() {
    var tradeContract = FlightIns.new(
            tc.address, {
                value: 100e+18,
                from: sellerAddr,
                data: APPcontracts["<stdin>:FlightIns"].code,
                gas: gasCnt},
                function(e, c) {
                    if (!e) {
                        if (c.address) {
                            console.log('FlightIns created at: ' + c.address)
                        }
                    }
                    else {console.log("Failed to create FligthIns contract: " + e)}
                });
    mineBlocks(1);
    return tradeContract;
}

function createFlight() {
    var tradeContract = Flight.new(
        tc.address, {
            from: sellerAddr,
            data: APPcontracts["<stdin>:Flight"].code,
            gas:gasCnt},
            function(e, c) {
                if (!e) {
                    if (c.address) {
                        console.log('Flight created at: ' + c.address)
                    }
                } else {
                    console.log('Failed to create Flight contract: ' + e)}
                });
    mineBlocks(1);
    return tradeContract;
}

function SteamPurchase(contract, steamId, delay) {
  // var timeoutSecs = Math.floor((new Date((new Date()).getTime() + (delay * 1000))).getTime() / 1000);
  // to simplify, delay is the time for SGX to wait before fetching
  // delay = 60, typically
  contract.purchase.sendTransaction( steamId, delay, {
      from: buyerAddr, 
      value: 1e+18 + TC_FEE, 
      gas: gasCnt
  });
  mineBlocks(1);
  return "Purchased!"
}

function FlightInsure(contract, flightID, time, fee) {
contract.insure.sendTransaction([web3.fromAscii(flightID, 32), web3.fromAscii(time, 32)], fee, {
        from: buyerAddr,
        value: fee * 1e+18 + TC_FEE,
        gas: gasCnt
    });
    mineBlocks(1);
    return "Insured!"
}

function FlightRequest(contract, fligthID, time, fee) {
    contract.request.sendTransaction([web3.fromAscii(fligthID, 32), web3.fromAscii(time, 32)], {
        from: buyerAddr,
        value: fee,
        gas: gasCnt
    });
    mineBlocks(1);
    return "Request sent!"
}

function Cancel(contract, id) {
    contract.cancel.sendTransaction(id, {
        from: buyerAddr,
        gas: gasCnt
    });
    mineBlocks(1);
    return "Cancellation request sent!"
}

function TestSteam(contract, steamId, delay) {
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

/* =========== The following should be run line-by-line as a demo =========== */
// loadScript("demorc.js");
// tc = setup_tc();
// var tradeContract = createSteamTrade(encryptedApiKey, 'Portal', 1e+18);
// TestSteam(tradeContract, buyerSteamId, 60);
// var insContract = createFlightIns();
// FlightInsure(insContract, "fightID", "epochtime", 5);
