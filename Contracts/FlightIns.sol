pragma solidity ^0.4.9;

import "TownCrier.sol";

contract FlightIns {
    event Insure(address beneficiary, uint dataLength, bytes32[] data, int72 requestId);
    event PaymentLog(int flag);
    event PaymentInfo(address payee, uint payeeBalance, uint gasRemaining, uint64 requestId, uint delay);
    event FlightCancel(address canceller, address requester, bool success);

    uint constant TC_FEE = (35000 + 20000) * 5 * 10**10;
    uint constant FEE = 10**18;
    uint constant PAYOUT = 2 * 10**19;
    uint32 constant PAYOUT_DELAY = 30;

    bytes4 constant TC_CALLBACK_FID = 0x3d622256; // bytes4(sha3("pay(uint64,bytes32)"));

    TownCrier public TC_CONTRACT;
    address owner;
    address[2**64] requesters;
    uint[2**64] premium;

    function() payable { }

    function FlightIns(TownCrier tcCont) public payable{
        TC_CONTRACT = tcCont;
        owner = msg.sender;
    }

    function insure(bytes32[] encryptedFlightInfo, uint payment) public payable{
        if (msg.value != payment * FEE + TC_FEE) {
            Insure(msg.sender, encryptedFlightInfo.length, encryptedFlightInfo, -1);
            return;
        }

        uint64 requestId = TC_CONTRACT.request.value(TC_FEE)(1, this, TC_CALLBACK_FID, encryptedFlightInfo);
        requesters[requestId] = msg.sender;
        premium[requestId] = payment;
        Insure(msg.sender, encryptedFlightInfo.length, encryptedFlightInfo, int72(requestId));
    }

    function pay(uint64 requestId, bytes32 delay) public {
        address requester = requesters[requestId];
        if (msg.sender != address(TC_CONTRACT)) {
            PaymentLog(-1);
            return;
        } else if (requesters[requestId] == 0) {
            PaymentLog(-2);
            return;
        }

        PaymentLog(1);

        if (uint(delay) == 0) {
            PaymentInfo(requester, premium[requestId], msg.gas, requestId, uint(delay));
        } else if (uint(delay) == 1 || uint(delay) == 2) {
            address(requester).send(premium[requestId] * 5 * FEE);
            PaymentInfo(requester, premium[requestId], msg.gas, requestId, uint(delay));
        } else if (uint(delay) == 3 || uint(delay) == 4) {
            address(requester).send(premium[requestId] * FEE);
            PaymentInfo(requester, premium[requestId], msg.gas, requestId, uint(delay));
        }
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

