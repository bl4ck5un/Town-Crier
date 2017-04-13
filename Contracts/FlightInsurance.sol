contract FlightInsurance {
    struct Policy{
        address requester;
        uint64 tc_id;
        bytes32[] data;
        uint flightTime;
        uint fee;
        uint premium;
    }

    event Insure(int64 policyId, address requester, uint length, bytes32[] data, uint premium, uint timestamp);
    event Request(uint64 policyId, address requester, int64 requestId);
    event Response(int64 policyId, address requester, uint64 requestid, uint64 error, uint data);
    event Cancel(uint64 policyId, address requester, bool success, uint timestamp);

    uint constant MIN_GAS = 30000 + 20000;
    uint constant GAS_PRICE = 5 * 10 ** 10;
    uint constant TC_FEE = MIN_GAS * GAS_PRICE;
    uint constant ETHER_TO_WEI = 10 ** 18;
    uint constant PAYOUT_RATE = 5;
    uint constant PAYOUT_DELAY = 30;
    uint constant DAY_EPOCH = 24 * 60 * 60;

    bytes4 constant TC_CALLBACK_FID = bytes4(sha3("response(uint64,uint64,bytes32)"));

    TownCrier public TC_CONTRACT;
    address owner;
    uint64 policyCnt;
    Policy[2**64] policies;
    uint64[2**64] id_map;

    function() payable {} // must be payable

    function FlightInsurance(TownCrier tcCont) public payable{
        TC_CONTRACT = tcCont;
        owner = msg.sender;
        policyCnt = 1;
    }

    function insure(bytes32[] flightInfo, uint payment) public payable{
        payment = payment * ETHER_TO_WEI;

        uint time = 0;
        for (uint j = 0; j < 32; j++) {
            byte char = byte(bytes32(uint(flightInfo[1]) * 2 ** (8 * j)));
            if (char != 0) {
                time = time * 10 + uint(char) - 48;
            }
        }

        if (msg.value < payment + TC_FEE 
                || block.timestamp + DAY_EPOCH > time) {
            if (!msg.sender.send(msg.value)) {
                throw;
            }
            Insure(-1, msg.sender, flightInfo.length, flightInfo, payment, time);
            return;
        }

        uint64 policyId = policyCnt;
        ++policyCnt;

        policies[policyId].requester = msg.sender;
        policies[policyId].tc_id = 0;
        policies[policyId].data = flightInfo;
        policies[policyId].flightTime = time;
        policies[policyId].fee = msg.value - payment;
        policies[policyId].premium = payment;

        Insure(int64(policyId), msg.sender, flightInfo.length, flightInfo, payment, block.timestamp);
    }

    function request(uint64 policyId) public {
        if (policyId == 0 || policies[policyId].requester != msg.sender
                || policies[policyId].tc_id != 0 || policies[policyId].fee == 0) {
            Request(policyId, msg.sender, -1);
            return;
        }

        policies[policyId].tc_id = TC_CONTRACT.request.value(policies[policyId].fee)(1, this, TC_CALLBACK_FID, 0, policies[policyId].data); 
        if (policies[policyId].tc_id == 0) {
            if (!msg.sender.send(policies[policyId].premium + policies[policyId].fee)) {
                Request(policyId, msg.sender, -2);
                throw;
            }
            policies[policyId].fee = 0;
            Request(policyId, msg.sender, 0);
        } else {
            id_map[policies[policyId].tc_id] = policyId;
            Request(policyId, msg.sender, int64(policies[policyId].tc_id));
        }
    }



    function response(uint64 requestId, uint64 error, bytes32 respData) public {
        if (msg.sender == address(TC_CONTRACT)) { //!!!
            Response(-1, msg.sender, 0, 0, 0);
            return;
        }

        uint64 policyId = id_map[requestId];
        address requester = policies[policyId].requester;
        uint premium = policies[policyId].premium;
        uint delay = uint(respData);

        if (error == 0){ // no problem
            if (delay >= PAYOUT_DELAY) {
                if (!requester.send(premium * PAYOUT_RATE)) {
                    Response(-2, msg.sender, 0, 0, 0);
                    throw;
                }
            } else {
                if (!owner.send(premium)) {
                    Response(-4, msg.sender, 0, 0, 0);
                    throw;
                }
            }
        } else if (error == 1) { // invalid request 
            if (!requester.send(premium)) {
                Response(-8, msg.sender, 0, 0, 0);
                throw;
            }
        } else {
            if (!requester.send(premium + policies[policyId].fee)) {
                Response(-16, msg.sender, 0, 0, 0);
                throw;
            }
        }
        Response(int64(policyId), msg.sender, requestId, error, delay);
        policies[policyId].fee = 0;
    }

    function cancel(uint64 policyId) public {
        if (policyId == 0 || policies[policyId].requester != msg.sender
                || policies[policyId].fee == 0 || policies[policyId].tc_id != 0
                || block.timestamp + DAY_EPOCH > policies[policyId].flightTime) {
            Cancel(policyId, msg.sender, false, block.timestamp);
            return;
        }

        if (!msg.sender.send(policies[policyId].fee + policies[policyId].premium)) {
            Cancel(policyId, msg.sender, false, 0);
            throw;
        }

        policies[policyId].fee = 0;
        Cancel(policyId, msg.sender, true, block.timestamp);
    }
}

