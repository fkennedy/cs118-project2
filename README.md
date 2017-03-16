# CS118 Project 2

## Authors

* Frederick Kennedy (404667930)
* Johnathan Estacio (404491851)

## NOTES
* We need to implement a check for when the ACK is corrupted .. if we care enough to get those points.
..* Maybe can use checksums on the ACK?

## TODO
"Implement a simple window-based, reliable data transfer protocol
built on top of Selective Repeat protocol."

### Client
* Create a new buffer called fileBuffer to hold received packets (only needs to have a length of ~1024, the packet length)
  * Create a new file using fopen(filename, "+w") and store in a file pointer called fp
* Create a while loop to run until file transmission is done: while(!fin)
* Cases for receiving a packet (recv(2))
  1. numBytesReceived < 0 => File packets not received
  2. Received a packet out of order => We can either ignore it (retransmit) OR we can send an ACK with the seq number telling the server that we expect that packet next.
    * Let's keep track of the sequence number we expect in expectedSeqNum
    * Increment expectedSeqNum by the fileBufferLen at the end of each iteration
  3. fin = 1 => Finished, so send ACK and write fileBuffer to fp
  4. Packet is normal => Write data to file (fwrite)

### Server
* Read in file and add to a buffer called fileBuffer
  * Check if have file, if not send response
  * Get length of file and store into fileLen
* Create a while loop to run until file transmission is done: while(!fin)
* Create a while loop inside to send packets until the end of the sending window
  * Check that there are more packets to send 
    * Keep an index tracking the current position in the file we are at called currPos
      * Adjust according to the current windowPos
* Copy next part of file into a packet buffer to send
* Increment currPos by the packet length and numPacketsSent by 1
* Create another while loop inside (outside the previous) to wait on ACKs from sockets (select(2))
  * Use a timeout interval to wait on (pass to select(2))
    * If timeout, just break and continue the outer loop (resend packets)
  * Fetch the ACK from the socket (recv(2))
  * Cases
    1. fin = 1 => Finished, break
    2. ACK is corrupted => ignore it
    3. ACK is the one we were expecting (is within the window) => Increment both windowPos and expectedACK by the packet length.

## Getting started
### Files
* server.c - C source code for server
* client.c - C source code for client
* packet.h - Header file for a packet

### Building the program
To build the server and client programs:
```
    make
```
To build the server program:
```
    make server
```
To build the client program:
```
    make client
```
To clean up the directory:
```
    make clean
```

### Running the program
To run the server:
```
    ./server <port number>
```
To run the client:
```
    ./client <port number> <host name> <file name>
```

## References
* [Socket Tutorial](http://www.linuxhowtos.org/C_C++/socket.htm)
* [3-Way Handshake](http://www.inetdaemon.com/tutorials/internet/tcp/3-way_handshake.shtml)
* [_exit(3)](http://man7.org/linux/man-pages/man3/exit.3.html)
* [_recvfrom(2)_](https://linux.die.net/man/2/recvfrom)
* [_sendto(2)_](https://linux.die.net/man/2/sendto)
* [_gethostbyname(3)_](http://man7.org/linux/man-pages/man3/gethostbyname.3.html)
* [_gethostbyaddr(3)_](https://linux.die.net/man/3/gethostbyaddr)
* [_fopen(2)_](https://linux.die.net/man/3/fopen)
* [_fread(3)_](http://man7.org/linux/man-pages/man3/fread.3.html)
