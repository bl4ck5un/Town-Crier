Examples
========

.. note::
  To show how to interface with the ``Town Crier`` Contract, we present a  skeleton ``Application`` Contract that does nothing other than sending queries, logging responses and cancelling queries.

The source code of the application contract can be found `here <https://github.com/bl4ck5un/Town-Crier/blob/master/contracts/Application.sol>`_. Now we go through the contract code line by line.

Preliminaries
-------------

First, you need to annotate your contract with the version pragma:

.. code-block:: javascript

	pragma solidity ^0.4.9;

Second, you need to include in your contract the function declaration of the ``TownCrier`` Contract so that the application contract can call those functions with the address of the ``TownCrier`` Contract.

.. code-block:: javascript

	contract TownCrier {
	    function request(uint8 requestType,
                    address callbackAddr,
                    bytes4 callbackFID,
                    uint timestamp,
                    bytes32[] requestData) public payable returns (uint64);
	    function cancel(uint64 requestId) public returns (int);
	}


.. note::
    You do not need to include ``response()`` here because an appilcation contract should not make a function call to it but wait for being called by it.

Outline of ``Application.sol``
------------------------------

Let's look at the layout of the ``Application`` Contract:

.. code-block:: javascript

	contract Application {
	    event Request(int64 requestId, address requester, uint dataLength, bytes32[] data);
	    event Response(int64 requestId, address requester, uint64 error, uint data);
	    event Cancel(uint64 requestId, address requester, bool success);

	    bytes4 constant TC_CALLBACK_FID = bytes4(sha3("response(uint64,uint64,bytes32)"));

	    address[2**64] requesters;
	    uint[2**64] fee;

	    function() public payable;
	    function Application(TownCrier tcCont) public;
	    function request(uint8 requestType, bytes32[] requestData) public payable;
	    function response(uint64 requestId, uint64 error, bytes32 respData) public;
	    function cancel(uint64 requestId) public;
	}

* The events ``Request()``, ``Response`` and ``Cancel()`` keeps logs of the ``requestId`` assigned to a query, the response from TC and the result of a cancellation respectively for a user to fetch from the blockchain.
* The constant ``TC_CALLBACK_FID`` is the first 4 bytes of the hash of the function ``response()`` that the ``TownCrier`` Contract calls when relaying the response from TC. The name of the callback function can differ but the three parameters should be exactly the same as in this example.
* The address array ``requesters`` stores the addresses of the requesters.
* The uint array ``fee`` stores the amounts of wei requesters pay for their queries.

As you can see above, the ``Application`` Contract consists of a set of five basic functions:

Default Function
++++++++++++++++

.. code-block:: javascript

    function() public payable;

This fallback function must be payable so that TC can provide a refund under certain conditions. The fallback function should not cost more than 2300 gas, otherwise it will run out of gas when TC refunds ether to it. In our contract, it simply does nothing.

.. code-block:: javascript

    function() public payable {}

Constructor
+++++++++++

.. code-block:: javascript

    function Application(TownCrier tc) public;

This is the constructor which registers the address of the TC Contract and the owner of this contract during creation so that it can call the ``request()`` and ``cancel()`` functions in the TC contract.

.. code-block:: javascript

    TownCrier public TC_CONTRACT;
    address owner;

    function Application(TownCrier tcCont) public {
        TC_CONTRACT = tcCont;
        owner = msg.sender;
    }

Submitting Requests
+++++++++++++++++++

.. code-block:: javascript

    function request(uint8 requestType, bytes32[] requestData) public payable;

A user calls this function to send a request to the ``Application`` Contract. This function forwards the query to the ``request()`` of the TC Contract by

.. code-block:: javascript

	requestId = TC_CONTRACT.request.value(msg.value)(requestType, TC_CALLBACK_ADD, TC_CALLBACK_FID, timestamp, requestData);

``msg.value`` is the fee the user pays for this request. ``TC_CALLBACK_ADD`` is the address of the callback contract. or ``this`` for the current contract. ``TC_CALLBACK_FID`` is the first 4 bytes of the hash of the callback function signature, as defined above.

.. code-block:: javascript

    uint constant MIN_GAS = 30000 + 20000;
    uint constant GAS_PRICE = 5 * 10 ** 10;
    uint constant TC_FEE = MIN_GAS * GAS_PRICE;

    function request(uint8 requestType, bytes32[] requestData) public payable {
        if (msg.value < TC_FEE) {
            // If the user doesn't pay enough fee for a request,
            // we should discard the request and return the ether.
            if (!msg.sender.send(msg.value)) throw;
            return;
        }

        int requestId = TC_CONTRACT.request.value(msg.value)(requestType, this, TC_CALLBACK_FID, 0, requestData);
        if (requestId == 0) {
            // If the TC Contract returns 0 indicating the request fails
            // we should discard the request and return the ether.
            if (!msg.sender.send(msg.value)) throw;
            return;
        }

        // If the request succeeds,
        // we should record the requester and how much fee he pays.
        requesters[uint64(requestId)] = msg.sender;
        fee[uint64(requestId)] = msg.value;
        Request(int64(requestId), msg.sender, requestData.length, requestData);
    }

