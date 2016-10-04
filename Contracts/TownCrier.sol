pragma solidity ^0.4.2;

contract TownCrier {
    struct Request {
        address requester;
        uint fee;
        address callbackAddr;
        bytes4 callbackFID;
        bytes32 paramsHash;
    }

    event RequestLog(address self, int16 flag);
    event RequestInfo(uint64 id, uint8 requestType, address requester, uint fee, address callbackAddr, bytes32 paramsHash, bytes32[] requestData);
    event DeliverLog(uint gasLeft, int flag);
    event DeliverInfo(uint64 requestId, uint fee, uint gasPrice, uint gasLeft, uint callbackGas, bytes32 paramsHash, bytes32 response);
//    event DeliverSig(uint8 v, bytes32 r, bytes32 s, address recoveredAddr);
    event Cancel(uint64 requestId, address canceller, address requester, int flag);

    address constant SGX_ADDRESS = 0x89b44e4d3c81ede05d0f5de8d1a68f754d73d997;

    uint public constant GAS_PRICE = 5 * 10**10;
    uint public constant MAX_FEE = (31 * 10**5) * GAS_PRICE;

    uint public constant MIN_FEE = 35000 * GAS_PRICE;
    uint public constant CANCELLATION_FEE = 24500 * GAS_PRICE;

    uint constant CANCELLED_FEE_FLAG = 1;
    uint constant DELIVERED_FEE_FLAG = 0;

    uint64 requestCnt;
    Request[2**64] requests;

    // Contracts that receive Ether but do not define a fallback function throw
    // an exception, sending back the Ether (this was different before Solidity
    // v0.4.0). So if you want your contract to receive Ether, you have to
    // implement a fallback function.
    function () { }

    function TownCrier() public {
        // Start request IDs at 1 for two reasons:
        //   1. We can use 0 to denote an invalid request (ids are unsigned)
        //   2. Storage is more expensive when changing something from zero to non-zero,
        //      so this means the first request isn't randomly more expensive.
        // Reason 2 also makes us want to initialize an array value somewhere.
        requestCnt = 1;
        requests[0].requester = msg.sender;
    }

    function request(uint8 requestType, address callbackAddr, bytes4 callbackFID, bytes32[] requestData) public payable returns (uint64) {
        RequestLog(this, 0);
        if (msg.value < MIN_FEE || msg.value > MAX_FEE) {
            RequestInfo(0, requestType, msg.sender, msg.value, callbackAddr, 0, requestData);
            RequestLog(this, -1);
            return 0;
        } else {
            uint64 requestId = requestCnt;
            requestCnt++;

            bytes32 paramsHash = sha3(requestType, requestData);
            requests[requestId].requester = msg.sender;
            requests[requestId].fee = msg.value;
            requests[requestId].callbackAddr = callbackAddr;
            requests[requestId].callbackFID = callbackFID;
            requests[requestId].paramsHash = paramsHash;
            RequestInfo(requestId, requestType, msg.sender, msg.value, callbackAddr, paramsHash, requestData);
            RequestLog(this, 1);
            return requestId;
        }
    }

    function deliver(uint64 requestId, bytes32 paramsHash, bytes32 respData) public {
        uint fee = requests[requestId].fee;
        if (msg.sender != SGX_ADDRESS
                || requests[requestId].requester == 0
                || fee == DELIVERED_FEE_FLAG) {
            DeliverInfo(requestId, fee, tx.gasprice, msg.gas, 0, paramsHash, respData);
            DeliverLog(msg.gas, -1);
            return;
        } else if (requests[requestId].paramsHash != paramsHash) {
            DeliverInfo(requestId, fee, tx.gasprice, msg.gas, 0, paramsHash, respData);
            DeliverLog(msg.gas, -4);
            return;
        } else if (requests[requestId].fee == CANCELLED_FEE_FLAG) {
            DeliverLog(msg.gas, 1);
            if (! SGX_ADDRESS.send(CANCELLATION_FEE)){
                throw;
            }
            requests[requestId].fee = DELIVERED_FEE_FLAG;
            DeliverLog(msg.gas, int(CANCELLATION_FEE));
            return;
        }

        DeliverLog(msg.gas, 8);
        if (!SGX_ADDRESS.send(fee)) {
            throw;
        }
        requests[requestId].fee = DELIVERED_FEE_FLAG;
        DeliverLog(msg.gas, 16);

        uint callbackGas = (fee - MIN_FEE) / tx.gasprice;
        DeliverInfo(requestId, fee, tx.gasprice, msg.gas, callbackGas, paramsHash, respData);
        bool deliverSuccess = requests[requestId].callbackAddr.call.gas(callbackGas)(requests[requestId].callbackFID, requestId, respData);
        if (deliverSuccess) {
            DeliverLog(msg.gas, 32);
        } else {
            DeliverLog(msg.gas, -2);
        }
    }

    function cancel(uint64 requestId) public returns (bool) {
        // Compute the gas reimbursement necessary if TownCrier attemts to respond to this request later.
        Cancel(requestId, msg.sender, requests[requestId].requester, int(fee));

        // If the request has was sent by this user and has money left on it, then cancel it.
        uint fee = requests[requestId].fee;
        if (requests[requestId].requester == msg.sender && fee > CANCELLATION_FEE) {
            if (msg.sender.send(fee - CANCELLATION_FEE)) {
                throw;
            }
            requests[requestId].fee = CANCELLED_FEE_FLAG;
            Cancel(requestId, msg.sender, requests[requestId].requester, int(CANCELLED_FEE_FLAG));
            return true;
        } else {
            Cancel(requestId, msg.sender, requests[requestId].requester, -int(CANCELLED_FEE_FLAG));
            return false;
        }
    }
}
