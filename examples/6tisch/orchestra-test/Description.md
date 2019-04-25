A simple RPL network with UDP communication. This is a self-contained example:
it includes a DAG root (`udp-server.c`) and DAG nodes (`udp-client.c`).

The DAG root also acts as UDP server and sink node. The DAG nodes are UDP client. The clients
send a UDP request periodically, that simply includes a counter as payload.
When receiving a request, The sink node collects the requests from the DAG nodes.

The `.csc` files show example network in the Cooja simulator for cooja motes.


