contract TownCrier {
    struct Request {
        uint8 requestType;
        address requester;
        uint fee;
        address callbackAddr;
        bytes4 callbackFID;
        bytes32[] requestData;
    }

    event RequestLog(uint gasLeft, int16 flag);
    event RequestInfo(uint64 id, uint8 requestType, address requester, uint fee, address callbackAddr, uint reqLen, bytes32[] requestData);
    event DeliverLog(uint gasLeft, int flag);
    event DeliverInfo(uint64 requestId, uint fee, uint gasPrice, uint gasLeft, uint callbackGas, uint requestLen, bytes32 response);
//    event DeliverSig(uint8 v, bytes32 r, bytes32 s, address recoveredAddr);
    event Cancel(uint64 requestId, address canceller, address requester, int flag);

    address constant SGX_ADDRESS = 0x9d10ea5ad51e1af69cd8d4dcfa60f577818607b2;

    uint constant GAS_PRICE = 5 * 10**10;
    uint constant MAX_FEE = (3 * 10**6) * GAS_PRICE;

    uint constant MIN_FEE_BASE = 30000 * GAS_PRICE;
    uint constant FEE_PER_32BYTES = 2600 * GAS_PRICE;

    uint constant CANCELLATION_FEE_BASE = 23000 * GAS_PRICE;
    uint constant CANCELLATION_FEE_PER_32BYTES = 2200 * GAS_PRICE;

    uint constant CANCELLED_FEE_FLAG = 1;
    uint constant DELIVERED_FEE_FLAG = 0;

    uint64 requestCnt = 0;
    Request[2**64] requests;

    function request(uint8 requestType, address callbackAddr, bytes4 callbackFID, bytes32[] requestData) public returns (uint64) {
        RequestLog(msg.gas, 0);
        uint minFee = MIN_FEE_BASE + (FEE_PER_32BYTES * requestData.length);
        if (msg.value < minFee || msg.value > MAX_FEE) {
            RequestInfo(0, requestType, msg.sender, msg.value, callbackAddr, requestData.length, requestData);
            RequestLog(msg.gas, -1);
            return 0;
        } else {
            requestCnt++;
            uint64 requestId = requestCnt;

            requests[requestId].requestType = requestType;
            requests[requestId].requester = msg.sender;
            requests[requestId].fee = msg.value;
            requests[requestId].callbackAddr = callbackAddr;
            requests[requestId].callbackFID = callbackFID;
            requests[requestId].requestData = requestData;
            RequestInfo(requestId, requestType, msg.sender, msg.value, callbackAddr, requestData.length, requests[requestId].requestData);
            RequestLog(msg.gas, 1);
            return requestId;
        }
    }

//    function deliver(uint64 requestId, uint8 requestType, bytes requestData, bytes32 respData, uint8 v, bytes32 r, bytes32 s) public {
    function deliver(uint64 requestId, uint8 requestType, bytes32[] requestData, bytes32 respData) public {
        bytes32[] storage storedRequestData = requests[requestId].requestData;
        uint fee = requests[requestId].fee;
//        address signer = ecrecover(sha3(requestId, requestType, requestData, respData), v, r, s);
//        DeliverSig(v, r, s, signer);
//        if (signer != SGX_ADDRESS
        if (msg.sender != SGX_ADDRESS
                || requests[requestId].requester == 0
                || requests[requestId].requestType != requestType
                || fee == DELIVERED_FEE_FLAG) {
            DeliverInfo(requestId, fee, tx.gasprice, msg.gas, 0, requestData.length, respData);
            DeliverLog(msg.gas, -1);
            return;
        } else if (storedRequestData.length != requestData.length) {
            DeliverInfo(requestId, fee, tx.gasprice, msg.gas, 0, requestData.length, respData);
            DeliverLog(msg.gas, -4);
            return;
        } else if (requests[requestId].fee == CANCELLED_FEE_FLAG) {
            DeliverLog(msg.gas, 1);
            uint cancellationFee = CANCELLATION_FEE_BASE + (requestData.length * CANCELLATION_FEE_PER_32BYTES);
            SGX_ADDRESS.send(cancellationFee);
            requests[requestId].fee = DELIVERED_FEE_FLAG;
            DeliverLog(msg.gas, int(cancellationFee));
            return;
        }

        DeliverLog(msg.gas, 4);
        for (uint16 i = 0; i < requestData.length; i++) {
            if (storedRequestData[i] != requestData[i]) {
                DeliverLog(msg.gas, int16(i));
                return;
            }
        }

        DeliverLog(msg.gas, 8);
        SGX_ADDRESS.send(fee);
        requests[requestId].fee = DELIVERED_FEE_FLAG;
        DeliverLog(msg.gas, 16);

        uint callbackGas = (fee - (MIN_FEE_BASE + FEE_PER_32BYTES * requestData.length)) / tx.gasprice;
        DeliverInfo(requestId, fee, tx.gasprice, msg.gas, callbackGas, requestData.length, respData);
        bool deliverSuccess = requests[requestId].callbackAddr.call.gas(callbackGas)(requests[requestId].callbackFID, requestId, respData);
        if (deliverSuccess) {
            DeliverLog(msg.gas, 32);
        } else {
            DeliverLog(msg.gas, -2);
        }
    }

    function cancel(uint64 requestId) public returns (bool) {
        // Compute the gas reimbursement necessary if TownCrier attemts to respond to this request later.
        uint cancellationFee = CANCELLATION_FEE_BASE + (requests[requestId].requestData.length * CANCELLATION_FEE_PER_32BYTES);
        Cancel(requestId, msg.sender, requests[requestId].requester, int(fee));

        // If the request has was sent by this user and has money left on it, then cancel it.
        uint fee = requests[requestId].fee;
        if (requests[requestId].requester == msg.sender && fee > cancellationFee) {
            msg.sender.send(fee - cancellationFee);
            requests[requestId].fee = CANCELLED_FEE_FLAG;
            Cancel(requestId, msg.sender, requests[requestId].requester, int(CANCELLED_FEE_FLAG));
            return true;
        } else {
            Cancel(requestId, msg.sender, requests[requestId].requester, -int(CANCELLED_FEE_FLAG));
            return false;
        }
    }
}

