import "TownCrier.sol";

contract SteamTrade {
    event UINT(uint u);
    event Buy(bytes32 buyerId, uint32 time, bytes32[2] encSellerApi, bytes32 item, int flag);

    address owner;
    TownCrier public TC_CONTRACT;
    bytes32 public ID_S;
    bytes32 public ITEM;
//    bytes32[] public LIST_I;
    bytes32[2] public encAPI;
    uint public P;
    address[2**64] buyers;
    uint constant TC_FEE = (35000 + 20000) * 5 * 10**10;
    bytes4 constant TC_CALLBACK_FID = 0x3d622256;

    function SteamTrade(TownCrier tcCont, bytes32 encApiKey0, bytes32 encApiKey1, bytes32 item, uint p) public {
        TC_CONTRACT = tcCont;
        owner = msg.sender;
        ITEM = item;
        P = p;
        encAPI[0] = encApiKey0;
        encAPI[1] = encApiKey1;
    }

    function purchase(bytes32 ID_B, uint32 T_B) returns (uint) {
        uint i;
        uint j;
        if (msg.value != P + TC_FEE) {
            Buy(ID_B, T_B, encAPI, ITEM, -1);
            msg.sender.send(msg.value);
            return 0;
        }

        Buy(ID_B, T_B, encAPI, ITEM, 1);

        bytes32[] memory format = new bytes32[](6);
        format[0] = encAPI[0];
        format[1] = encAPI[1];
        format[2] = ID_B;
        format[3] = bytes32(T_B);
        format[4] = bytes32(1);
        format[5] = ITEM;
        uint64 requestId = TC_CONTRACT.request.value(TC_FEE)(2, this, TC_CALLBACK_FID, format);
        buyers[requestId] = msg.sender;
        UINT(format.length);
        return format.length;
    } 

    function pay(uint64 requestId, bytes32 result) public { // uint result, uint payout) public {
        address buyer = buyers[requestId];
        if (msg.sender != address(TC_CONTRACT)) { // && msg.sender != 0x50adbfc5017cc4fe557e64425c8d0ce674f8de69) {
            UINT(666);
            return;
        }
        if (buyer == 0) {
            return;
        }

        if (uint(result) > 0) {
            owner.send(P);
            UINT(1);
        }
        else {
            buyer.send(P);
            UINT(0);
        }
    }
    
}
