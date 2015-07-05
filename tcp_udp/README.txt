Eugene Sokolov
January 4, 2013
Recreation of tcp and udp functions, implementing the API sockets protocol

This set of programs is meant to send and recieve messages via TCP and UDP interfaces

Usage:
1. make
2. Open a client by running ./tcpclient port
3. Open a server by running ./tcpserverhostname port
4. Communicate from the client to the server (1 way)

To create a proxy and communicate in both directions
1. make
2. Open a proxy by running ./proxy port
3. Connect from the server to the proxy by connection to that port
4. Open the client and connect to a different port
5. Connect to proxy and the client
6. Communicate between the client and the server (note that they are connection via the proxy and are not connected directly to each other)
