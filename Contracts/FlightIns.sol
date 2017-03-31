pragma solidity ^0.4.9;

import "TownCrier.sol";

contract FlightIns {
    event Insure(address beneficiary, uint dataLength, bytes32[] data, int72 requestId);
    event PaymentLog(int flag);
    event PaymentInfo(address payee, uint payeeBalance, uint gasRemaining, uint64 requestId, uint64 error, uint256 delay);
    //event FlightCancel(address canceller, address requester, bool success);

    uint constant TC_FEE = (30000 + 30000) * 5 * 10**10;
    uint constant FEE = 10**18;
    uint constant PAYOUT = 2 * 10**19;
    uint32 constant PAYOUT_DELAY = 30;

    bytes4 constant TC_CALLBACK_FID = bytes4(sha3("pay(uint64, uint64, bytes32)"));

    TownCrier public TC_CONTRACT;
    address owner;
    address[2**64] requesters;
    uint[2**64] premium;

    function() payable {} // must be payable

    function FlightIns(TownCrier tcCont) public payable{
        TC_CONTRACT = tcCont;
        owner = msg.sender;
    }

    function insure(bytes32[] encryptedFlightInfo, uint payment) public payable{
        if (msg.value < payment * FEE + TC_FEE) {
            if (!msg.sender.send(msg.value)) {
                throw;
            }
            Insure(msg.sender, encryptedFlightInfo.length, encryptedFlightInfo, -1);
            return;
        }

        uint64 requestId = TC_CONTRACT.request.value(TC_FEE)(1, this, TC_CALLBACK_FID, 0, encryptedFlightInfo);
        if (requestId == 0) {
            if (!msg.sender.send(msg.value)) {
                throw;
            }
            Insure(msg.sender, encryptedFlightInfo.length, encryptedFlightInfo, -2);
            return;
        }

        requesters[requestId] = msg.sender;
        premium[requestId] = payment;
        Insure(msg.sender, encryptedFlightInfo.length, encryptedFlightInfo, int72(requestId));
    }

    function pay(uint64 requestId, uint64 error, bytes32 respData) public {
        address requester = requesters[requestId];
        if (msg.sender != address(TC_CONTRACT)) {
            PaymentLog(-1);
            return;
        } else if (requesters[requestId] == 0) {
            PaymentLog(-2);
            return;
        }

        PaymentLog(1);

        uint delay = uint(respData);

        if (error == 0){ // no problem
            if (delay >= 30 || delay < 0) {
                if (!requester.send(premium[requestId] * 5 * FEE)) {
                    PaymentLog(-4);
                    throw;
                }
            }
        } else { 
            if (!requester.send(premium[requestId] * FEE)) {
                PaymentLog(-8);
                throw;
            }
        }

        PaymentInfo(requester, premium[requestId], msg.gas, requestId, error, delay);
        requesters[requestId] = 0;
        PaymentLog(2);
    }

//    function cancel(uint64 requestId) public returns (bool) {
//        if (requesters[requestId] == msg.sender) {
//            bool tcCancel = TC_CONTRACT.cancel(requestId);
//            if (tcCancel) {
//                FlightCancel(msg.sender, requesters[requestId], true);
//                requesters[requestId] = 0;
//                msg.sender.send(FEE);
//                return true;
//            }
//        }
//        FlightCancel(msg.sender, requesters[requestId], false);
//        return false;
//    }
}

