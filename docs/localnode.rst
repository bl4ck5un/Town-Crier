How to set up a local Town Crier instance
=========================================

There are three components

- An C++ SGX backend (i.e., the enclave)
- A Python relay
- A smart contract frontend (i.e., the TC contract)

There are three simple steps to set up a Town Crier node and hook it up with an example relying contract:

1. Set up the Town Crier smart contract, and the relying smart contract
2. Launch Town Crier
3. Send a bunch of requests to Town Crier and see it working!

We'll go through the process step by step.

Set up the smart contracts
~~~~~~~~~~~~~~~~~~~~~~~~~~

In this step, we first create a local blockchain for testing, then deploy the Town Crier smart contract and an
application smart contract on it.

For this purpose you usually want to create a `private net`_ by setting ``networkid`` to an integer other than
1 to 4, and (optionally) disabling peer-to-peer discovery and other unnecessary features.

We provided a set of scripts to make the process easier. Feel free to follow alone or tweak it to your needs.

1. Create or clone a blockchain using ``geth``.


    .. code-block:: bash

        # start in the root directory
        cd scripts/privatenet
        make console  # this does step 1 - 3

2. Deploy the TC contract.

    .. code-block:: bash

        # IN THE GETH CONSOLE
        > tc = create_tc();
        # this could take a while for the first time (2 minutes on my laptop).
        # Wait until "Generating DAG in progress" ends.
        # When finished, you'll see a message saying
        # Town Crier created at: 0x18322346bfb90378ceaf16c72cee4496723636b9 (the address may be different)

3. Create an Application contract to test Town Crier.

    .. code-block:: bash

        # in the same geth console
        > app = createApp(tc.address);

        # set up logging for both contracts
        > setup_log(tc);
        > watch_events(tc);
        > watch_events(app);

Launch Town Crier
~~~~~~~~~~~~~~~~~

Keep the geth console open, now open a new terminal to launch Town Crier.
This is as simple as:

.. code-block:: bash

    cd dockerfiles/services
    make up-debug


Send a bunch of queries
~~~~~~~~~~~~~~~~~~~~~~~

Now go back to the ``geth`` console.

.. code-block:: bash

    > request(app, 5, ['bitcoin']);

.. _private net: https://geth.ethereum.org/docs/interface/peer-to-peer

