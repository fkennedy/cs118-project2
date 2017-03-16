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
#define RC_SUCCESS  0
#define RC_EXIT     1
#define RC_ERROR    -1

// Constants
#define MAX_SEQ_NO  30720
#define WINDOW_SIZE 5120
#define TIME_OUT    500
#define HEADER_SIZE 20
#define PACKET_SIZE 1024

// Function headers
void error(char * msg);
int recvfromwithheaders(int sockfd, void *buf, size_t len, int flags, 
        struct sockaddr *src_addr, socklen_t *addrlen, char* fileBuffer, int* ack, int* fin, int* expectedSeqNum, int* ackChecksum, int* datasize);

// Main
int main(int argc, char* argv[]) {
    // Validate args
    if (argc != 4) {
         fprintf(stderr, "Usage: %s <hostname> <port> <filename>\n", argv[0]);
         exit(RC_SUCCESS);
    }

    // Declare variables
    int rc; // return code
    int sockfd; // socket
    int portno; // port number to listen on
    char* hostname; // hostname
    char* filename;

    // Server
    struct sockaddr_in serverinfo;
    socklen_t servlen = sizeof(serverinfo);

    // Get host name
    hostname = argv[1];
    // Get port number
    portno = atoi(argv[2]);
    // Get file name
    filename = argv[3];

    // Set server info
    memset(&serverinfo, 0, servlen);
    serverinfo.sin_family = AF_INET;
    serverinfo.sin_port = htons(portno);
    serverinfo.sin_addr.s_addr = INADDR_ANY;

    // Create a socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == RC_ERROR)
        error("ERROR: could not create a socket.");
    else
        printf("Socket created\n");

    // Send file name
    if ((sendto(sockfd, filename, strlen(filename), 0, (const struct sockaddr *) &serverinfo, servlen)) == RC_ERROR)
        error("ERROR: Failed to send filename.");
    else
        printf("File name sent\n");

    // File variables
    char fileBuffer[PACKET_SIZE]; // message buffer
    FILE *fp = fopen(filename, "+wb");

    // Variables
    int numBytesReceived = 0;

    // Headers
    int fin = 0;
    int ack = 0;
    int ackChecksum = 0;
    int datasize = 0;
    int expectedSeqNum = 0;

    while (!fin) {
        if ((numBytesReceived = recvfromwithheaders(sockfd, fileBuffer, PACKET_SIZE, 0, 
                (struct sockaddr *) &serverinfo, &servlen, fileBuffer, &ack, &fin, &ackChecksum, &datasize, &expectedSeqNum)) == RC_ERROR)
            error("ERROR: receiving packet.");

        printf("%s\n", fileBuffer);
    
        expectedSeqNum += PACKET_SIZE;
    }

    return RC_SUCCESS;
}

// Helper functions
void error(char *msg) {
    perror(msg);
    exit(RC_ERROR);
}

int recvfromwithheaders(int sockfd, void *buf, size_t len, int flags, 
        struct sockaddr *src_addr, socklen_t *addrlen, char* fileBuffer, int* ack, int* fin, int* expectedSeqNum, int* ackChecksum, int* datasize) {
    
    int numBytesReceived;
    int traverseIndex = 0;

    printf("HERE");

    if ((numBytesReceived = recvfrom(sockfd, buf, len, flags, src_addr, addrlen)) == RC_ERROR)
        error("ERROR: did not receive packet correctly.");

    printf("HERE2");

    // Packet stucture: 
    // | ack | fin | expectedSeqNum | ackChecksum | datasize | fileBuffer |

    char* traverse = buf;

    memcpy(&ack, traverse + traverseIndex, sizeof(ack));
    traverseIndex += sizeof(ack);

    memcpy(&fin, traverse + traverseIndex, sizeof(fin));
    traverseIndex += sizeof(fin);

    memcpy(&expectedSeqNum, traverse + traverseIndex, sizeof(expectedSeqNum));
    traverseIndex += sizeof(expectedSeqNum);

    memcpy(&ackChecksum, traverse + traverseIndex, sizeof(ackChecksum));
    traverseIndex += sizeof(ackChecksum);

    memcpy(&datasize, traverse + traverseIndex, sizeof(datasize));
    traverseIndex += sizeof(datasize);

    memcpy(&expectedSeqNum, traverse + traverseIndex, sizeof(expectedSeqNum));
    traverseIndex += sizeof(expectedSeqNum);

    memcpy(&fileBuffer, traverse + traverseIndex, sizeof(datasize));

    return *datasize;
}