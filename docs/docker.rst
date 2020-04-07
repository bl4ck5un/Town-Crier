How to use various docker images
================================

The preferred way to launch TC is via docker services.
Of course, you'll need `docker <https://docs.docker.com/get-docker/>`_ and `docker-compose <https://docs.docker.com/compose/install/>`_ properly installed.

Setup
~~~~~

First, get the docker service files from

.. code-block:: bash

    git clone https://github.com/bl4ck5un/Town-Crier-docker-sevices
    cd rinkeby

The current version of TC uses Infura as a Web3 provider to access Rinkeby testnet.
(Support for other networks as well as private nets are being added.)
To use Infura, set the following environment variables (or put them in a ``.env`` file):

.. code-block::

    # .env
    WEB3_INFURA_PROJECT_ID=your project id
    WEB3_INFURA_API_SECRET=your api secret

.. warning::
    Keep ``WEB3_INFURA_PROJECT_ID`` and ``WEB3_INFURA_API_SECRET`` secrets.


Launch TC
~~~~~~~~~

Thanks to docker-compose, TC---the backend as well as the relay---can be launch together in one command: ``docker-compose up``.
To run it in the background, use ``docker-compose up -d``.
Other advanced uses of ``docker-compose`` can be found in their documentation.

There is a convenience Makefile that wraps around common docker-compose commands.
For example, you can type ``make up`` instead of ``docker-compose up``.