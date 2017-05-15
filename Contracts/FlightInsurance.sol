contract FlightInsurance {
    struct Policy{
        address requester;
        uint64 tc_id; // the requestId in the TownCrier Contract
        bytes32[] data; // query data
        uint fee; // amount of wei a requester pays for TC to deliver the response
        uint premium; // amount of wei a requester pays for a policy
    }

    event Insure(int64 policyId, address requester, uint length, bytes32[] data, uint premium, uint timestamp); // log for policies
    event Request(uint64 policyId, address requester, int64 requestId); // log for requests sent to the TownCrier Contract
    event Response(int64 policyId, address requester, uint64 requestid, uint64 error, uint data); // log for responses from TC
    event Cancel(uint64 policyId, address requester, bool success, uint timestamp); // log for cancellations

    uint constant MIN_GAS = 30000 + 20000; // minimum gas required for a query
    uint constant GAS_PRICE = 5 * 10 ** 10;
    uint constant TC_FEE = MIN_GAS * GAS_PRICE;
    uint constant ETHER_TO_WEI = 10 ** 18;
    uint constant PAYOUT_RATE = 5; // pay a requester 5 times as much as he paid as premium for the policy when the flight is delayed
    uint constant PAYOUT_DELAY = 30; // a flight is counted as delayed if it departs no less than 30 minutes after scheduled
    uint constant DAY_EPOCH = 24 * 60 * 60; // seconds in a day

    bytes4 constant TC_CALLBACK_FID = bytes4(sha3("response(uint64,uint64,bytes32)"));

    TownCrier public TC_CONTRACT;
    address owner; // creator of this contract
    uint64 policyCnt; // counter for the policies
    Policy[2**64] policies;
    uint64[2**64] id_map; // maps the requestId in the TownCrier Contract to the policyId in this contract

    function() public payable {} // must be payable

    function FlightInsurance(TownCrier tcCont) public payable{
        TC_CONTRACT = tcCont;
        owner = msg.sender;
        policyCnt = 1;
    }

    function insure(bytes32[] flightInfo, uint payment) public payable{
        payment = payment * ETHER_TO_WEI; // conver the payment unit from ether to wei

        if (msg.value < payment + TC_FEE
                || block.timestamp + DAY_EPOCH > uint(flightInfo[1])) {
            // A buyer could only buy a policy at least 24 hours 
            // ahead the scheduled departure time of his flight.
            if (!msg.sender.send(msg.value)) {
                throw;
            }
            Insure(-1, msg.sender, flightInfo.length, flightInfo, payment, uint(flightInfo[1]));
            return;
        }

        // Record the policy information.
        uint64 policyId = policyCnt;
        ++policyCnt;

        policies[policyId].requester = msg.sender;
        policies[policyId].tc_id = 0;
        policies[policyId].data = flightInfo;
        policies[policyId].fee = msg.value - payment;
        policies[policyId].premium = payment;

        Insure(int64(policyId), msg.sender, flightInfo.length, flightInfo, payment, block.timestamp);
    }

    function request(uint64 policyId) public {
        if (policyId == 0 || policies[policyId].requester != msg.sender
                || policies[policyId].tc_id != 0 || policies[policyId].fee == 0) {
            // If the requester is not the owner of the policy,
            // or the policy has been responded to or cancelled,
            // discard the request.
            Request(policyId, msg.sender, -1);
            return;
        }

        policies[policyId].tc_id = TC_CONTRACT.request.value(policies[policyId].fee)(1, this, TC_CALLBACK_FID, 0, policies[policyId].data); //calling request() in the TownCrier Contract
        if (policies[policyId].tc_id == 0) {
            // Request fails.
            // Refund the policy owner the premium and delivery fee he paid.
            policies[policyId].fee = 0;
            if (!msg.sender.send(policies[policyId].premium + policies[policyId].fee)) {
                Request(policyId, msg.sender, -2);
                throw;
            }
            Request(policyId, msg.sender, 0);
        } else {
            // Successfully sent a request to TC.
            // Record the mapping between the policyId and the requestId.
            id_map[policies[policyId].tc_id] = policyId;
            Request(policyId, msg.sender, int64(policies[policyId].tc_id));
        }
    }



    function response(uint64 requestId, uint64 error, bytes32 respData) public {
        if (msg.sender != address(TC_CONTRACT)) {
            // If message sender is not the TownCrier Contract,
            // discard the response.
            Response(-1, msg.sender, 0, 0, 0);
            return;
        }

        uint64 policyId = id_map[requestId]; // look up the map and find the policyId associated with this requestId
        address requester = policies[policyId].requester;
        uint premium = policies[policyId].premium;
        uint delay = uint(respData);

        policies[policyId].fee = 0; // set the policy as responded
        if (error == 0){ 
            // No error exists, then delay makes sense.
            if (delay >= PAYOUT_DELAY) {
                // Flight delayed or cancelled.
                // Pay the policy owner 5 times as much as the premium.
                requester.send(premium * PAYOUT_RATE);
            } else {
                // Flight not delayed.
                // Premium goes to the creator of this contract.
                owner.send(premium);
            }
        } else if (error == 1) {
            // Flight not found.
            // Refund the policy owner the premium he paid.
            requester.send(premium);
        } else {
            // Error occurs in TC.
            // Refund the policy owner the premium and query fee he paid.
            requester.send(premium + policies[policyId].fee);
        }
        Response(int64(policyId), msg.sender, requestId, error, delay); // log the response
    }

    function cancel(uint64 policyId) public {
        if (policyId == 0 || policies[policyId].requester != msg.sender
                || policies[policyId].fee == 0 || policies[policyId].tc_id != 0
                || block.timestamp + DAY_EPOCH > uint(policies[policyId].data[1])) {
            // A request could only cancel a policy 
            // when the policy has not been reponded or requested to TC,
            // and it's at least 24 hours ahead the scheduled departure time of the insured flight.
            Cancel(policyId, msg.sender, false, uint(policies[policyId].data[1]));
            return;
        }

        policies[policyId].fee = 0; // set the policy as cancelled
        
        // Since the policy has not been requested to TC,
        // the policy owner can be fully refunded.
        if (!msg.sender.send(policies[policyId].fee + policies[policyId].premium)) {
            Cancel(policyId, msg.sender, false, 0);
            throw;
        }

        Cancel(policyId, msg.sender, true, block.timestamp);
    }
}

