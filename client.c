#include <stdio.h>
#include <sys/types.h>  // definitions of a number of data types used in socket.h and netinet/in.h
#include <sys/socket.h> // definitions of structures needed for sockets, e.g. sockaddr
#include <netinet/in.h> // constants and structures needed for internet domain addresses, e.g. sockaddr_in
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "packet.h"

// Return codes
#define RC_SUCCESS  0
#define RC_EXIT     1
#define RC_ERROR    -1

// Constants
#define MAX_SEQ_NO  30720
#define WINDOW_SIZE 5120
#define TIME_OUT    500
#define HEADER_SIZE 20

// Function headers
void error(char * msg);

// Main
int main(int argc, char* argv[]) {
    // Declare variables
    int sockfd; // socket
    int portno; // port number to listen on
    char* hostname; // hostname
    char* filename; // filename

    // Server
    struct sockaddr_in serv_addr; // server's address
    int servlen; // byte size of server's address
    struct hostent *server; // server host info

    // Buffer
    char buffer[PACKET_SIZE]; // message buffer

    // Validate args
    if (argc != 4) {
         fprintf(stderr, "Usage: %s <hostname> <port> <filename>\n", argv[0]);
         exit(RC_SUCCESS);
    }

    // Get host name
    hostname = argv[1];
    // Get port number
    portno = atoi(argv[2]);
    // Get file name
    filename = argv[3];

    // Create parent socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == RC_ERROR)
        error("ERROR: Could not open socket\n");

    // Get host by name
    server = gethostbyname(hostname);
    if (server == NULL)
        error("ERROR: Could not find host\n");

    // Build server's internet address
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char*) server->h_addr, (char*) &serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);
    servlen = sizeof(serv_addr);

    if ((sendto(sockfd, filename, strlen(filename), 0, server->h_addr, server->h_length)) == RC_ERROR) {
        printf("h_addr: %s\n", server->h_addr);
        printf("sockfd: %d\nfilename: %s\nstrlen(filename): %d\n servlen: %d\n", sockfd, filename, strlen(filename), servlen);
        error("ERROR: Failed to send filename.\n");
    }

    return RC_SUCCESS;
}

// Helper functions
void error(char *msg) {
    perror(msg);
    exit(RC_SUCCESS);
}