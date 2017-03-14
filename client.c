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

    // Packets
    struct packet packetReceive;
    struct packet packetRequest;
    struct packet packetSend;

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

    // 3-Way Handshake
    // Send SYN
    struct packet syn;
    SYN.type = 2;
    SYN.SEQ = 0;
    if (sendto(sockfd, &SYN, sizeof(SYN), 0, (struct sockaddr *) &serv_addr, servlen) == RC_ERROR)
        error("ERROR: Could not initiate 3-way handshake\n");

    // Receive SYN-ACK
    while (1) {
        bzero((char *) &packetReceive, sizeof(packetReceive));
        if (recvfrom(sockfd, &packetReceive, sizeof(packetReceive), 0, (struct sockaddr *) &serv_addr, (socklen_t *) &servlen == RC_ERROR))
            error("ERROR: SYN-ACK packet lost\n");
        else {
            if (packetReceive.type == 1 && packetReceive.ACK == -1)
                break;
            else
                error("ERROR: ERROR: Could not receive SYN-ACK packet\n");
        }
    }

    // Send ACK
    // Send request packet
    packetRequest.type = 4;
    packetRequest.SEQ = 0;
    packetRequest.ACK = -1;
    packetRequest.size = strlen(filename);
    strcpy(packetRequest.data, filename);

    if (sendto(sockfd, &packetRequest, sizeof(packetRequest), 0, (struct sockaddr *) &serv_addr, servlen) == RC_ERROR)
        error("ERROR: Could not send request packet\n");

    return RC_SUCCESS;
}

// Helper functions
void error(char *msg) {
    perror(msg);
    exit(RC_SUCCESS);
}