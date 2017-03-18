#ifndef helper
#define helper

#include <stdio.h>
#include <sys/types.h>  // definitions of a number of data types used in socket.h and netinet/in.h
#include <sys/socket.h> // definitions of structures needed for sockets, e.g. sockaddr
#include <netinet/in.h> // constants and structures needed for internet domain addresses, e.g. sockaddr_in
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <time.h>

// Return codes
#define RC_SUCCESS  1
#define RC_ERROR    -1

// Constants
#define MAX_SEQ_NO 30720
#define WINDOW_SIZE 5120
#define TIME_OUT 500
#define HEADER_SIZE 20
#define PACKET_SIZE 1024
#define PAYLOAD_SIZE 1004

struct packet {
    unsigned int location;
    unsigned int location_next;
    int SEQ;
    int ACK;
    int ACKed;
    int size;
    struct timespec timer;
};

// Function headers
void intHandler(int sig);
void error(char* msg);
int sendTo(int sockfd, char* buffer, size_t size, struct sockaddr *dest_addr, socklen_t destlen, int SEQ, int SIN, int FIN, unsigned int start);
int recvFrom(int sockfd, char* buffer, int* size, struct sockaddr *src_addr, socklen_t *srclen, int* SEQ, int* SIN, int* FIN, unsigned int* start);
int add(int* list, int ACK);

#endif