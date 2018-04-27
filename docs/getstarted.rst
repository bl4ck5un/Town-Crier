Get Started: Write Your First TC-powered Contract
=================================================

The TC contract has a very simple API for your contract to call.

Submit queries via ``request``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

An application contract sends queries to TC by calling the ``request`` function.

.. code-block:: javascript

  request(uint8 requestType, address callbackAddr, \
        bytes4 callbackFID, uint256 timestamp, \
        bytes32[] requestData) public payable returns(int256);

Parameters:
  - ``requestType``: indicates the query type (see below).
  - ``callbackAddr``: the address of the recipient contract (see below).
  - ``callbackFID``: the callback function selector (see below).
  - ``timestamp``: reserved. Unused for now.
  - ``requestData``: data specifying query parameters. The format depends on the query type.

.. warning::
  Calling contracts must prepay the gas cost incurred by the Town Crier server in delivering a response to the application contract  properly. The calling contract is responsible to set ``msg.value`` properly.

Return: The function returns an ``int256`` value denoted as ``requestId``.

  - If ``requestId > 0``, then the request is successfully submitted. The calling contract can use ``requestId`` to check the response or status.
  - If ``requestId = -2^250``, then the request fails because the requester didn't send enough fee to the TC Contract.
  - If ``requestId = 0``, then the TC service is suspended due to some internal reason. No more requests or cancellations can be made but previous requests will still be responded to by TC.
  - If ``requestId < 0 && requestId != -2^250``, then the TC Contract is upgraded and requests should be sent to the new address ``-requestId`` (negative of ``requestId``).


Canceling requests via ``cancel``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: javascript

  cancel(uint64 requestId) public returns(bool);

Unprocessed requests can be canceled by calling function ``cancel(requestId)``.
The fee paid by the requester is refunded (minus processing costs, denoted as cancellation fee).

For more details about how Town Crier contract works, you can look at the source code of the contract TownCrier.sol_.

Receiving responses from TC
~~~~~~~~~~~~~~~~~~~~~~~~~~~

To receive a response from TC, the requester need to specify the recipient contract as well as the recipient function.

.. warning::
  Very importantly, TC requires that the recipient function to have the following signature.

.. code-block:: javascript

  function FUNCTION_NAME(uint64 requestId, uint64 error, bytes32 respData) public;

This is the function that will be called by the TC Contract to deliver the response from TC server.

To do so, you should set the ``callbackFID`` parameter to ``bytes4(sha3("FUNCTION_NAME(uint64,uint64,bytes32)"))``, namely the selector_ of your ``FUNCTION_NAME`` function.

Parameters: ``error`` and ``respData``.
  - If ``error = 0``, the request has been successfully processed and the calling contract can then safely use ``respData``. The fee paid is consumed by TC.
  - If ``error = 1``, the provided request is invalid or cannot be found on the website. In this case, similarly, the fee is consumed by TC.
  - If ``error > 1``, then an error has occured in the Town Crier server. In this case, the fee is fully refunded but the transaction fee (for making this call).


.. _TownCrier.sol: https://github.com/bl4ck5un/Town-Crier/blob/master/contracts/TownCrier.sol
.. _selector: https://solidity.readthedocs.io/en/latest/abi-spec.html#function-selector
