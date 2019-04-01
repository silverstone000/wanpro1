Router part:
 =======

Input argument: [controller address] [port]

## Program structure

Main module, used to storage all shared data structure;

Taking controller information for forwarding module to make connection.

It runs on main thread, block after starting all other modules with their separate thread.

## Three submodules: neighbour, lsa and forwarding

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

## Details

### Shared Resources

 * `route_table`
 
 Maintained by lsa module.
 Accessed by forwarding module for next hop lookup.
 In form of a map with desitination id as key and next hop id as value.
 
 * `id_table`


