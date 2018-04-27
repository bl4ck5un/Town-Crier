Understand the Town Crier contract
=======================================

The ``TownCrier`` contract provides a uniform interface for queries from and replies to an application contract, which we also refer as a "Requester".
This interface consists of the following three functions.


.. code-block:: javascript

  request(uint8 requestType, address callbackAddr, \
        bytes4 callbackFID, uint256 timestamp, \
        bytes32[] requestData) public payable returns(uint64);


An application contract sends queries to TC by calling function ``request()``, and it needs to send the following parameters.

- ``requestType``: indicates the query type. You can find the query types and respective formats that Town Crier currently supports on the Dev page.
- ``callbackAddr``: the address of the application contract to which the response from Town Crier is forwarded.
- ``callbackFID``: specifies the callback function in the application contract to receive the response from TC.
- ``timestamp``: currently unused parameter. This parameter will be used in a future feature. Currently TC only responds to requests immediately. Eventually TC will support requests with a future query time pre-specified by ``timestamp``. At present, developers can ignore this parameter and just set it to 0.
- ``requestData``: data specifying query parameters. The format depends on the query type.

When the ``request`` function is called, a request is logged by event ``RequestInfo()``.
The function returns a ``requestId`` that is uniquely assigned to this request.
The application contract can use the ``requestId`` to check the response or status of a request in its logs.
The Town Crier server watches events and processes a request once logged by ``RequestInfo()``.

Requesters must prepay the gas cost incurred by the Town Crier server in relaying a response to the application contract.
``msg.value`` is the amount of wei a requester pays and is recorded as ``Request.fee``.

.. code-block:: javascript

  deliver(uint64 requestId, bytes32 paramsHash, uing64 error, bytes32 respData) public;

After fetching data and generating the response for the request with ``requestId``, TC sends a transaction calling function ``deliver()``.
``deliver()`` verifies that the function call is made by SGX and that the hash of query parameters is correct.
Then it calls the callback function of the application contract to transmit the response.

The response includes ``error`` and ``respData``.
If ``error = 0``, the application contract request has been successfully processed and the application contract can then safely use ``respData``.
The fee paid by the application contract for the request is consumed by TC.
If ``error = 1``, the application contract request is invalid or cannot be found on the website.
In this case, similarly, the fee is consumed by TC.
If ``error > 1``, then an error has occured in the Town Crier server.
In this case, the fee is fully refunded but the transaction cost for requesting by the application contract won't be compensated.

.. code-block:: javascript

  cancel(uint64 requestId) public returns(bool);


A requester can cancel a request whose response has not yet been issued by calling function ``cancel()``.
``requestId`` is required to specify the query.
The fee paid by the Appliciation Contract is then refunded (minus processing costs, denoted as cancellation fee).

For more details, you can look at the source code of the contract [TownCrier.sol].
