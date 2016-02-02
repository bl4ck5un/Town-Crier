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
    event DeliverLog(uint gasLeft, int16 flag);
    event DeliverInfo(uint64 requestId, uint fee, uint gasPrice, uint callbackGas, uint gasLeft, bytes32 response, uint respUint);

    address constant SGX_ADDRESS = 0x9d10ea5ad51e1af69cd8d4dcfa60f577818607b2;
    uint constant MIN_FEE_BASE = 64000 * (5 * 10**10);
    uint constant FEE_PER_BYTE = 650 * (5 * 10**10);
    uint constant MAX_FEE = (3 * 10**6) * (5 * 10**10);

    uint constant CANCELLED_FEE_VALUE = 1;
    uint constant DELIVERED_FEE_VALUE = 0;

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

    function deliver(uint64 requestId, uint8 requestType, bytes requestData, bytes32 respData) public {
        bytes4 callbackFID = requests[requestId].callbackFID;
        uint minFee = MIN_FEE_BASE + (FEE_PER_BYTE * requestData.length);
        uint maxCallbackGas = msg.gas - (minFee / tx.gasprice);
        uint fee = requests[requestId].fee;
        bytes storedRequestData = requests[requestId].requestData;
        DeliverInfo(requestId, fee, tx.gasprice, maxCallbackGas, msg.gas, respData, uint(respData));
        if (msg.sender != SGX_ADDRESS) {
            DeliverLog(msg.gas, -10);
            return;
        } else if (requests[requestId].requester == 0) {
            DeliverLog(msg.gas, -11);
            return;
        } else if (requests[requestId].requestType != requestType) {
            DeliverLog(msg.gas, -12);
            return;
        } else if (storedRequestData.length != requestData.length) {
            DeliverLog(msg.gas, -13);
            return;
        } else if (fee == DELIVERED_FEE_VALUE) {
            DeliverLog(msg.gas, -14);
            return;
        }
        DeliverLog(msg.gas, 513);
        for (uint16 i = 0; i < requestData.length; i++) {
            if (storedRequestData[i] != requestData[i]) {
                DeliverLog(msg.gas, int16(i));
                return;
            }
        }

        DeliverLog(msg.gas, 1000);
        if (fee == CANCELLED_FEE_VALUE) {
            DeliverLog(msg.gas, -1);
            SGX_ADDRESS.send(minFee);
            requests[requestId].fee = DELIVERED_FEE_VALUE;
            DeliverLog(msg.gas, -2);
            return;
        }

        uint gasForCallback = (fee - minFee) / tx.gasprice;
        if (gasForCallback > maxCallbackGas)
            gasForCallback = maxCallbackGas;
        DeliverInfo(requestId, fee, tx.gasprice, gasForCallback, msg.gas, bytes4(respData), uint(respData));
        bool deliverSuccess = requests[requestId].callbackAddr.call.gas(gasForCallback)(requests[requestId].callbackFID, requestId, respData);
        if (deliverSuccess) {
            DeliverLog(msg.gas, 3000);
        } else {
            DeliverLog(msg.gas, -3);
        }

        SGX_ADDRESS.send(fee);
        requests[requestId].fee = DELIVERED_FEE_VALUE;
        DeliverLog(msg.gas, 4000);
    }

    function cancel(uint64 requestId) public {
        uint fee = requests[requestId].fee;
        uint minFee = MIN_FEE_BASE + (FEE_PER_BYTE * requests[requestId].requestData.length);
        if (requests[requestId].requester == msg.sender && fee >= minFee) {
            msg.sender.send(fee - minFee);
            requests[requestId].fee = CANCELLED_FEE_VALUE;
        }
    }
}

contract FlightInsurance {
    event Insure(address sender, uint dataLength, string data, int72 requestId);
    event PaymentLog(int flag);
    event PaymentInfo(address payee, uint payeeBalance, uint gasRemaining, uint64 requestId, uint delay, uint amount);

    uint constant TC_FEE = (65000 + (650 * 256) + 21000) * 5 * 10**10;
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
}

