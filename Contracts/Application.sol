contract Application {
    event Request(int64 requestId, address requester, uint dataLength, bytes32[] data);
    event Response(int64 requestId, address requester, uint64 error, uint data);
    event Cancel(uint64 requestId, address requester, bool success);

    uint constant MIN_GAS = 30000 + 20000;
    uint constant GAS_PRICE = 5 * 10 ** 10;
    uint constant TC_FEE = MIN_GAS * GAS_PRICE;
    uint constant CANCELLATION_FEE = 25000 * GAS_PRICE;

    bytes4 constant TC_CALLBACK_FID = bytes4(sha3("response(uint64,uint64,bytes32)"));

    TownCrier public TC_CONTRACT;
    address owner;
    address[2**64] requesters;
    uint[2**64] fee;

    function() public payable {} // must be payable

    function Application(TownCrier tcCont) public {
        TC_CONTRACT = tcCont;
        owner = msg.sender;
    }

    function request(uint8 requestType, bytes32[] requestData) public payable {
        if (msg.value < TC_FEE) {
            if (!msg.sender.send(msg.value)) {
                throw;
            }
            Request(-1, msg.sender, requestData.length, requestData);
            return;
        }

        uint64 requestId = TC_CONTRACT.request.value(msg.value)(requestType, this, TC_CALLBACK_FID, 0, requestData);
        if (requestId == 0) {
            if (!msg.sender.send(msg.value)) { 
                throw;
            }
            Request(-2, msg.sender, requestData.length, requestData);
            return;
        }

        requesters[requestId] = msg.sender;
        fee[requestId] = msg.value;
        Request(int64(requestId), msg.sender, requestData.length, requestData);
    }

    function response(uint64 requestId, uint64 error, bytes32 respData) public {
        if (msg.sender != address(TC_CONTRACT)) {
            Response(-1, msg.sender, 0, 0); 
            return;
        }

        address requester = requesters[requestId];
        requesters[requestId] = 0;

        if (error < 2) {
            Response(int64(requestId), requester, error, uint(respData));
        } else {
            if (!requester.send(fee[requestId])) {
                Response(-2, msg.sender, error, 0);
                throw;
            }
            Response(int64(requestId), msg.sender, error, 0);
        }
    }

    function cancel(uint64 requestId) public {
        if (requestId == 0 || requesters[requestId] != msg.sender) {
            Cancel(requestId, msg.sender, false);
            return;
        }

        bool tcCancel = TC_CONTRACT.cancel(requestId);
        if (tcCancel) {
            if (!msg.sender.send(fee[requestId] - CANCELLATION_FEE)) {
                Cancel(requestId, msg.sender, false);
                throw;
            }
            Cancel(requestId, msg.sender, true);
            requesters[requestId] = 0;
        } else {
            Cancel (requestId, msg.sender, false);
        }
    }
}


