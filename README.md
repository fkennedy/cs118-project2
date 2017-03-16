# CS118 Project 2

## Authors

* Frederick Kennedy (404667930)
* Johnathan Estacio (404491851)

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

## Notes
* [Socket Tutorial](http://www.linuxhowtos.org/C_C++/socket.htm)
* [3-Way Handshake](http://www.inetdaemon.com/tutorials/internet/tcp/3-way_handshake.shtml)
* [_exit(3)_](http://man7.org/linux/man-pages/man3/exit.3.html)
* [_recvfrom(2)_](https://linux.die.net/man/2/recvfrom)
* [_sendto(2)_](https://linux.die.net/man/2/sendto)
* [_gethostbyname(3)_](http://man7.org/linux/man-pages/man3/gethostbyname.3.html)
* [_gethostbyaddr(3)_](https://linux.die.net/man/3/gethostbyaddr)
* [_fopen(2)_](https://linux.die.net/man/3/fopen)
* [_fread(3)_](http://man7.org/linux/man-pages/man3/fread.3.html)
* [_memset(3)_](http://man7.org/linux/man-pages/man3/memset.3.html)
* [do...while](https://www.tutorialspoint.com/cprogramming/c_do_while_loop.htm)