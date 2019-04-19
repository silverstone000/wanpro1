Router part:
 =======

Input argument: <router id>

# Program structure

The main class storages all shared data structures;

Taking controller information for forwarding module to make connection.

It runs on main thread, block after starting all other modules with their separate thread.

It maintains three classes: neighbour, lsa and forwarding.

## Neighbour module:

It maintains a UDP echo server, for other routers’ cost measurements. 
The main thread processes `connect()` and `disconnect()` messages, start or end cost measurement thread as well as inform lsa module the change in connectivity by filling its message queue.
Cost measurement method using ICMP like method to send packet to others’ UDP server. For each neighbour, it uses a separate thread. 

The cost value is scaled from 0 to max timeout value to 0 to 65535.

The cost value is smoothed using Exponential smoothing with a smooth factor of 0.9. 

Each time a timeout of cost measurement happens, the cost is calculated using 2 times the maximum cost value in smoothing.

## Lsa module:

It maintains two link-state databases, one for modifying, one as snapshot for routing table calculation. 

The main thread processes the module’s message queue and processes ls advertisement message queue, including ack, adv and resend operation.

A thread periodically updates routing table using swapped link-state database. 

A thread periodically sends link-state database to all known and connected neighbours including processing ack message and resend on time-out ones.(by sending specific messages to forwarding module’s message queue)

## Forwarding module:

First the `initialize()` method is called in main thread to create a router configuration file in a public folder and initialize id table for ip, port look up, also, a TCP server is started for incoming messages.

To send a message, a TCP client is initiated for each message. The connection is closed after transmission completed.

An incoming TCP connection is seen as a TCP session which is handled by `tcp_session`, parse of the content happens inside the TCP session. 

# Details

### Shared Resources

 * `route_table`
 
 Maintained by lsa module.
 Accessed by forwarding module for next hop lookup.
 In form of a map with destination id as key and next hop id as value.
 
 * `id_table`

A lookup table from router id to ip, port pair lookup.(A map with router id as key and ip address and port pair as value)

Accessed by forwarding and neighbour module.

Maintained by forwarding module.

* Message Queues for Each module

Each module has its message queue hosted in the main module for itself and other modules to access.

Message queues are FIFO queues, storing specified `struct` object in it.

### Neighbour Resources

* `cost_map`

Recording cost information with neighbours.

Used and maintained inside neighbour module only.

### Lsa Resources

* `lsa_db1 lsa_db2`

Two LS database, one in continuous update, one as snapshot for routing table calculations. 

They swap rows when calculation of a new routing table is needed. Swapping take place in the form of exchange access pointers pointing to them.

LS databases are in form of a map with router id as key and cost as value hosting in a map with router id as key and that map as value. (`map<ROUTER_ID, map<ROUTER_ID, int>>`)

LSA messages contain cost information also hold similar data structure with a map with router id as key and a map with router id as key and cost as value as value.

e.g.
`		{
			"1" :
			{
				"2":10
			},
			"2" :
			{
				"1":12
			}
		}`

Where `1` and `2` are ids and `12` and `10` are costs

* `sent_state`

It records the acknowledgement status of sent ls advertisement.

It is initialized based on a snapshot of id table. 

## Routing table calculation

Routing table is calculated periodically if ls database has changed.

Dijkstra's algorithm is used for calculation.

The Dijkstra's algorithm implementation uses a priority queue for fast retrieve of current node.

Next-hop calculation integrated into algorithm running routine in the way that when finded a route with shorter distance for neighbour node, check if current node is the source node, if it is, then set next hop of neighbour node to neighbour node, else set next hop to `next_hop[current node]` (current node’s next hop). 

## Network interface

Communication between routers and between controller and routers uses TCP links.

Each router runs a UDP server for delay measurement. 

## Router naming

Each router is identified by a router id assigned through input argument of the program. The router will put a configuration file containing router id, ip address of the machine it is running and the port it starts its server in a public folder. These files are read to gain information about other routers.

The ip address is obtained through using system command `ifconfig` with a regular expression to extract the ip address of first interface of the machine.

## Messages

Messages are serialized into json format from internal data structure for transmission.

Three types of messages are used. Type 1 for communication between routers. Type 2 for communication with controller. Type 3 for file transfer through the network.

## File transfer support

Using the type 3 message, file transfer program can transfer arbitrary contents to target by specifying the router id of target router with its corresponding file transfer program. Type 3 message contains a data field for arbitrary content delivery.

## Race condition handling

Multiple threads may have race condition on shared resources.

Several mutex objects as shared resource in main module are used to solve race condition.

Whenever a shared resource needed to be accessed, the corresponding mutex object is locked. 

## Parameter Setting

Most parameters are set using macros in header file. 

The input argument of router program is router’s id.

## Program Human Interaction

The main router program accepts information to connect to other routers and send error message if any. Information of routing table and cost of links are printed periodically. 

