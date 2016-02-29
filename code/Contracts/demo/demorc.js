var minerAddr = eth.accounts[0]
var sellerAddr = eth.accounts[1]
var buyerAddr = eth.accounts[2]
var sgxAddr = "0x89b44e4d3c81ede05d0f5de8d1a68f754d73d997"

var gasCnt = 3e+6

personal.unlockAccount(minerAddr, '123123')
personal.unlockAccount(sellerAddr, '123123')
personal.unlockAccount(buyerAddr, '123123')
// personal.unlockAccount(sgxAddr)

var encryptedApiKey = ['deadbeef', 'deadbeef']
var buyerSteamId = '32884794'
var source = 'contract TownCrier { struct Request { address requester; uint fee; address callbackAddr; bytes4 callbackFID; bytes32 paramsHash; } event RequestLog(address self, int16 flag); event RequestInfo(uint64 id, uint8 requestType, address requester, uint fee, address callbackAddr, bytes32 paramsHash, bytes32[] requestData); event DeliverLog(uint gasLeft, int flag); event DeliverInfo(uint64 requestId, uint fee, uint gasPrice, uint gasLeft, uint callbackGas, bytes32 paramsHash, bytes32 response); event Cancel(uint64 requestId, address canceller, address requester, int flag); address constant SGX_ADDRESS = 0x89b44e4d3c81ede05d0f5de8d1a68f754d73d997; uint public constant GAS_PRICE = 5 * 10**10; uint public constant MAX_FEE = (31 * 10**5) * GAS_PRICE; uint public constant MIN_FEE = 35000 * GAS_PRICE; uint public constant CANCELLATION_FEE = 24500 * GAS_PRICE; uint constant CANCELLED_FEE_FLAG = 1; uint constant DELIVERED_FEE_FLAG = 0; uint64 requestCnt; Request[2**64] requests; function TownCrier() public { requestCnt = 1; requests[0].requester = msg.sender; } function request(uint8 requestType, address callbackAddr, bytes4 callbackFID, bytes32[] requestData) public returns (uint64) { RequestLog(this, 0); if (msg.value < MIN_FEE || msg.value > MAX_FEE) { RequestInfo(0, requestType, msg.sender, msg.value, callbackAddr, 0, requestData); RequestLog(this, -1); return 0; } else { uint64 requestId = requestCnt; requestCnt++; bytes32 paramsHash = sha3(requestType, requestData); requests[requestId].requester = msg.sender; requests[requestId].fee = msg.value; requests[requestId].callbackAddr = callbackAddr; requests[requestId].callbackFID = callbackFID; requests[requestId].paramsHash = paramsHash; RequestInfo(requestId, requestType, msg.sender, msg.value, callbackAddr, paramsHash, requestData); RequestLog(this, 1); return requestId; } } function deliver(uint64 requestId, bytes32 paramsHash, bytes32 respData) public { uint fee = requests[requestId].fee; if (msg.sender != SGX_ADDRESS || requests[requestId].requester == 0 || fee == DELIVERED_FEE_FLAG) { DeliverInfo(requestId, fee, tx.gasprice, msg.gas, 0, paramsHash, respData); DeliverLog(msg.gas, -1); return; } else if (requests[requestId].paramsHash != paramsHash) { DeliverInfo(requestId, fee, tx.gasprice, msg.gas, 0, paramsHash, respData); DeliverLog(msg.gas, -4); return; } else if (requests[requestId].fee == CANCELLED_FEE_FLAG) { DeliverLog(msg.gas, 1); SGX_ADDRESS.send(CANCELLATION_FEE); requests[requestId].fee = DELIVERED_FEE_FLAG; DeliverLog(msg.gas, int(CANCELLATION_FEE)); return; } DeliverLog(msg.gas, 8); SGX_ADDRESS.send(fee); requests[requestId].fee = DELIVERED_FEE_FLAG; DeliverLog(msg.gas, 16); uint callbackGas = (fee - MIN_FEE) / tx.gasprice; DeliverInfo(requestId, fee, tx.gasprice, msg.gas, callbackGas, paramsHash, respData); bool deliverSuccess = requests[requestId].callbackAddr.call.gas(callbackGas)(requests[requestId].callbackFID, requestId, respData); if (deliverSuccess) { DeliverLog(msg.gas, 32); } else { DeliverLog(msg.gas, -2); } } function cancel(uint64 requestId) public returns (bool) { Cancel(requestId, msg.sender, requests[requestId].requester, int(fee)); uint fee = requests[requestId].fee; if (requests[requestId].requester == msg.sender && fee > CANCELLATION_FEE) { msg.sender.send(fee - CANCELLATION_FEE); requests[requestId].fee = CANCELLED_FEE_FLAG; Cancel(requestId, msg.sender, requests[requestId].requester, int(CANCELLED_FEE_FLAG)); return true; } else { Cancel(requestId, msg.sender, requests[requestId].requester, -int(CANCELLED_FEE_FLAG)); return false; } }}contract SteamTrade { event UINT(uint u); event Buy(bytes32 buyerId, uint32 time, bytes32[2] encSellerApi, bytes32 item, int flag); address owner; TownCrier public TC_CONTRACT; bytes32 public ID_S; bytes32 public ITEM; bytes32[2] public encAPI; uint public P; address[2**64] buyers; uint constant TC_FEE = (35000 + 20000) * 5 * 10**10; bytes4 constant TC_CALLBACK_FID = 0x3d622256; function SteamTrade(TownCrier tcCont, bytes32 encApiKey0, bytes32 encApiKey1, bytes32 item, uint p) public { TC_CONTRACT = tcCont; owner = msg.sender; ITEM = item; P = p; encAPI[0] = encApiKey0; encAPI[1] = encApiKey1; } function purchase(bytes32 ID_B, uint32 T_B) returns (uint) { uint i; uint j; if (msg.value != P + TC_FEE) { Buy(ID_B, T_B, encAPI, ITEM, -1); msg.sender.send(msg.value); return 0; } Buy(ID_B, T_B, encAPI, ITEM, 1); bytes32[] memory format = new bytes32[](6); format[0] = encAPI[0]; format[1] = encAPI[1]; format[2] = ID_B; format[3] = bytes32(T_B); format[4] = bytes32(1); format[5] = ITEM; uint64 requestId = TC_CONTRACT.request.value(TC_FEE)(2, this, TC_CALLBACK_FID, format); buyers[requestId] = msg.sender; UINT(format.length); return format.length; } function pay(uint64 requestId, bytes32 result) public { address buyer = buyers[requestId]; if (msg.sender != address(TC_CONTRACT)) { UINT(666); return; } if (buyer == 0) { return; } if (uint(result) > 0) { owner.send(P); UINT(1); } else { buyer.send(P); UINT(0); } } }'
var contracts = eth.compile.solidity(source)
var TownCrier = eth.contract(contracts.TownCrier.info.abiDefinition)
var SteamTrade = eth.contract(contracts.SteamTrade.info.abiDefinition)

