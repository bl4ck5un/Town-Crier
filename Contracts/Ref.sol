pragma solidity ^0.4.9;

contract TownCrier {
        function request(uint8 requestType, address callbackAddr, bytes4 callbackFID, uint timestamp, bytes32[] requestData) public payable returns (uint64);
}

