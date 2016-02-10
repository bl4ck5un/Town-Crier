#!/bin/bash
#
# Usage:
#    Change the addresses to three distinct addresses that exist on your local blockchain.
#    Make sure userAddr and sgxAddr have a substantial amount of money, and connect using
#    geth with the --etherbase address of minerAddr.

echo 'userAddr = "0xec0907991a5560bfe9ae853adaadb8f7578670af"'
echo 'minerAddr = "0xed0013df219cab5faae57e579b3c5901d40d43ff"'
echo 'sgxAddr = "0x9d10ea5ad51e1af69cd8d4dcfa60f577818607b2"'

echo 'gasCnt = 3 * Math.pow(10,6)'

echo 'personal.unlockAccount(userAddr)'
echo 'cranberry'
echo 'personal.unlockAccount(minerAddr)'
echo 'cranberry2'
echo 'personal.unlockAccount(sgxAddr)'
echo 'sgx'

#echo "source = '$(sed 's/\(\/\/.*$\|import "[^"]\+";\)//' *.sol | paste -sd '' | sed 's/\s\+/ /g')'"
echo "source = '$(sed 's/\(\/\/.*$\|import "[^"]\+";\)//' TownCrier.sol PutOption.sol FlightInsurance.sol | paste -sd '' | sed 's/\s\+/ /g')'"

#solc --abi --optimize -o compiled/abi *.sol
#solc --bin --optimize -o compiled/bin *.sol

#echo "SigVer = eth.contract($(cat compiled/abi/VerifySignature.abi))"
#echo "svCode = '$(cat compiled/bin/VerifySignature.bin)'"
#echo "sv = SigVer.new({from: userAddr, data: svCode, gas: gasCnt}, function(e, c) {if (!e && c.address) {console.log(c.address)} else {console.log(e)}})"

echo "contracts = eth.compile.solidity(source)"
echo "TownCrier = eth.contract(contracts.TownCrier.info.abiDefinition)"
echo "FlightInsurance = eth.contract(contracts.FlightInsurance.info.abiDefinition)"
echo "PutOption = eth.contract(contracts.PutOption.info.abiDefinition)"

#echo "tcCode = '$(cat compiled/bin/TownCrier.bin)'"
#echo "fiCode = '$(cat compiled/bin/FlightInsurance.bin)'"
#echo "poCode = '$(cat compiled/bin/PutOption.bin)'"

tc = TownCrier.new({from: userAddr, data: contracts.TownCrier.code, gas: gasCnt}, function(e, c) {if (!e && c.address) {console.log(c.address)} else {console.log(e)}});
// 0x08be24cd8dcf73f8fa5db42b855b4370bd5c448b
miner.start(1); admin.sleepBlocks(1); miner.stop(1);

// send some money
eth.sendTransaction({from: userAddr, to: tc.address, value: Math.pow(10,15)})
miner.start(1); admin.sleepBlocks(1); miner.stop(1);

tc.RequestLog(function(e,r) { console.log('RequestLog: ' + JSON.stringify(r.args)) })
tc.RequestInfo(function(e,r) { if (!e) { console.log('RequestInfo: ' + JSON.stringify(r.args)) } else { console.log(e) } });

// send raw test
// get Error: PC 00000999: CALLDATACOPY GAS: 2976720 COST: 360287972773330944003 ERROR: Out of gas
tc.request.sendTransaction(0, "0x01", "0000", ["data1", "data2"], {from: userAddr, value: Math.pow(10,10), gas: gasCnt})

// flight insurance
fi = FlightInsurance.new(tc.address, {from: userAddr, data: contracts.FlightInsurance.code, gas: gasCnt, value: Math.pow(10,22)}, function(e, c) {if (!e && c.address) {console.log(c.address)} else {console.log(e)}});
fi.Insure(function(e,r) { if (!e) { console.log('Insure: ' + JSON.stringify(r.args)) } else { console.log(e) } });

fi.insure.sendTransaction(["aaaaaa", "bbbbbb"], {from: userAddr, gas: gasCnt, value: 5 * Math.pow(10,18)})
	
