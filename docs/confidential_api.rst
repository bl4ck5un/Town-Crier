Query API documentation
========================

We use ECDH (on ``SECP256K1``) + AES-256 hybrid encryption for users to encrypt their queries.

Our public key is (base64)
``BIHqhi/x96ZLLFhDH7/4QPLYn/KTj1i1wRdQ7jcDIPknVs6E+ILJdZpw8yX2j+QhFKnJnnJDOiesCDEK2qVr7Rw=``

We built a `web-based tool`_ to facilitate the encryption.
We refer users to https://github.com/bl4ck5un/hybrid-enc/ for a reference
implementation of the above encryption scheme.


Flight Departure Delay
-----------------------

query content
~~~~~~~~~~~~~~~

.. code-block:: json

  {"flight": "ABC", "time": 123123123}


example encryption output
~~~~~~~~~~~~~~~~~~~~~~~~~~



Steam Trade
------------

query content
~~~~~~~~~~~~~~~

.. code-block:: json

  {
    "api_key": "0xdeadbeef",
    "buyer_id": "dummy_id",
    "cutoff_time": "148889898",
    "items": ["item1","item2"]
  }

example encryption output
~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: shell

  ['BK1OC1KkBi1ZlMJKh9NIPVp8sPaYqqbC', 'kRhJdQVuWSZA9yX2xbhzB9HcPrvr/gr/', 'Aids9oUSwQbxdrIJjEfI0SGZAAAAAAAA', 'AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA', 'ABp5WAPVjfXpNNbZZCWNgLbIJweK0QwW', 'kNeoR7BT3tl5OY6AA11rhPxZ7sJRtKzW', 'f+kY+SCT7jJHAVhVAD+cKDXc2LWX6c4z', 'aUJhPjeA/HHa304HeP8lGQJApVx+g30U', 'rh1O58/2JeeNhjurTgQp8kOESo+VVlrx', 'z/w=']

.. _web-based tool: https://github.com/bl4ck5un/Town-Crier/tree/master/utils/encrypt
