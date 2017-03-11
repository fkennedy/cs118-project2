#include <stdio.h>
#include <sys/types.h>  // definitions of a number of data types used in socket.h and netinet/in.h
#include <sys/socket.h> // definitions of structures needed for sockets, e.g. sockaddr
#include <netinet/in.h> // constants and structures needed for internet domain addresses, e.g. sockaddr_in
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>

// Return codes
#define RC_SUCCESS 1
#define RC_ERROR -1

// Constants
#define BUFFER_SIZE 1024

// Function headers
void error(char * msg);

// Main
int main(int argc, char* argv[]) {
    // Declare variables
    int sockfd; // socket
    int portno; // port number to listen on
    struct sockaddr_in serv_addr; // server's address
    socklen_t servlen = sizeof(serv_addr); // byte size of server's address
    struct sockaddr_in cli_addr; // client's address
    socklen_t clilen = sizeof(cli_addr); // byte size of client's address

    // Validate args
    if (argc != 2) {
         fprintf(stderr,"Usage: server %s <port>\n", argv[0]);
         exit(RC_SUCCESS);
    }

    // Get port number
    portno = atoi(argv[1]);

    // Create parent socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == RC_ERROR)
        error("ERROR: Could not open socket");

    // Get server address
    bzero((char *) &serv_addr, servlen);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    // Bind socket
    if (bind(sockfd, (struct sockaddr *) &serv_addr, servlen) == RC_ERROR)
        error("ERROR: Could not bind");

    return RC_SUCCESS;
}

// Helper functions
void error(char *msg) {
    perror(msg);
    exit(RC_SUCCESS);
}