import "TownCrier.sol";

contract FlightInsurance {
    event Insure(address sender, uint dataLength, bytes32[] data, int72 requestId);
    event PaymentLog(int flag);
    event PaymentInfo(address payee, uint payeeBalance, uint gasRemaining, uint64 requestId, uint delay, uint amount);
    event FlightCancel(address canceller, address requester, bool success);

    uint constant TC_FEE = (35000 + 4400) * 5 * 10**10;
    uint constant FEE = 5 * 10**18;
    uint constant PAYOUT = 10**20;
    uint32 constant PAYOUT_DELAY = 30;

    bytes4 constant TC_CALLBACK_FID = 0x3d622256; // bytes4(sha3("pay(uint64,bytes32)"));

    TownCrier TC_CONTRACT;
    address[2**64] requesters;

    function FlightInsurance(TownCrier tcCont) public {
        TC_CONTRACT = tcCont;
    }

    function insure(bytes32[] encryptedFlightInfo) public {
        if (msg.value != FEE) {
            Insure(msg.sender, encryptedFlightInfo.length, encryptedFlightInfo, -1);
            return;
        }

        Insure(msg.sender, encryptedFlightInfo.length, encryptedFlightInfo, -2);
        uint64 requestId = TC_CONTRACT.request.value(TC_FEE)(0, this, TC_CALLBACK_FID, encryptedFlightInfo);
        Insure(msg.sender, encryptedFlightInfo.length, encryptedFlightInfo, -3);
        requesters[requestId] = msg.sender;
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

        PaymentInfo(requester, requester.balance, msg.gas, requestId, uint(delay), 1);
        if (uint(delay) >= PAYOUT_DELAY) {
            address(requester).send(PAYOUT);
            PaymentInfo(requester, requester.balance, msg.gas, requestId, uint(delay), PAYOUT);
        } else {
            PaymentInfo(requester, requester.balance, msg.gas, requestId, uint(delay), 0);
        }
        requesters[requestId] = 0;
        PaymentLog(2);
    }

    function cancel(uint64 requestId) public returns (bool) {
        if (requesters[requestId] == msg.sender) {
            bool tcCancel = TC_CONTRACT.cancel(requestId);
            if (tcCancel) {
                FlightCancel(msg.sender, requesters[requestId], true);
                requesters[requestId] = 0;
                msg.sender.send(FEE);
                return true;
            }
        }
        FlightCancel(msg.sender, requesters[requestId], false);
        return false;
    }
}

