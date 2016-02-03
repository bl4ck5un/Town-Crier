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

echo "source = '$(sed 's/\/\/.*$//' TownCrier.sol | paste -sd '' | sed 's/\s\+/ /g')'"

echo "contracts = eth.compile.solidity(source)"
echo "TownCrier = eth.contract(contracts.TownCrier.info.abiDefinition)"
echo "FlightInsurance = eth.contract(contracts.FlightInsurance.info.abiDefinition)"

echo "tc = TownCrier.new({from: userAddr, data: contracts.TownCrier.code, gas: gasCnt}, function(e, c) {if (!e && c.address) {console.log(c.address)} else {console.log(e)}})"

echo "miner.start(1); admin.sleepBlocks(1); miner.stop();"

echo "tc.RequestLog(function(e,r) { console.log('RequestLog: ' + JSON.stringify(r.args)) })"
echo "tc.RequestInfo(function(e,r) { console.log('RequestInfo: ' + JSON.stringify(r.args)) })"
#echo 'tc.request.sendTransaction(0, "0x01", "0000", "test", {from: userAddr, value: Math.pow(10,17), gas: gasCnt})'

echo "fi = FlightInsurance.new(tc.address, {from: userAddr, data: contracts.FlightInsurance.code, gas: gasCnt, value: Math.pow(10,22)}, function(e, c) {if (!e && c.address) {console.log(c.address)} else {console.log(e)}})"

echo "miner.start(1); admin.sleepBlocks(1); miner.stop();"

echo "dataAry = Array(256); for (var i = 0; i < dataAry.length; i++) { dataAry[i] = 'a'; }; dataAry = dataAry.join('')"
#echo "dataAry = 'a'"
echo "fi.Insure(function(e,r) { if (!e) { console.log('Insure: ' + JSON.stringify(r.args)) } else { console.log(e) } })"
echo "fi.insure.sendTransaction(dataAry, {from: userAddr, gas: gasCnt, value: 5 * Math.pow(10,18)})"

echo "miner.start(1); admin.sleepBlocks(1); miner.stop();"

echo "fi.PaymentLog(function(e,r) { if (!e) { console.log('PaymentLog: ' + JSON.stringify(r.args)) } else { console.log(e) } })"
echo "fi.PaymentInfo(function(e,r) { if (!e) { console.log('PaymentInfo: ' + JSON.stringify(r.args)) } else { console.log(e) } })"
echo "tc.DeliverLog(function(e,r) { console.log('DeliverLog: ' + JSON.stringify(r.args)) })"
echo "tc.DeliverInfo(function(e,r) { console.log('DeliverInfo: ' + JSON.stringify(r.args)) })"

echo "sgxBeforeCash1 = Number(debug.dumpBlock('latest').accounts['9d10ea5ad51e1af69cd8d4dcfa60f577818607b2'].balance.substring(0,12))"
echo "sgxBeforeCash2 = Number(debug.dumpBlock('latest').accounts['9d10ea5ad51e1af69cd8d4dcfa60f577818607b2'].balance.substring(12))"

echo "tc.deliver.sendTransaction(1, 0, dataAry, '0x00000000000000000000000000000000000000000000000000000000000000aa', {from: sgxAddr, gas: gasCnt})"

echo "miner.start(1); admin.sleepBlocks(1); miner.stop();"

echo "sgxAfterCash1 = Number(debug.dumpBlock('latest').accounts['9d10ea5ad51e1af69cd8d4dcfa60f577818607b2'].balance.substring(0,12))"
echo "sgxAfterCash2 = Number(debug.dumpBlock('latest').accounts['9d10ea5ad51e1af69cd8d4dcfa60f577818607b2'].balance.substring(12))"

echo "console.log('sgx lost money? ' + (sgxAfterCash1 < sgxBeforeCash1 || (sgxAfterCash1 === sgxBeforeCash1 && sgxAfterCash2 < sgxBeforeCash2)))"

echo "tc.Cancel(function(e,r) { if (!e) { console.log('Cancel: ' + JSON.stringify(r.args)) } else { console.log(e) } })"
echo "fi.FlightCancel(function(e,r) { if (!e) { console.log('FlightCancel: ' + JSON.stringify(r.args)) } else { console.log(e) } })"
echo "fi.cancel.sendTransaction(1, {from: userAddr, gas: gasCnt})"

echo "miner.start(1); admin.sleepBlocks(1); miner.stop();"