exprDate = (new Date(2016,2,1))
// echo "exprDate = new Date((new Date()).getTime() + 5000000)"
po = PutOption.new(tc.address, 'GOOGL', (50e+19)/2, 50, (700e+19)/2, exprDate.getTime()/1000, {from: userAddr, data: contracts.PutOption.code, gas: gasCnt, value: new BigNumber((700e+19 - 50e+19)*50/2).add(5e+16)}, function(e, c) {if (!e && c.address) {console.log(c.address)} else {console.log(e)}});
miner.start(1); admin.sleepBlocks(1); miner.stop(1);
// 0x453a55115c44b88231cbc0c9423d8a446ec7ed26

po.buy.sendTransaction(20, {from: minerAddr, gas: gasCnt, value: (50e+19)/2 * 20})
miner.start(1); admin.sleepBlocks(1); miner.stop(1);

//echo "var putTs = undefined;"
//echo "var putF = function(e,r) { if (!e) { console.log('Put: ' + JSON.stringify(r.args)); putTs = r.args.timestamp; } else { console.log(e) } }"
//echo "po.Put(putF)"
//#echo "po.Put(function(e,r) { if (!e) { console.log('Put: ' + JSON.stringify(r.args)) } else { console.log(e) } })"
po.put.sendTransaction({from: minerAddr, gas: gasCnt})
miner.start(1); admin.sleepBlocks(1); miner.stop(1);

#echo "tc.Cancel(function(e,r) { if (!e) { console.log('Cancel: ' + JSON.stringify(r.args)) } else { console.log(e) } })"
#echo "fi.FlightCancel(function(e,r) { if (!e) { console.log('FlightCancel: ' + JSON.stringify(r.args)) } else { console.log(e) } })"
#echo "fi.cancel.sendTransaction(1, {from: userAddr, gas: gasCnt})"
#
#echo "miner.start(1); admin.sleepBlocks(1); miner.stop(1);"

#echo "fi.PaymentLog(function(e,r) { if (!e) { console.log('PaymentLog: ' + JSON.stringify(r.args)) } else { console.log(e) } })"
#echo "fi.PaymentInfo(function(e,r) { if (!e) { console.log('PaymentInfo: ' + JSON.stringify(r.args)) } else { console.log(e) } })"
echo "po.Pay(function(e,r) { if (!e) { console.log('Pay: ' + JSON.stringify(r.args)) } else { console.log(e) } })"

echo "tc.DeliverLog(function(e,r) { console.log('DeliverLog: ' + JSON.stringify(r.args)) })"
echo "tc.DeliverInfo(function(e,r) { console.log('DeliverInfo: ' + JSON.stringify(r.args)) })"
##echo "tc.DeliverSig(function(e,r) { console.log('DeliverSig: ' + JSON.stringify(r.args)) })"

echo "sgxBeforeCash1 = Number(debug.dumpBlock('latest').accounts['9d10ea5ad51e1af69cd8d4dcfa60f577818607b2'].balance.substring(0,12))"
echo "sgxBeforeCash2 = Number(debug.dumpBlock('latest').accounts['9d10ea5ad51e1af69cd8d4dcfa60f577818607b2'].balance.substring(12))"

##echo "signature = eth.sign(sgxAddr, tc.hash(1, 0, dataAry, '0x00000000000000000000000000000000000000000000000000000000000000aa'))"
##echo "sig_v = Number(signature.substring(0,4))"
##echo "sig_r = '0x' + signature.substring(4,68)"
##echo "sig_s = '0x' + signature.substring(68)"
#echo "tc.deliver.sendTransaction(1, 0, dataAry, '0x00000000000000000000000000000000000000000000000000000000000000aa', {from: sgxAddr, gas: gasCnt, gasPrice: 5e+10})"
echo "tc.deliver.sendTransaction(1, 1, ['GOOGL', putTs], '0x0000000000000000000000000000000000000000000000000000000000000258', {from: sgxAddr, gas: gasCnt, gasPrice: 5e+10})"

echo "miner.start(1); admin.sleepBlocks(1); miner.stop(1);"

echo "sgxAfterCash1 = Number(debug.dumpBlock('latest').accounts['9d10ea5ad51e1af69cd8d4dcfa60f577818607b2'].balance.substring(0,12))"
echo "sgxAfterCash2 = Number(debug.dumpBlock('latest').accounts['9d10ea5ad51e1af69cd8d4dcfa60f577818607b2'].balance.substring(12))"

echo "console.log('sgx lost money? ' + (sgxAfterCash1 < sgxBeforeCash1 || (sgxAfterCash1 === sgxBeforeCash1 && sgxAfterCash2 < sgxBeforeCash2)))"
