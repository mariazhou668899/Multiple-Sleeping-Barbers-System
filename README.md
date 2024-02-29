# Operating System Programming - socket programming: 

1. Purpose
This assignment is intended for three purposes: (1) to utilize various socket-related system calls, (2) to create a multi-threaded server and (3) to evaluate the throughput of different mechanisms when using TCP/IP do to do point-to-point communication over a network.

2. Client-Server Model
In this program you will use the client-server model where a client process establishes a connection to a server, sends data or requests, and closes the connection.  The server will accept the connection and create a thread to service the request and then wait for another connection on the main thread.  Servicing the request consists of (1) reading the number of iterations the client will perform, (2) reading the data sent by the client, and (3) sending the number of reads which the server performed.   

3.  Program Specification
Write a Client.cpp and Server.cpp that establishes a TCP connection and sends buffers of data from the client to server.   To start, the client sends a message to the server which contains the number of iterations it will perform (each iteration sends 1500 bytes of data).  When the server has read the full set of data from the client it will send an acknowledgment message back which includes the number of socket read() calls performed. 
The client will send the data over in three possible ways depending on the type of test being performed (see below for details of the three tests).

