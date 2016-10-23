'use strict';


function initConfig() {
    //var fiContractAddress = '0xc94437bca37ee8445e458e553860999049433597';
    //var abiArray = [{"constant":false,"inputs":[{"name":"requestId","type":"uint64"},{"name":"delay","type":"uint256"}],"name":"pay","outputs":[],"type":"function"},{"constant":false,"inputs":[],"name":"kill","outputs":[],"type":"function"},{"constant":true,"inputs":[],"name":"owner","outputs":[{"name":"","type":"address"}],"type":"function"},{"constant":true,"inputs":[{"name":"","type":"uint256"}],"name":"policies","outputs":[{"name":"beneficiary","type":"address"},{"name":"flight","type":"string"},{"name":"paid","type":"bool"},{"name":"expired","type":"bool"}],"type":"function"},{"constant":false,"inputs":[{"name":"_flight","type":"string"}],"name":"insure","outputs":[{"name":"","type":"uint256"}],"type":"function"},{"inputs":[],"type":"constructor"},{"anonymous":false,"inputs":[{"indexed":false,"name":"policyid","type":"uint256"},{"indexed":false,"name":"beneficiary","type":"address"},{"indexed":false,"name":"flight","type":"string"}],"name":"Payment","type":"event"}];

    var fiContractAddress = '0x8622039a3d41aabe6301e289e7d3525dd747287c';
    var tcContractAddress = '0xd8ecae22aa26953488e0c91acb6120db0be74cda';
    var fiAbiArray =
        [{
            constant: false,
            inputs: [{
                name: "requestId",
                type: "uint64"
            }, {
                name: "delay",
                type: "bytes32"
            }],
                name: "pay",
                outputs: [],
                payable: false,
                type: "function"
        }, {
            constant: false,
            inputs: [{
                name: "encryptedFlightInfo",
                type: "bytes32[]"
            }, {
                name: "payment",
                type: "uint256"
            }],
                name: "insure",
                outputs: [],
                payable: true,
                type: "function"
        }, {
            constant: true,
            inputs: [],
            name: "TC_CONTRACT",
            outputs: [{
                name: "",
                type: "address"
            }],
            payable: false,
            type: "function"
        }, {
            inputs: [{
                name: "tcCont",
                type: "address"
            }],
            type: "constructor"
        }, {
            payable: false,
            type: "fallback"
        }, {
            anonymous: false,
            inputs: [{
                indexed: false,
                name: "beneficiary",
                type: "address"
            }, {
                indexed: false,
                name: "dataLength",
                type: "uint256"
            }, {
                indexed: false,
                name: "data",
                type: "bytes32[]"
            }, {
                indexed: false,
                name: "requestId",
                type: "int72"
            }],
                name: "Insure",
                type: "event"
        }, {
            anonymous: false,
            inputs: [{
                indexed: false,
                name: "flag",
                type: "int256"
            }],
            name: "PaymentLog",
            type: "event"
        }, {
            anonymous: false,
            inputs: [{
                indexed: false,
                name: "payee",
                type: "address"
            }, {
                indexed: false,
                name: "payeeBalance",
                type: "uint256"
            }, {
                indexed: false,
                name: "gasRemaining",
                type: "uint256"
            }, {
                indexed: false,
                name: "requestId",
                type: "uint64"
            }, {
                indexed: false,
                name: "delay",
                type: "uint256"
            }, {
                indexed: false,
                name: "amount",
                type: "uint256"
            }],
                name: "PaymentInfo",
                type: "event"
        }, {
            anonymous: false,
            inputs: [{
                indexed: false,
                name: "canceller",
                type: "address"
            }, {
                indexed: false,
                name: "requester",
                type: "address"
            }, {
                indexed: false,
                name: "success",
                type: "bool"
            }],
                name: "FlightCancel",
                type: "event"
        }];


    var tcAbiArray = 
        [{
            constant: false,
            inputs: [{
                name: "requestId",
                type: "uint64"
            }],
            name: "cancel",
            outputs: [{
                name: "",
                type: "bool"
            }],
            payable: false,
            type: "function"
        }, {
            constant: true,
            inputs: [],
            name: "MIN_FEE",
            outputs: [{
                name: "",
                type: "uint256"
            }],
            payable: false,
            type: "function"
        }, {
            constant: true,
            inputs: [],
            name: "CANCELLATION_FEE",
            outputs: [{
                name: "",
                type: "uint256"
            }],
            payable: false,
            type: "function"
        }, {
            constant: true,
            inputs: [],
            name: "GAS_PRICE",
            outputs: [{
                name: "",
                type: "uint256"
            }],
            payable: false,
            type: "function"
        }, {
            constant: false,
            inputs: [{
                name: "requestType",
                type: "uint8"
            }, {
                name: "callbackAddr",
                type: "address"
            }, {
                name: "callbackFID",
                type: "bytes4"
            }, {
                name: "requestData",
                type: "bytes32[]"
            }],
                name: "request",
                outputs: [{
                    name: "",
                    type: "uint64"
                }],
                payable: true,
                type: "function"
        }, {
            constant: false,
            inputs: [{
                name: "requestId",
                type: "uint64"
            }, {
                name: "paramsHash",
                type: "bytes32"
            }, {
                name: "respData",
                type: "bytes32"
            }],
                name: "deliver",
                outputs: [],
                payable: false,
                type: "function"
        }, {
            constant: true,
            inputs: [],
            name: "MAX_FEE",
            outputs: [{
                name: "",
                type: "uint256"
            }],
            payable: false,
            type: "function"
        }, {
            inputs: [],
            type: "constructor"
        }, {
            payable: false,
            type: "fallback"
        }, {
            anonymous: false,
            inputs: [{
                indexed: false,
                name: "self",
                type: "address"
            }, {
                indexed: false,
                name: "flag",
                type: "int16"
            }],
                name: "RequestLog",
                type: "event"
        }, {
            anonymous: false,
            inputs: [{
                indexed: false,
                name: "id",
                type: "uint64"
            }, {
                indexed: false,
                name: "requestType",
                type: "uint8"
            }, {
                indexed: false,
                name: "requester",
                type: "address"
            }, {
                indexed: false,
                name: "fee",
                type: "uint256"
            }, {
                indexed: false,
                name: "callbackAddr",
                type: "address"
            }, {
                indexed: false,
                name: "paramsHash",
                type: "bytes32"
            }, {
                indexed: false,
                name: "requestData",
                type: "bytes32[]"
            }],
                name: "RequestInfo",
                type: "event"
        }, {
            anonymous: false,
            inputs: [{
                indexed: false,
                name: "gasLeft",
                type: "uint256"
            }, {
                indexed: false,
                name: "flag",
                type: "int256"
            }],
                name: "DeliverLog",
                type: "event"
        }, {
            anonymous: false,
            inputs: [{
                indexed: false,
                name: "requestId",
                type: "uint64"
            }, {
                indexed: false,
                name: "fee",
                type: "uint256"
            }, {
                indexed: false,
                name: "gasPrice",
                type: "uint256"
            }, {
                indexed: false,
                name: "gasLeft",
                type: "uint256"
            }, {
                indexed: false,
                name: "callbackGas",
                type: "uint256"
            }, {
                indexed: false,
                name: "paramsHash",
                type: "bytes32"
            }, {
                indexed: false,
                name: "response",
                type: "bytes32"
            }],
                name: "DeliverInfo",
                type: "event"
        }, {
            anonymous: false,
            inputs: [{
                indexed: false,
                name: "requestId",
                type: "uint64"
            }, {
                indexed: false,
                name: "canceller",
                type: "address"
            }, {
                indexed: false,
                name: "requester",
                type: "address"
            }, {
                indexed: false,
                name: "flag",
                type: "int256"
            }],
                name: "Cancel",
                type: "event"
        }];

    var app = angular.module('flightInsuranceApp');
    app.constant('FI_CONTRACT_ADDRESS', fiContractAddress);
    app.constant('TC_CONTRACT_ADDRESS', tcContractAddress);
    app.constant('FI_ABI_ARRAY', fiAbiArray);
    app.constant('TC_ABI_ARRAY', tcAbiArray);
}

initConfig();
