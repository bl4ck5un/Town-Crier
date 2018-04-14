# Get Started with Town Crier
For smart contracts on blockchain systems, it's currently difficult to get real-world data because there's no authentication for any data fed back to the blockchain. The Town Crier system addresses this problem by using trusted hardware such as Intel SGX to provide an authenticated data feed. When using the data fed by the Town Crier system in smart contracts, people don't have to trust any other party except SGX and the website where the data is scraped.
In order to get certain information, a smart contract may send a request to the Town Crier server. Then the Town Crier server will fetch required data from a trusted website and send it back to the contract. The parsing of a request and the generation of a response with SGX's signature are processed on an SGX enclave inside the Town Crier server. The fetching of required data is done via TLS connection between the enclave and the website. SGX guarantees confidentiality and integrity, which means the status of a program running on an SGX enclave cannot be either revealed or modified by an adversary. TLS provides a secure channel for the communication of two parties, which also guarantees confidentiality and integrity. For more details of Town Crier system and its security guarantees, please look at our paper [Town Crier: An Authenticated Data Feed for Smart Contracts].

## Understand the ```TownCrier``` Contract
The ```TownCrier``` contract provides uniform interfaces for different application contracts to send requests to the Town Crier server and for the Town Crier server to respond to applications. It consists of the following three functions.
* ```request(uint8 requestType, address callbackAddr, bytes4 callbackFID, uint256 timeout, bytes32[] requestData) public payable returns(uint64)```
When called by an application contract, ```request()``` records the information of the request, including
    - ```requestType```: the type of the request, for the Town Crier server to process it accordingly.
    - ```callbackAddr```: the address of the application contract, used for response.
    - ```callbackFID```: the specification for the callback function in application contract.
    - ```timestamp```: parameter required for a potential feature. Currently Town Crier server could only send response back once discover the request and successfully fetch the data. We expect to make it support fetching data at a certain time and then responding. You'll see why we need this in the Flight Insurance example. It is not used in the TownCrier server for now. 
    - ```requestData```: data required to specifying the request.

    After a request is logged by event ```RequestInfo()```, the function will return ```requestId``` which is assigned to this request such that the requester could use it to look up the logs for response or status of the request.
    The Town Crier server will watch events and process the request once find one ```RequestInfo()```. 
    Requesters should pay for the gas cost for the TownCrier server to send the response to the blockchain afterwards. ```msg.value``` is the amount of wei a requester pays and recorded as ```Request.fee```.
* ```deliver(uint64 requestId, bytes32 paramsHash, uing64 error, bytes32 respData) public```
After fetching data and generating the response for the request with ```requestId```, the Town Crier server calls function ```deliver()```. ```deliver()``` verifies that the function call is made by SGX and that the hash of request parameters is correct, and then it will call the callback function of the application contract to transmit the response. The response includes ```error``` and ```respData```. The application contract can use ```respData``` if there's no exception, which could be indicated by ```error = 0```. Otherwise the application contract should process according to ```error```. The fee paid by requester when requesting will go to the SGX account.
* ```cancel(uint64 requestId) public returns(bool)```
Either the requester or the SGX could cancel a request which hasn't been responded to. The fee paid by requester when requesting would be fully refunded if the request is cancelled by the Town Crier server, and be partially refunded if it is cancelled by the requester. 

