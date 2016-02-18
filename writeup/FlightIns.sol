// A simple flight insurance contract using Town Crier's private datagram.
contract FlightIns {
  uint8  constant TC_REQ_TYPE = 0;
  uint   constant TC_FEE      = (35000 + 20000) * 5 * 10**10;
  uint   constant FEE         = 10**18;     //   $5 in wei
  uint   constant PAYOUT      = 2 * 10**19; // $200 in wei
  uint32 constant MIN_DELAY   = 30;

  // The function identifier in Solidity is the first 4 bytes
  // of the sha3 hash of the functions' canonical signature.
  // This contract's callback is bytes4(sha3("pay(uint64,bytes32)"))
  bytes4 constant CALLBACK_FID = 0x3d622256;

  TownCrier tc;
  address[2**64] requesters;

  // Constructor which sets the address of the Town Crier contract.
  function FlightIns(TownCrier _tc) public {
    tc = _tc;
  }

  // A user can purchase insurance through this entry point.
  // encFN is an encryption of the flight number and date
  // as well as the time when Town Crier should respond to the request.
  function insure(bytes32[] encFN) public {
    if (msg.value != FEE) return;

    // Adding money to a function call involves calling ".value()"
    // on the function itself before calling it with arguments.
    uint64 requestId =
      tc.request.value(TC_FEE)(TC_REQ_TYPE, this, CALLBACK_FID, encFN);
    requesters[requestId] = msg.sender;
  }

  // This is the entry point for Town Crier to respond to a request.
  function pay(uint64 requestId, bytes32 delay) public {
    // Check that this is a response from Town Crier
    // and that the ID is valid and unfulfilled.
    address requester = requesters[requestId];
    if (msg.sender != address(tc) || requester == 0) return;

    if (uint(delay) >= MIN_DELAY) {
      address(requester).send(PAYOUT);
    }
    requesters[requestId] = 0;
  }
}

