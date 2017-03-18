# CS118 Project 2

## Authors

* Frederick Kennedy (404667930) Discussion 1D
    * Worked on the server
* Johnathan Estacio (404491851) Discussion 1B
    * Worked on the client and the report

## Getting started
### Source code
* server.c - C source code for server
* client.c - C source code for client
* helper.h - Header file for helper.c and all the necessary information needed for both server and client
* helper.c - C source code for helper functions
* Makefile - Makefile of the project
        
### Files
* image.jpg - File of size > MB
* spec.pdf  - File of size > KB
* test.html - File of size > B
* report.pdf - Report of the project

### Building the program
To build the server and client programs:
```
    make
```
To clean up the directory:
```
    make clean
```
To create a tarball

### Running the program
To run the server:
```
    ./server <port number>
```
To run the client:
```
    ./client <host name> <port number> <file name>
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