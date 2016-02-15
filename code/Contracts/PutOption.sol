import "TownCrier-vote.sol";

contract PutOption {
    event Put(bytes32 timestamp, uint gas, bytes32[] data);
    event Pay(uint unitPrice, int amount);

    uint constant TC_FEE = (30000 + (2600 * 2) + 18000) * 5 * 10**10;

    bytes4 constant TC_CALLBACK_FID = 0x212e78ad; // bytes4(sha3("settle(uint64,bytes32)"));

    address ISSUER;

    TownCrierVote public TC_CONTRACT;
    bytes32 public TICKER;
    uint public UNIT_PRICE;
    uint public MAX_UNITS;
    uint public STRIKE_PRICE;
    uint public EXPR_DATE;

    address buyer;
    uint units;

    bool optionPut;
    bool cancelled;

    function PutOption(TownCrierVote tcContract, bytes32 ticker, uint unitPrice, uint maxUnits, uint strikePrice, uint exprDate) public {
        if (msg.value < (strikePrice - unitPrice) * maxUnits + TC_FEE) throw;

        ISSUER = msg.sender;
        TC_CONTRACT = tcContract;
        TICKER = ticker;
        UNIT_PRICE = unitPrice;
        MAX_UNITS = maxUnits;
        STRIKE_PRICE = strikePrice;
        EXPR_DATE = exprDate;
        optionPut = false;
        cancelled = false;
    }

    function buy(uint unitsToBuy) public {
        if (cancelled
                || block.timestamp >= EXPR_DATE
                || buyer != 0
                || unitsToBuy > MAX_UNITS
                || unitsToBuy * UNIT_PRICE != msg.value) throw;

        buyer = msg.sender;
        units = unitsToBuy;

        uint issuerRefund = this.balance - (unitsToBuy * STRIKE_PRICE) - TC_FEE;
        if (issuerRefund > 0) {
            ISSUER.send(issuerRefund);
        }
    }

    function put() public {
        if (msg.sender != buyer || block.timestamp >= EXPR_DATE || optionPut) throw;

        optionPut = true;

        uint time = block.timestamp;
        bytes32[] memory tcData = new bytes32[](2);
        tcData[0] = TICKER;
        tcData[1] = bytes32(time);

        Put(bytes32(time), msg.gas, tcData);

        TC_CONTRACT.request.value(TC_FEE)(1, this, TC_CALLBACK_FID, tcData);
    }

    function settle(uint64 requestId, bytes32 priceBytes) public {
        if (msg.sender != address(TC_CONTRACT)) throw;

        uint price = uint(priceBytes);
        uint optionValue;
        if (price < STRIKE_PRICE) {
            optionValue = (STRIKE_PRICE - price) * units;
            buyer.send(optionValue);
            Pay(price, int(optionValue));
        } else {
            Pay(price, 0);
        }
        ISSUER.send(this.balance);
    }

    function recover() public {
        if (msg.sender != ISSUER
                || (buyer != 0 && block.timestamp < EXPR_DATE)
                || optionPut
                || cancelled) throw;

        cancelled = true;
        ISSUER.send(this.balance);
    }
}

