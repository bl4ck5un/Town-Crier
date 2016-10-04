contract TownCrierVote {
    struct Request {
        uint fee;
        address callbackAddr;
        bytes4 callbackFID;
        bytes32 paramsHash;
        uint8 responseCnt;
        bytes32 response0;
        bytes32 response1;
    }

    event RequestLog(uint gasLeft, int16 flag);
    event RequestInfo(uint64 id, uint fee, bytes32 paramsHash, uint8 requestType, uint timestamp, bytes32[] requestData);
    event DeliverLog(uint gasLeft, address sender, int flag);
    event DeliverInfo(uint64 requestId, uint fee, uint gasPrice, uint gasLeft, uint callbackGas, bytes32 paramsHash, bytes32 response);
    event Cancel(uint64 requestId, address canceller, address requester, int flag);

    address constant SGX1_ADDRESS = 0x9d10ea5ad51e1af69cd8d4dcfa60f577818607b2;
    address constant SGX2_ADDRESS = 0xec0907991a5560bfe9ae853adaadb8f7578670af;
    address constant SGX3_ADDRESS = 0xed0013df219cab5faae57e579b3c5901d40d43ff;

    uint constant GAS_PRICE = 5 * 10**10;

    uint constant MAX_FEE = (3 * 10**6) * GAS_PRICE;

    uint constant FIRST_DELIVER_COST = 74000 * GAS_PRICE;
    uint constant SECOND_DELIVER_COST = 59000 * GAS_PRICE;
    uint constant LAST_DELIVER_COST = 35500 * GAS_PRICE;
    uint constant MIN_FEE = FIRST_DELIVER_COST + SECOND_DELIVER_COST + LAST_DELIVER_COST;

    uint64 requestCnt;
    Request[2**64] requests;

    function TownCrierVote() public {
        // Start request IDs at 1 for two reasons:
        //   1. We can use 0 to denote an invalid request (ids are unsigned)
        //   2. Storage is more expensive when changing something from zero to non-zero,
        //      so this means the first request isn't randomly more expensive.
        // Reason 2 also makes us want to initialize an array value somewhere.
        requestCnt = 1;
        requests[0].requester = msg.sender;
    }

    function request(uint8 requestType, address callbackAddr, bytes4 callbackFID, bytes32[] requestData) public returns (uint64) {
//    function request(uint8 requestType, address callbackAddr, bytes32[] requestData) public returns (uint64) {
//        bytes4 callbackFID = bytes4(0);
        RequestLog(msg.gas, 0);
        if (msg.value < MIN_FEE || msg.value > MAX_FEE) {
            RequestInfo(0, msg.value, 0, requestType, block.timestamp, requestData);
            RequestLog(msg.gas, -1);
            return 0;
        } else {
            uint64 requestId = requestCnt;
            requestCnt++;

            uint timestamp = block.timestamp;
            bytes32 paramsHash = sha3(requestType, timestamp, requestData);
            requests[requestId].fee = msg.value;
            requests[requestId].callbackAddr = callbackAddr;
            requests[requestId].callbackFID = callbackFID;
            requests[requestId].paramsHash = paramsHash;
            requests[requestId].responseCnt = 0;

            RequestInfo(requestId, msg.value, paramsHash, requestType, timestamp, requestData);
            RequestLog(msg.gas, 1);
            return requestId;
        }
    }

    function deliver(uint64 requestId, bytes32 paramsHash, bytes32 respData) public {
        uint fee = requests[requestId].fee;
        if (fee == 0) {
            DeliverInfo(requestId, fee, tx.gasprice, msg.gas, 0, paramsHash, respData);
            DeliverLog(msg.gas, msg.sender, -1);
            return;
        } else if (msg.sender != SGX1_ADDRESS
                && msg.sender != SGX2_ADDRESS
                && msg.sender != SGX3_ADDRESS) {
            DeliverInfo(requestId, fee, tx.gasprice, msg.gas, 0, paramsHash, respData);
            DeliverLog(msg.gas, msg.sender, -2);
            return;
        } else if (requests[requestId].paramsHash != paramsHash) {
            DeliverInfo(requestId, fee, tx.gasprice, msg.gas, 0, paramsHash, respData);
            DeliverLog(msg.gas, msg.sender, -3);
            return;
        }

        uint8 responseCnt = requests[requestId].responseCnt;
        if (responseCnt == 0) {
            requests[requestId].response0 = respData;
            requests[requestId].responseCnt++;
            DeliverLog(msg.gas, msg.sender, 1);
            msg.sender.send(FIRST_DELIVER_COST);
            return;
        } else if (responseCnt == 1) {
            requests[requestId].response1 = respData;
            requests[requestId].responseCnt++;
            DeliverLog(msg.gas, msg.sender, 2);
            msg.sender.send(SECOND_DELIVER_COST);
            return;
        }

        DeliverLog(msg.gas, msg.sender, 3);

        bytes32 response0 = requests[requestId].response0;
        bytes32 response1 = requests[requestId].response1;
        bytes32 response;
        if (respData == response0 || respData == response1) {
            response = respData;
        } else if (response0 == response1) {
            response = response1;
        } else {
            // None of the options won, so refund gas costs and abort without delivering data.
            DeliverLog(msg.gas, 0, -8);
            msg.sender.send(LAST_DELIVER_COST);
            return;
        }

        DeliverLog(msg.gas, 0, 4);
        msg.sender.send(fee - (FIRST_DELIVER_COST + SECOND_DELIVER_COST));
        requests[requestId].fee = 0;

        uint callbackGas = (fee - MIN_FEE) / tx.gasprice;
        DeliverInfo(requestId, fee, tx.gasprice, msg.gas, callbackGas, paramsHash, response);
        DeliverLog(msg.gas, 0, 8);
        bool deliverSuccess = requests[requestId].callbackAddr.call.gas(callbackGas)(requests[requestId].callbackFID, requestId, response);
        if (deliverSuccess) {
            DeliverLog(msg.gas, 0, 32);
        } else {
            DeliverLog(msg.gas, 0, -2);
        }
    }
}