.. warning::
    Developers need to send enough fee.

    TC requires at least **3e4** gas for all the operations (besides calling the callback function). The gas price is set to **5e10 wei**. So the caller should pay at least **(3e4 + callback_gas) * 5e10 wei**. Otherwise the ``request`` call will fail (and the TC Contract will return 0 as ``requestId``). Developers should handle this failure.

For our ``Application.sol``, the callback function (``response``) costs about 2e4 gas, so the caller should pay no less than (3e4 + 2e4) * 5e10 = 2.5e15 wei (denoted as ``TC_FEE``).

.. note::

    TC server sets the gas limit as **3e6** when sending the response to the TC Contract. If a requester paid more gas than that, the excess ether will not be used for the callback function. It will go directly to the SGX wallet. This is a way to offer a tip for the Town Crier service.

Receiving Responses
+++++++++++++++++++

.. code-block:: javascript

    function response(uint64 requestId, uint64 error, bytes32 respData) public;

This is the function to be called by the TC Contract to deliver the response from TC server. The selector for this function is passed to the request call. See `Submitting Requests`_.

.. code-block:: javascript

    function response(uint64 requestId, uint64 error, bytes32 respData) public {
        // If the response is not sent from the TC Contract,
        // we should discard the response.
        if (msg.sender != address(TC_CONTRACT)) return;

        address requester = requesters[requestId];
        // Set the request state as responded.
        requesters[requestId] = 0;

        if (error < 2) {
            // If either TC responded with no error or the request is invalid by the requester's fault,
            // public the response on the blockchain by event Response().
            Response(int64(requestId), requester, error, uint(respData));
        } else {
            // If error exists by TC's fault,
            // fully refund the requester.
            requester.send(fee[requestId]);
            Response(int64(requestId), msg.sender, error, 0);
        }
    }

.. warning::
    Since the gas limit for sending a response back to the TC Contract is set as **3e6** by the Town Crier server, as mentioned above, the callback function should not consume more gas than that. Otherwise the callback function will run out of gas and fail. The TC service does not take responsibility for such failures, and treats queries that fail in this way as successfully responded to.

To estimate how much gas the callback function costs, you can use web3.eth.estimateGas_.

Cancellation
++++++++++++

.. code-block:: javascript

    function cancel(uint64 requestId) public;

This function calls the ``cancel()`` function of the TC Contract, to cancel a unprocessed request.

.. code-block:: javascript

    uint constant CANCELLATION_FEE = 25000 * GAS_PRICE;

    function cancel(uint64 requestId) public {
        // If the cancellation request is not sent by the requester himself,
        // discard the cancellation request.
        if (requestId == 0 || requesters[requestId] != msg.sender) return;

        bool tcCancel = TC_CONTRACT.cancel(requestId);
        if (tcCancel) {
            // If the cancellation succeeds,
            // set the request state as cancelled and partially refund the requester.
            requesters[requestId] = 0;
            if (!msg.sender.send(fee[requestId] - CANCELLATION_FEE)) throw;
            Cancel(requestId, msg.sender, true);
        }
    }

TC charges **2.5e4 * 5e10 = 1.25e15 wei**, denoted as ``CANCELLATION_FEE`` here, for cancellation.
In this function a user is partially refunded ``fee - CANCELLATION_FEE``. A developer must carefully set a cancelled flag for the request before refunding the requester in order to prevent reentrancy attacks.

Send queries to Application.sol
-------------------------------

You can play with the ``Application.sol`` deployed on Rinkeby testnet, at `0xdE34AfC49b8A15bEb76A6E942bD687143C1574B6 <https://rinkeby.etherscan.io/address/0xdE34AfC49b8A15bEb76A6E942bD687143C1574B6>`_.

Assuming we're at the geth console loaded with the following script. You can find a script for this purpose `here <https://github.com/bl4ck5un/Town-Crier/blob/master/scripts/rinkeby/launch.js>`_.

.. code-block:: javascript

    function createApp(tc) {
        unlockAccounts();
        var tradeContract = App.new(
            tc, {
                from: tcDevWallet,
                data: "0x" + compiledContract.contracts["Application"].bin,
                gas: gasCnt
            },
            function (e, c) {
                if (!e) {
                    if (c.address) {
                        console.log('Application created at: ' + c.address)
                    }
                } else {
                    console.log('Failed to create Application contract: ' + e)
                }
            });
        return tradeContract;
    }

    function request(contract, type, requestData) {
        unlockAccounts();
        contract.request.sendTransaction(type, requestData, {
            from: tcDevWallet,
            value: 3e15,
            gas: gasCnt
        });
        return "Request sent!";
    }

    function watch_events(contract) {
        var his = contract.allEvents({fromBlock: 0, toBlock: 'latest'});
        var events;
        his.get(function (error, result) {
            if (!error) {
                console.log(result.length);
                for (var i = 0; i < result.length; ++i) {
                    console.log(i + " : " + result[i].event);
                }
                events = result;
            } else {
                console.log("error");
                events = "error";
            }
        });
        return events;
    }