function setup_tc() {
  var tc = TownCrier.new({from: minerAddr, data: contracts.TownCrier.code, gas: gasCnt}, function(e, c) {if (!e && c.address) {console.log(c.address)} else {console.log(e)}})
  miner.start(1); admin.sleepBlocks(1); miner.stop(1);
  return tc;
}

function createSteamTrade(apiKey, item, price) {
  var tradeContract = SteamTrade.new(tc.address, apiKey[0], apiKey[1], item, price, {from: sellerAddr, data: contracts.SteamTrade.code, gas: gasCnt}, function(e, c) {if (!e && c.address) {console.log(c.address)} else {console.log(e)}});
  miner.start(1); admin.sleepBlocks(1); miner.stop(1);
  return tradeContract;
}

function purchase(contract, steamId, delay) {
  // var timeoutSecs = Math.floor((new Date((new Date()).getTime() + (delay * 1000))).getTime() / 1000);
  // to simplify, delay is the time for SGX to wait before fetching
  // delay = 60, typically
  contract.purchase.sendTransaction(steamId, delay, {from: buyerAddr, value: 1e+18 + (55 * 5e+13), gas: gasCnt})
  miner.start(1); admin.sleepBlocks(1); miner.stop(1);
}

function check_balance(){
    var before = Number(debug.dumpBlock('latest').accounts[sellerAddr.substring(2)].balance);
    miner.start(1); admin.sleepBlocks(1); miner.stop(1);
    var after = Number(debug.dumpBlock('latest').accounts[sellerAddr.substring(2)].balance);

    console.log('seller balance before: ' + before);
    console.log('seller balance after: ' + before);
    console.log('balance delta: ' + (before - after)*1e-18);
}

/* =========== The following should be run line-by-line as a demo =========== */

// tc = setup_tc();
// var tradeContract = createSteamTrade(encryptedApiKey, 'Portal', 1e+18);
// purchase(tradeContract, buyerSteamId, 60);
// check_balance();
