pragma solidity ^0.4.9;

contract TownCrier {
    struct Request { // the data structure for each request
        address requester; // the address of the requester
        uint fee; // the amount of wei the requester pays for the request
        address callbackAddr; // the address of the contract to call for delivering response
        bytes4 callbackFID; // the specification of the callback function
        bytes32 paramsHash; // the hash of the request parameters
    }

    event RequestInfo(uint64 id, uint8 requestType, address requester, uint fee, address callbackAddr, bytes32 paramsHash, uint timestamp, bytes32[] requestData); // log of requests, the Town Crier server watches this event and processes requests
    event DeliverInfo(uint64 requestId, uint fee, uint gasPrice, uint gasLeft, uint callbackGas, bytes32 paramsHash, uint64 error, bytes32 respData); // log of responses
    event Cancel(uint64 requestId, address canceller, address requester, uint fee, int flag); // log of cancellations

    address constant SGX_ADDRESS = 0x89B44e4d3c81EDE05D0f5de8d1a68F754D73d997; // address of the SGX account

    uint public constant GAS_PRICE = 5 * 10**10;
    uint public constant MAX_GAS = 295 * 10 ** 4;
    uint public constant MIN_FEE = 30000 * GAS_PRICE; // least fee required for the requester to pay such that SGX could call deliver() to send a response
    uint public constant CANCELLATION_FEE = 25000 * GAS_PRICE; // charged when the requester cancels a request that is not responded

    uint constant CANCELLED_FEE_FLAG = 1;
    uint constant DELIVERED_FEE_FLAG = 0;

    uint64 requestCnt;
    Request[2**64] requests;

    // Contracts that receive Ether but do not define a fallback function throw
    // an exception, sending back the Ether (this was different before Solidity
    // v0.4.0). So if you want your contract to receive Ether, you have to
    // implement a fallback function.
    function () {}

    function TownCrier() public {
        // Start request IDs at 1 for two reasons:
        //   1. We can use 0 to denote an invalid request (ids are unsigned)
        //   2. Storage is more expensive when changing something from zero to non-zero,
        //      so this means the first request isn't randomly more expensive.
        requestCnt = 1;
        requests[0].requester = msg.sender;
    }

    function request(uint8 requestType, address callbackAddr, bytes4 callbackFID, uint timestamp, bytes32[] requestData) public payable returns (uint64) {
        RequestLog(this, 0);
        if (msg.value < MIN_FEE) {
            // If the amount of ether sent by the requester is too little or 
            // too much, refund the requester and discard the request.
            RequestLog(this, -1);
            if (!msg.sender.send(msg.value)) {
                RequestLog(this, -2);
                throw;
            }
            return 0;
        } else {
            // Record the request.
            uint64 requestId = requestCnt;
            requestCnt++;

            bytes32 paramsHash = sha3(requestType, requestData);
            requests[requestId].requester = msg.sender;
            requests[requestId].fee = msg.value;
            requests[requestId].callbackAddr = callbackAddr;
            requests[requestId].callbackFID = callbackFID;
            requests[requestId].paramsHash = paramsHash;

            // Log the request for the Town Crier server to process.
            RequestInfo(requestId, requestType, msg.sender, msg.value, callbackAddr, paramsHash, timestamp, requestData);
            RequestLog(this, 0);
            return requestId;
        }
    }

    function deliver(uint64 requestId, bytes32 paramsHash, uint64 error, bytes32 respData) public {
        if (msg.sender != SGX_ADDRESS ||
                requestId <= 0 ||
                requests[requestId].requester == 0 ||
                requests[requestId].fee == DELIVERED_FEE_FLAG) {
            // If the response is not delivered by the SGX account or the 
            // request has already been responded to, discard the response.
            DeliverLog(msg.gas, -1);
            return;
        }
        
        uint fee = requests[requestId].fee;
        if (requests[requestId].paramsHash != paramsHash) {
            // If the hash of request parameters in the response is not 
            // correct, discard the response for security concern.
            DeliverLog(msg.gas, -4);
            return;
        } else if (fee == CANCELLED_FEE_FLAG) {
            // If the request is cancelled by the requester, cancellation 
            // fee goes to the SGX account and set the request as having
            // been responded to.
            DeliverLog(msg.gas, 1);
            if (!SGX_ADDRESS.send(CANCELLATION_FEE)){
                DeliverLog(msg.gas, -8);
                throw;
            }
            requests[requestId].fee = DELIVERED_FEE_FLAG;
            DeliverLog(msg.gas, int(CANCELLATION_FEE));
            return;
        }

        DeliverLog(msg.gas, 8);
        requests[requestId].fee = DELIVERED_FEE_FLAG;
        
        if (error < 2) {
            if (!SGX_ADDRESS.send(fee)) { // send the fee to the SGX account for its delivering
                DeliverLog(msg.gas, -16);
                throw;
            }
        } else {
            if (!requests[requestId].requester.send(fee)) {
                DeliverLog(msg.gas, -32);
                throw;
            }
        }

        DeliverLog(msg.gas, 16);

        uint callbackGas = (fee - MIN_FEE) / tx.gasprice; // gas left for the callback function
        if (callbackGas > MAX_GAS) {
            callbackGas = MAX_GAS;
        }

        DeliverInfo(requestId, fee, tx.gasprice, msg.gas, callbackGas, paramsHash, error, respData); // log the response information
        
        bool deliverSuccess = requests[requestId].callbackAddr.call.gas(callbackGas)(requests[requestId].callbackFID, requestId, error, respData); // call the callback function in the application contract
        if (deliverSuccess) {
            DeliverLog(msg.gas, 32);
        } else {
            DeliverLog(msg.gas, -2);
        }
    }

    function cancel(uint64 requestId) public returns (bool) {
        uint fee = requests[requestId].fee;
        if (requests[requestId].requester == msg.sender && fee >= CANCELLATION_FEE) {
            // If the request was sent by this user and has money left on it,
            // then cancel it.
            requests[requestId].fee = CANCELLED_FEE_FLAG;
            if (!msg.sender.send(fee - CANCELLATION_FEE)) {
                Cancel(requestId, msg.sender, requests[requestId].requester, fee - CANCELLATION_FEE, -2);
                throw;
            }
            Cancel(requestId, msg.sender, requests[requestId].requester, requests[requestId].fee, 1);
            return true;
        } else {
            Cancel(requestId, msg.sender, requests[requestId].requester, fee, -1);
            return false;
        }
    }
}