Let's try to trigger Application.sol to query for bitcoin price (from coinmarketcap.com) and Bitcoin Fee.

First, create an instance of Application.sol.

.. code-block:: javascript

    > var App = web3.eth.contract(JSON.parse("[{"constant":false,"inputs":[{"name":"requestType","type":"uint8"},{"name":"requestData","type":"bytes32[]"}],"name":"request","outputs":[],"payable":true,"stateMutability":"payable","type":"function"},{"constant":false,"inputs":[{"name":"requestId","type":"uint64"}],"name":"cancel","outputs":[],"payable":false,"stateMutability":"nonpayable","type":"function"},{"constant":true,"inputs":[],"name":"TC_CONTRACT","outputs":[{"name":"","type":"address"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":false,"inputs":[{"name":"requestId","type":"uint64"},{"name":"error","type":"uint64"},{"name":"respData","type":"bytes32"}],"name":"response","outputs":[],"payable":false,"stateMutability":"nonpayable","type":"function"},{"inputs":[{"name":"tcCont","type":"address"}],"payable":false,"stateMutability":"nonpayable","type":"constructor"},{"payable":true,"stateMutability":"payable","type":"fallback"},{"anonymous":false,"inputs":[{"indexed":false,"name":"requestId","type":"int64"},{"indexed":false,"name":"requester","type":"address"},{"indexed":false,"name":"dataLength","type":"uint256"},{"indexed":false,"name":"data","type":"bytes32[]"}],"name":"Request","type":"event"},{"anonymous":false,"inputs":[{"indexed":false,"name":"requestId","type":"int64"},{"indexed":false,"name":"requester","type":"address"},{"indexed":false,"name":"error","type":"uint64"},{"indexed":false,"name":"data","type":"uint256"}],"name":"Response","type":"event"},{"anonymous":false,"inputs":[{"indexed":false,"name":"requestId","type":"uint64"},{"indexed":false,"name":"requester","type":"address"},{"indexed":false,"name":"success","type":"bool"}],"name":"Cancel","type":"event"}]"));
    > app = App.at("0xdE34AfC49b8A15bEb76A6E942bD687143C1574B6");


Now, send a few requests!

.. code-block:: javascript

    > request(app, 2, []); // get current bitcoin transaction fee
    > request(app, 5, ['bitcoin']);"; // get current bitcoin price

To see the responses (and the requests), examine the log:

.. code-block:: javascript

    > watch_events(app);

You'll see something like this for bitcoin transaction fee query:

.. code-block:: javascript

    9 : Request
    {
       "args":{
          "data":[

          ],
          "dataLength":"0",
          "requestId":"5",
          "requester":"0x8f108aab17e3b90f6855a73349511f5944b7e146"
       },
       "blockNumber":2182246,
       "transactionHash":"0x027f8b992b65b58f1aa2191e6ae55d1c074cdaa475a71823d1c879ddc8cbae79",
    }
    10 : Response
    {
       "args":{
          "data":"100", // fastestFee=100 from https://bitcoinfees.earn.com/api/v1/fees/recommended
          "error":"0",
          "requestId":"5",
          "requester":"0x8f108aab17e3b90f6855a73349511f5944b7e146"
       },
       "blockNumber":2182248,
       "transactionHash":"0x630d8f7fae392c2ff6a0115956e72cad5fae4e008f3ef5e543d02c112a7d5cf5",
    }

For bitcoin price query:

.. code-block:: javascript

    11 : Request
    {
       "args":{
          "data":[
             // ascii of 'bitcoin', to get the current bitcoin price
             "0x626974636f696e00000000000000000000000000000000000000000000000000"
          ],
          "dataLength":"1",
          "requestId":"6",
          "requester":"0x8f108aab17e3b90f6855a73349511f5944b7e146"
       },
       "blockNumber":2182269,
       "transactionHash":"0x673a1db9c675646c6319959f879bd8a3f711393667e6343c2125e707a70e8616",
    }
    12 : Response
    {
       "args":{
          "data":"9204", // bitcoin price is 9204 USD at Fri Apr 27 00:44:58 EDT 2018
          "error":"0",
          "requestId":"6",
          "requester":"0x8f108aab17e3b90f6855a73349511f5944b7e146"
       },
       "blockNumber":2182271,
       "transactionHash":"0x1464d26cbab1238ce8ac4ac48cd2019425be59c451099d2437056ac6c253bf40",
    }

.. _web3.eth.estimateGas: https://github.com/ethereum/wiki/wiki/JavaScript-API#web3ethestimategas
