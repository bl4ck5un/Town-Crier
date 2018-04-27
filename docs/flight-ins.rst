Flight Insurance Contract
===========================

The ``Application`` Contract above is only able to send queries to and receive responses from TC.
For many real-life applications there is one critical factor missing: **timing**.
A stock exchange contract which settles at a specified time needs to know the stock quote for that time.
A trip insurance contract which needs to find out whether a flight departs on time must fetch the flight state after the scheduled departure time.
A contract for the sale of a physical good has to wait for a period for delivery after payment to check whether the good was successfully delivered.
The immediate response TC currently supports cannot directly fulfill such requirements in these applications, but
developers can use other strategies to get around this limitation. (Future versions of TC will allow for pre-specified, future query times using the ``timestamp`` parameter.)

Here we present a design for a flight insurance application that illustrates use of the full existing range of TC features.

Application setting
--------------------

Suppose Alice wants to set up a flight insurance service and creates a smart contract ``FlightInsurance`` for this purpose.
A user can buy a policy for his flight from Alice by sending money to the ``FlightInsurance`` Contract.
``FlightInsurance`` offers a payout to the user should his insured flight be delayed or cancelled.
(Unfortunately, TC cannot detect whether you've been senselessly beaten and dragged off your flight by United Airlines.)

Problem with the ``Application`` Contract
------------------------------------------

The ``FlightInsurance`` Contract contains the same five basic components found above in the ``Application`` Contract.
However, we don't want to query TC immediately after a user, say Bob, purchases a policy. If we do so, this will result in one of two bad cases.
One is that Bob purchases a policy for a flight that has already been delayed or cancelled, which is unfair to Alice.
The other is that Bob purchases a policy before the scheduled departure time of his flight so when the ``FlightInsurance`` Contract queries immediately, it will get response of "not delayed" since the flight hasn't yet left. This is unfair to Bob.

A scheme to get around the problem
----------------------------------------

To address this problem, we can separate the two operations and deal with them at different times.
The ``FlightInsurance`` Contract needs to include a ``Insure()`` function for a user to buy a policy for his flight a certain period ahead of the scheduled departure time, say 24 hours.
The contract can simply compare ``block.timestamp`` with the flight data to guarantee this restriction.
If the request is valid, then the contract will store the flight data and issue a policy ID to the user for querying TC later.
When the flight departs, the user can send a transaction to ``Request()`` with the policy ID in the ``FlightInsurance`` Contract, and the contract will query TC.
Then everything works much as in the ``Application`` Contract.

You can take a look at the ``FlightInsurance.sol`` for the complete ``FlightInsurance`` Contract logic.