The source code of the TownCrier contract is as below. You can look into it for more details.
```
pragma solidity ^0.4.9;

contract TownCrier {
    struct Request { // the data structure for each request
        address requester; // the address of the requester
        uint fee; // the amount of wei the requester pays for the request
        address callbackAddr; // the address of the contract to call for response delivering
        bytes4 callbackFID; // the specification of the callback function
        bytes32 paramsHash; // the hash of the request parameters
    }

    event RequestLog(address self, int16 flag); // for request debug
    event RequestInfo(uint64 id, uint8 requestType, address requester, uint fee, address callbackAddr, bytes32 paramsHash, uint timestamp, bytes32[] requestData); // log of requests, the Town Crier server watches this event and processes requests
    event DeliverLog(uint gasLeft, int flag); // for deliver debug
    event DeliverInfo(uint64 requestId, uint fee, uint gasPrice, uint gasLeft, uint callbackGas, bytes32 paramsHash, uint64 error, bytes32 respData); // log of responses
    event Cancel(uint64 requestId, address canceller, address requester, int flag); // log of cancellations

    address constant SGX_ADDRESS = 0x89B44e4d3c81EDE05D0f5de8d1a68F754D73d997; // address of the SGX account

    uint public constant GAS_PRICE = 5 * 10**10;
    uint public constant MAX_FEE = (31 * 10**5) * GAS_PRICE;

    uint public constant MIN_FEE = 30000 * GAS_PRICE; // least fee required for the requester to pay such that SGX could call deliver() to send a response
    uint public constant CANCELLATION_FEE = 25000 * GAS_PRICE; // charged when the requester cancels a request that is not responded

    uint constant TC_CANCELLED_FLAG = 2;
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
        if (msg.value < MIN_FEE || msg.value > MAX_FEE) {
            // If the amount of ether sent by the requester is too little or too much, refund the requester and discard the request.
            RequestInfo(0, requestType, msg.sender, msg.value, callbackAddr, 0, timestamp, requestData);
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
            RequestLog(this, 1);
            return requestId;
        }
    }

    function deliver(uint64 requestId, bytes32 paramsHash, uint64 error, bytes32 respData) public {
        uint fee = requests[requestId].fee;
        if (msg.sender != SGX_ADDRESS ||
                requests[requestId].requester == 0
                || fee == DELIVERED_FEE_FLAG) {
            // If the response is not delivered by the SGX account or the request has already been responded to, discard the response.
            DeliverInfo(requestId, fee, tx.gasprice, msg.gas, 0, paramsHash, error, respData);
            DeliverLog(msg.gas, -1);
            return;
        } else if (requests[requestId].paramsHash != paramsHash) {
            // If the hash of request parameters in the response is not correct, discard the response for security concern.
            DeliverInfo(requestId, fee, tx.gasprice, msg.gas, 0, paramsHash, error, respData);
            DeliverLog(msg.gas, -4);
            return;
        } else if (fee == CANCELLED_FEE_FLAG) {
            // If the request is cancelled by the requester, cancellation fee goes to the SGX account and set the request as having been responded to.
            DeliverLog(msg.gas, 1);
            if (!SGX_ADDRESS.send(CANCELLATION_FEE)){
                DeliverLog(msg.gas, -8);
                throw;
            }
            requests[requestId].fee = DELIVERED_FEE_FLAG;
            DeliverLog(msg.gas, int(CANCELLATION_FEE));
            return;
        } else if (fee == TC_CANCELLED_FLAG) {
            // If the request is cancelled by the SGX, discard the response.
            DeliverInfo(requestId, fee, tx.gasprice, msg.gas, 0, paramsHash, error, respData);
            DeliverLog(msg.gas, 2);
            return;
        }

        DeliverLog(msg.gas, 8);
        if (!SGX_ADDRESS.send(fee)) { // send the fee to the SGX account for its delivering
            DeliverLog(msg.gas, -16);
            throw;
        }
        requests[requestId].fee = DELIVERED_FEE_FLAG;
        DeliverLog(msg.gas, 16);

        uint callbackGas = (fee - MIN_FEE) / tx.gasprice; // gas left for the callback function
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
        Cancel(requestId, msg.sender, requests[requestId].requester, int(fee));
        if (requests[requestId].requester == msg.sender && fee >= CANCELLATION_FEE) {
            // If the request was sent by this user and has money left on it, then cancel it.
            if (!msg.sender.send(fee - CANCELLATION_FEE)) {
                Cancel(requestId, msg.sender, requests[requestId].requester, -2);
                throw;
            }
            requests[requestId].fee = CANCELLED_FEE_FLAG;
            Cancel(requestId, msg.sender, requests[requestId].requester, int(CANCELLED_FEE_FLAG));
            return true;
        } else if (requests[requestId].requester == SGX_ADDRESS && fee > TC_CANCELLED_FLAG) {
            // If the request was sent by SGX and it hasn't been responded or cancelled yet, then cancel it and fully refund the requester.
            if (!msg.sender.send(fee)) {
                Cancel(requestId, msg.sender, requests[requestId].requester, -4);
                throw;
            }
            requests[requestId].fee = TC_CANCELLED_FLAG;
            Cancel(requestId, msg.sender, requests[requestId].requester, int(TC_CANCELLED_FLAG));
            return true;
        } else {
            Cancel(requestId, msg.sender, requests[requestId].requester, -1);
            return false;
        }
    }
}
```

## Application Contract upon Town Crier by Example
### Stock Market Price
```
```
## Tips for Designing Application Contracts
### Flight Insurance
```
```

[Town Crier: An Authenticated Data Feed for Smart Contracts]: https://eprint.iacr.org/2016/168.pdf

