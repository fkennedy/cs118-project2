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
    int rc; // return code
    int sockfd; // socket
    char* portno; // port number to listen on
    char* hostname; // hostname
    char* filename; // filename

    // Server
    struct sockaddr_in serverinfo;

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

    // Set server info
    memset(&serverinfo, 0, sizeof serverinfo);
    serverinfo.sin_family = AF_INET;
    serverinfo.sin_port = htons(portno);
    serverinfo.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Create a socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == RC_ERROR)
        error("ERROR: could not create a socket.");

    // Send file name
    if ((sendto(sockfd, filename, strlen(filename), 0, &serverinfo, sizeof(serverinfo))) == RC_ERROR) {
        error("ERROR: Failed to send filename.");
    }

    return RC_SUCCESS;
}

// Helper functions
void error(char *msg) {
    perror(msg);
    exit(RC_ERROR);
}