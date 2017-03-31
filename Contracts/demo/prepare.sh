cat << EOF
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
EOF

# Make sure you modify the TC contract to have the right SGX address.
SRC=$(sed 's/\(\/\/.*$\|import "[^"]\+";\)//' ../TownCrier.sol ../SteamTrade.sol ../FlightIns.sol | paste -sd '' | sed 's/\s\+/ /g')

cat <<EOF
var source = '$SRC'
EOF

cat <<EOF > full.sol
$SRC
EOF

cat <<EOF
var contracts = eth.compile.solidity(source)
var TownCrier = eth.contract(contracts["<stdin>:TownCrier"].info.abiDefinition)
var SteamTrade = eth.contract(contracts["<stdin>:SteamTrade"].info.abiDefinition)
var FlightIns = eth.contract(contracts["<stdin>:FlightIns"].info.abiDefinition)

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
        data: contracts["<stdin>:TownCrier"].code, 
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
              data: contracts["<stdin>:SteamTrade"].code, 
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
                data: contracts["<stdin>:FlightIns"].code,
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


/* =========== The following should be run line-by-line as a demo =========== */
// loadScript("demorc.js");
// tc = setup_tc();
// var tradeContract = createSteamTrade(encryptedApiKey, 'Portal', 1e+18);
// TestSteam(tradeContract, buyerSteamId, 60);
// var insContract = createFlightIns();
// FlightInsure(insContract, "fightID", "epochtime", 5);
EOF
