cat << EOF  
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
EOF

# Make sure you modify the TC contract to have the right SGX aggress.
SRC=$(sed 's/\(\/\/.*$\|import "[^"]\+";\)//' ../TownCrier.sol ../SteamTrade.sol | paste -sd '' | sed 's/\s\+/ /g')

cat <<EOF
var source = '$SRC'
EOF

cat <<EOF
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
EOF
