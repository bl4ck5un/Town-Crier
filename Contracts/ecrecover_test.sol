// Test Example
// hash = 0xAB530A13E45914982B79F9B7E3FBA994CFD1F3FB22F71CEA1AFBF02B460C6D1D
// r = 0xBB48AE3726C5737344A54B3463FEC499CB108A7D11BA137BA3C7D043BD6D7E14
// s = 0xD7BA684E8B9A03322F46424DC3FFB8A6AB49CA2F645445F00D983BBE92B77FAC
// v = 28

contract VerifySignature {
    address owner;

    function VerifySignature() {
        owner = msg.sender;
    }

    function expectedAddress() returns(address) {
        return 0x89b44e4d3c81ede05d0f5de8d1a68f754d73d997;
    }
    
    function test(bytes32 hash, uint8 v, bytes32 r, bytes32 s) returns(address) {
        return ecrecover.gas(5000)(hash, v, r, s);
    }
}                              
