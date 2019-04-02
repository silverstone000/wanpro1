Router part:
 =======

Input argument: [controller address] [port]

# Program structure

Main module, used to storage all shared data structure;

Taking controller information for forwarding module to make connection.

It runs on main thread, block after starting all other modules with their separate thread.

It has three submodules: neighbour, lsa and forwarding.

## Neighbour module:

It maintains a UDP echo server, for other routers’ cost measurements. 
The main thread processes `connect()` and `disconnect()`, start or end cost measurement thread as well as inform lsa module the change in connectivity by filling its message queue.
Cost measurement method using ICMP like method to send packet to others’ UDP server. For each neighbour, it uses a separate thread.

## Lsa module:

It maintains two link-state databases, one for modifying, one as snapshot for routing table calculation. 

The main thread process the module’s message queue, process ls advertisement messages, including ack, adv and resend operation.

A thread periodically updates routing table using swapped link-state database.

A thread periodically sends link-state database to all know neighbours including process ack message and resend time-out one.(by sending specific messages to forwarding module’s message queue)

## Forwarding module:

First the `initialize()` method is called in main thread to get a router id from controller and initialize id table for ip,port look up, also, a tcp connection server is started for incoming connection.

The main thread maintain sockets as connections, it can either started by local router or remote router, depending on who started first. It process messages in json format and transform them into internal data structure for other modules to process. 

For message from controller, packets have no destination field, the receiver sees itself as destination. For message from other routers, it has destination field for look up in routing table and forward to other routers if the destination is not itself. 

# Details

### Shared Resources

 * `route_table`
 
 Maintained by lsa module.
 Accessed by forwarding module for next hop lookup.
 In form of a map with destination id as key and next hop id as value.
 
 * `id_table`

A lookup table for router id to ip, port pair lookup.(A map with router id as key and tcp endpoint(with information of ip address and port, half open socket) as value)

Accessed by forwarding and neighbour module.

Maintained by forwarding module.

* Message Queues for Each module

Each module has its message queue hosted in the main module for itself and other modules to access.

Message queues are in form of fifo queues storing specified `struct` object in it.

### Neighbour Resources

* `cost_map`

Recording cost information with neighbours.

Used and maintain inside neighbour module only.

### Lsa Resources

* `lsa_db1 lsa_db2`

Two LS database, one in continuous update, one as snapshot for routing table calculations. 

They swap rows when calculation of a new routing table is needed. Swapping take place in the form of exchange access pointers pointing to them.

LS databases are in form of a map with router id as key and cost as value hosting in a map with router id as key and that map as value. (`map<ROUTER_ID, map<ROUTER_ID, int>>`)

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

One router program holds a connection with controller and one for each neighbour. 

Messages are serialized into json format for transmission.

## Race condition handling

Multiple threads may have race condition on shared resources.

Several mutex objects as shared resource in main module are used to solve race condition.

Whenever a shared resource need to be accessed, the corresponding mutex object is locked. 

## Parameter Setting

Most parameters are set using macros in header file. 

## Program Human Interaction

The main router program only accepts information to connect to controller and send error message if any. Information can be accessed on controller by using controller to issue queries towards router. 

