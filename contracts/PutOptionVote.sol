import "TownCrier.sol";

contract PutOption {
    event Put(int flag, address tc, bytes32[] data);
    event Pay(uint unitPrice, int amount);

    uint constant TC_FEE = (35000 + 44000) * 5 * 10**10;

    bytes4 constant TC_CALLBACK_FID = 0x212e78ad; // bytes4(sha3("settle(uint64,bytes32)"));

    address ISSUER;

    TownCrier public TC_CONTRACT0;
    TownCrier public TC_CONTRACT1;
    TownCrier public TC_CONTRACT2;
    bytes32 public TICKER;
    uint public UNIT_PRICE;
    uint public MAX_UNITS;
    uint public STRIKE_PRICE;
    uint public EXPR_DATE;

    address buyer;
    uint units;

    bool optionPut;
    bool cancelled;

    bool[3] delivered;
    uint[3] prices;

    function PutOption(TownCrier tcContract0, TownCrier tcContract1, TownCrier tcContract2, bytes32 ticker, uint unitPrice, uint maxUnits, uint strikePrice, uint exprDate) public {
        if (msg.value < (strikePrice - unitPrice) * maxUnits + TC_FEE) throw;

        ISSUER = msg.sender;
        TICKER = ticker;
        UNIT_PRICE = unitPrice;
        MAX_UNITS = maxUnits;
        STRIKE_PRICE = strikePrice;
        EXPR_DATE = exprDate;
        optionPut = false;
        cancelled = false;

        TC_CONTRACT0 = tcContract0;
        TC_CONTRACT1 = tcContract1;
        TC_CONTRACT2 = tcContract2;
    }

    function buy(uint unitsToBuy) public {
        if (cancelled
                || block.timestamp >= EXPR_DATE
                || buyer != 0
                || unitsToBuy > MAX_UNITS
                || unitsToBuy * UNIT_PRICE != msg.value) throw;

        buyer = msg.sender;
        units = unitsToBuy;

        uint issuerRefund = this.balance - (unitsToBuy * STRIKE_PRICE) - (TC_FEE * 3);
        if (issuerRefund > 0) {
            ISSUER.send(issuerRefund);
        }
    }

    function put() public {
        if (msg.sender != buyer || block.timestamp >= EXPR_DATE || optionPut) throw;

        optionPut = true;

        bytes32[] memory tcData = new bytes32[](1);
        tcData[0] = TICKER;

        Put(0, address(TC_CONTRACT0), tcData);
        TC_CONTRACT0.request.value(TC_FEE)(1, this, TC_CALLBACK_FID, tcData);

        Put(1, address(TC_CONTRACT1), tcData);
        TC_CONTRACT1.request.value(TC_FEE)(1, this, TC_CALLBACK_FID, tcData);

        Put(2, address(TC_CONTRACT2), tcData);
        TC_CONTRACT2.request.value(TC_FEE)(1, this, TC_CALLBACK_FID, tcData);
    }

    function settle(uint64 requestId, bytes32 priceBytes) public {
        if (this.balance == 0) return;

        if (msg.sender == address(TC_CONTRACT0)) {
            Pay(uint(priceBytes), -1);
            _checkAndPay(0, uint(priceBytes));
        } else if (msg.sender == address(TC_CONTRACT1)) {
            Pay(uint(priceBytes), -2);
            _checkAndPay(1, uint(priceBytes));
        } else if (msg.sender == address(TC_CONTRACT2)) {
            Pay(uint(priceBytes), -3);
            _checkAndPay(2, uint(priceBytes));
        } else {
            throw;
        }
    }

    function _checkAndPay(uint8 index, uint price) private {
        if (delivered[index]) throw;

        delivered[index] = true;

        uint8 otherIdx0 = (index + 1) % 3;
        uint8 otherIdx1 = (index + 2) % 3;
        if ((delivered[otherIdx0] && prices[otherIdx0] == price)
                || (delivered[otherIdx1] && prices[otherIdx1] == price)) {
            uint optionValue;
            if (price < STRIKE_PRICE) {
                optionValue = (STRIKE_PRICE - price) * units;
                buyer.send(optionValue);
                Pay(price, int(optionValue));
            } else {
                Pay(price, 0);
            }
            ISSUER.send(this.balance);
        } else {
            prices[index] = price;
        }
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

