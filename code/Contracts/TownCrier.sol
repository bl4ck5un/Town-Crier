contract TownCrier {
    struct Request {
        uint8 requestType;
        address requester;
        uint fee;
        address callbackAddr;
        bytes4 callbackFID;
        bytes requestData;
    }

    event RequestLog(uint gasLeft, int16 flag);
    event RequestInfo(uint64 id, uint8 requestType, address requester, uint fee, address callbackAddr, uint reqLen, string requestData);
    event DeliverLog(uint gasLeft, int flag);
    event DeliverInfo(uint64 requestId, uint fee, uint gasPrice, uint callbackGas, uint gasLeft, bytes32 response);
//    event DeliverSig(uint8 v, bytes32 r, bytes32 s, address recoveredAddr);
    event Cancel(uint64 requestId, address canceller, address requester, int flag);

    address constant SGX_ADDRESS = 0x9d10ea5ad51e1af69cd8d4dcfa60f577818607b2;

    uint constant GAS_PRICE = 5 * 10**10;
    uint constant MAX_FEE = (3 * 10**6) * GAS_PRICE;

    uint constant MIN_FEE_BASE = 64000 * GAS_PRICE;
    uint constant FEE_PER_BYTE = 650 * GAS_PRICE;

    uint constant CANCELLATION_FEE_BASE = 23500 * GAS_PRICE;
    uint constant CANCELLATION_FEE_PER_BYTE = 71 * GAS_PRICE;

    uint constant CANCELLED_FEE_FLAG = 1;
    uint constant DELIVERED_FEE_FLAG = 0;

    uint64 requestCnt = 0;
    Request[2**64] requests;

    function request(uint8 requestType, address callbackAddr, bytes4 callbackFID, bytes requestData) public returns (uint64) {
        RequestLog(msg.gas, 0);
        uint minFee = MIN_FEE_BASE + (FEE_PER_BYTE * requestData.length);
        if (msg.value < minFee || msg.value > MAX_FEE) {
            RequestInfo(0, requestType, msg.sender, msg.value, callbackAddr, requestData.length, string(requestData));
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
            requests[requestId].requestData.length = requestData.length;
            requests[requestId].requestData = requestData;
            RequestInfo(requestId, requestType, msg.sender, msg.value, callbackAddr, requestData.length, string(requestData));
            RequestLog(msg.gas, 1);
            return requestId;
        }
    }

//    function deliver(uint64 requestId, uint8 requestType, bytes requestData, bytes32 respData, uint8 v, bytes32 r, bytes32 s) public {
    function deliver(uint64 requestId, uint8 requestType, bytes requestData, bytes32 respData) public {
        bytes storedRequestData = requests[requestId].requestData;
//        address signer = ecrecover(sha3(requestId, requestType, requestData, respData), v, r, s);
//        DeliverSig(v, r, s, signer);
//        if (signer != SGX_ADDRESS
        if (msg.sender != SGX_ADDRESS
                || requests[requestId].requester == 0
                || requests[requestId].requestType != requestType
                || storedRequestData.length != requestData.length
                || requests[requestId].fee == DELIVERED_FEE_FLAG) {
            DeliverLog(msg.gas, -1);
            return;
        } else if (requests[requestId].fee == CANCELLED_FEE_FLAG) {
            DeliverLog(msg.gas, 1);
            uint cancellationFee = CANCELLATION_FEE_BASE + (requestData.length * CANCELLATION_FEE_PER_BYTE);
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

        uint minFee = MIN_FEE_BASE + (FEE_PER_BYTE * requestData.length);
        uint maxCallbackGas = msg.gas - (minFee / tx.gasprice);
        DeliverInfo(requestId, requests[requestId].fee, tx.gasprice, maxCallbackGas, msg.gas, respData);

        uint gasForCallback = (requests[requestId].fee - minFee) / tx.gasprice;
        if (gasForCallback > maxCallbackGas)
            gasForCallback = maxCallbackGas;
        DeliverInfo(requestId, requests[requestId].fee, tx.gasprice, gasForCallback, msg.gas, respData);
        bool deliverSuccess = requests[requestId].callbackAddr.call.gas(gasForCallback)(requests[requestId].callbackFID, requestId, respData);
        if (deliverSuccess) {
            DeliverLog(msg.gas, 16);
        } else {
            DeliverLog(msg.gas, -2);
        }

        SGX_ADDRESS.send(requests[requestId].fee);
        requests[requestId].fee = DELIVERED_FEE_FLAG;
        DeliverLog(msg.gas, 32);
    }

    function cancel(uint64 requestId) public returns (bool) {
        // Compute the gas reimbursement necessary if TownCrier attemts to respond to this request later.
        uint cancellationFee = CANCELLATION_FEE_BASE + (requests[requestId].requestData.length * CANCELLATION_FEE_PER_BYTE);
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

contract FlightInsurance {
    event Insure(address sender, uint dataLength, string data, int72 requestId);
    event PaymentLog(int flag);
    event PaymentInfo(address payee, uint payeeBalance, uint gasRemaining, uint64 requestId, uint delay, uint amount);
    event FlightCancel(address canceller, address requester, bool success);

    uint constant TC_FEE = (65000 + (650 * 1024) + 21000) * 5 * 10**10;
    uint constant FEE = 5 * 10**18;
    uint constant PAYOUT = 10**20;
    uint32 constant PAYOUT_DELAY = 30;

    TownCrier TC_CONTRACT;
    address[2**64] requesters;

    function FlightInsurance(TownCrier tcCont) public {
        TC_CONTRACT = tcCont;
    }

    function insure(bytes encryptedFlightInfo) public {
        if (msg.value != FEE) {
            Insure(msg.sender, encryptedFlightInfo.length, string(encryptedFlightInfo), -1);
            return;
        }

        Insure(msg.sender, encryptedFlightInfo.length, string(encryptedFlightInfo), -2);
        bytes4 callbackFID = 0x3d622256; // bytes4(sha3("pay(uint64,bytes32)"));
        uint64 requestId = TC_CONTRACT.request.value(TC_FEE)(0, this, callbackFID, encryptedFlightInfo);
        Insure(msg.sender, encryptedFlightInfo.length, string(encryptedFlightInfo), -3);
        requesters[requestId] = msg.sender;
        Insure(msg.sender, encryptedFlightInfo.length, string(encryptedFlightInfo), int72(requestId));
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

