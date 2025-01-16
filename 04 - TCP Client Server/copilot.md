Write two C++ programs named client and server. 

The server should listen to IPv4 TCP Port 9001 for connections on all interfaces. 

It should accept connections and set socket options for non-blocking. 

It should use select() to monitor the currently open sockets.

For each accepted connection, it should create a file prefixed with /tmp/file_ where it will store all data received by the corresponding client. 

Upon accepting a connection from a client, the server should respond with "ready\n". 

The client should read the contents of a file where the name is provided as an argument on the command line and connect to the IP address and port specified by another argument. 

it should wait for the message "ready\n" and proceed to transmit the contents of the specified file to the server.