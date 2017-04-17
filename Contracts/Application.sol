contract Application {
    event Request(int64 requestId, address requester, uint dataLength, bytes32[] data); // log for requests
    event Response(int64 requestId, address requester, uint64 error, uint data); // log for responses
    event Cancel(uint64 requestId, address requester, bool success); // log for cancellations

    uint constant MIN_GAS = 30000 + 20000; // minimum gas required for a query
    uint constant GAS_PRICE = 5 * 10 ** 10;
    uint constant TC_FEE = MIN_GAS * GAS_PRICE;
    uint constant CANCELLATION_FEE = 25000 * GAS_PRICE;

    bytes4 constant TC_CALLBACK_FID = bytes4(sha3("response(uint64,uint64,bytes32)"));

    TownCrier public TC_CONTRACT;
    address owner; // creator of this contract
    address[2**64] requesters;
    uint[2**64] fee;

    function() public payable {} // must be payable

    function Application(TownCrier tcCont) public {
        TC_CONTRACT = tcCont; // storing the address of the TownCrier Contract
        owner = msg.sender;
    }

    function request(uint8 requestType, bytes32[] requestData) public payable {
        if (msg.value < TC_FEE) {
            // The requester paid less fee than required.
            // Reject the request and refund the requester.
            if (!msg.sender.send(msg.value)) {
                throw;
            }
            Request(-1, msg.sender, requestData.length, requestData);
            return;
        }

        uint64 requestId = TC_CONTRACT.request.value(msg.value)(requestType, this, TC_CALLBACK_FID, 0, requestData); // calling request() in the TownCrier Contract
        if (requestId == 0) {
            // The request fails.
            // Refund the requester.
            if (!msg.sender.send(msg.value)) { 
                throw;
            }
            Request(-2, msg.sender, requestData.length, requestData);
            return;
        }
        
        // Successfully sent a request to TC.
        // Record the request.
        requesters[requestId] = msg.sender;
        fee[requestId] = msg.value;
        Request(int64(requestId), msg.sender, requestData.length, requestData);
    }

    function response(uint64 requestId, uint64 error, bytes32 respData) public {
        if (msg.sender != address(TC_CONTRACT)) {
            // If the message sender is not the TownCrier Contract,
            // discard the response.
            Response(-1, msg.sender, 0, 0); 
            return;
        }

        address requester = requesters[requestId];
        requesters[requestId] = 0; // set the request as responded

        if (error < 2) {
            Response(int64(requestId), requester, error, uint(respData));
        } else {
            requester.send(fee[requestId]); // refund the requester if error exists in TC
            Response(int64(requestId), msg.sender, error, 0);
        }
    }

    function cancel(uint64 requestId) public {
        if (requestId == 0 || requesters[requestId] != msg.sender) {
            // If the requestId is invalid or the requester is not the message sender,
            // cancellation fails.
            Cancel(requestId, msg.sender, false);
            return;
        }

        bool tcCancel = TC_CONTRACT.cancel(requestId); // calling cancel() in the TownCrier Contract
        if (tcCancel) {
            // Successfully cancels the request in the TownCrier Contract,
            // then refund the requester with (fee - cancellation fee).
            requesters[requestId] = 0;
            if (!msg.sender.send(fee[requestId] - CANCELLATION_FEE)) {
                Cancel(requestId, msg.sender, false);
                throw;
            }
            Cancel(requestId, msg.sender, true);
        } else {
            // Cancellation in the TownCrier Contract fails.
            Cancel (requestId, msg.sender, false);
        }
    }
}


