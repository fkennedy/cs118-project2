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

ssize_t sendtowithheaders(int sockfd, const void *buf, size_t len, int flags,
               const struct sockaddr *dest_addr, socklen_t addrlen);

int recvfromwithheaders(int sockfd, void *buf, size_t len, int flags, 
        struct sockaddr *src_addr, socklen_t *addrlen, char* fileBuffer, int* ack, int* fin, int* seqNum, int* ackChecksum, int* datasize);

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
                (struct sockaddr *) &serverinfo, &servlen, fileBuffer, &ack, &fin, &ackChecksum, &datasize, &seqNum)) <= 0)
            error("ERROR: data is empty.");

        printf("\nFile Received: %s\n", fileBuffer);
        
        // File does not exist
        // TODO

        // Finished
        if (fin) {
            if ((sendtowithheaders(sockfd, filename, strlen(filename), 0, (const struct sockaddr *) &serverinfo, servlen,
                    &ack, &fin, &expectedSeqNum, &ackChecksum)) == RC_ERROR)
                error("ERROR: sending response to server.");
        }

        // Received a packet out of order
        else if (seqNum < expectedSeqNum - PACKET_SIZE || seqNum > expectedSeqNum) {
            fprintf(stderr, "Received a packet out of order, ignoring.\n");
            fin = 0;
        }

        // Packet received normally
        else {
            if ((sendtowithheaders(sockfd, filename, strlen(filename), 0, (const struct sockaddr *) &serverinfo, servlen,
                    &ack, &fin, &expectedSeqNum, &ackChecksum)) == RC_ERROR)
                error("ERROR: sending ACK to server.");

            expectedSeqNum += PACKET_SIZE;
        }

        // Write fileBuffer to file
        if (fwrite(fileBuffer, sizeof(char), datasize, fp) < 0)
            error("ERROR: could not write to file.");
    }

    close(sockfd);

    return RC_SUCCESS;
}

// Helper functions
void error(char *msg) {
    perror(msg);
    exit(RC_ERROR);
}

ssize_t sendtowithheaders(int sockfd, const void *buf, size_t len, int flags,
               const struct sockaddr *dest_addr, socklen_t addrlen, int* ack, int* fin, int* seqNum, int* ackChecksum, int* datasize) {

    // Packet stucture: 
    // | ack | fin | seqNum | ackChecksum | datasize | fileBuffer |

    char packetBuffer[PACKET_SIZE];
    char* traverse = packetBuffer;
    int traverseIndex = 0;

    datasize = strlen(buf);
    ackChecksum = ~ack;

    memcpy(traverse + traverseIndex, &ack, sizeof(ack));
    traverseIndex += sizeof(ack);   

    memcpy(traverse + traverseIndex, &fin, sizeof(fin));
    traverseIndex += sizeof(fin);   

    memcpy(traverse + traverseIndex, &seqNum, sizeof(seqNum));
    traverseIndex += sizeof(seqNum);   

    memcpy(traverse + traverseIndex, &ackChecksum, sizeof(ackChecksum));
    traverseIndex += sizeof(ackChecksum);   

    memcpy(traverse + traverseIndex, &datasize, sizeof(datasize));
    traverseIndex += sizeof(datasize); 

    memcpy(traverse + traverseIndex, &buf, sizeof(buf));
    traverseIndex += sizeof(buf);   

    return *datasize;  
}

int recvfromwithheaders(int sockfd, void *buf, size_t len, int flags, 
        struct sockaddr *src_addr, socklen_t *addrlen, char* fileBuffer, int* ack, int* fin, int* seqNum, int* ackChecksum, int* datasize) {
    
    int numBytesReceived;
    int traverseIndex = 0;

    printf("HERE");

    if ((numBytesReceived = recvfrom(sockfd, buf, len, flags, src_addr, addrlen)) == RC_ERROR)
        error("ERROR: did not receive packet correctly.");

    printf("HERE2");

    // Packet stucture: 
    // | ack | fin | seqNum | ackChecksum | datasize | fileBuffer |

    char* traverse = buf;

    memcpy(&ack, traverse + traverseIndex, sizeof(ack));
    traverseIndex += sizeof(ack);

    memcpy(&fin, traverse + traverseIndex, sizeof(fin));
    traverseIndex += sizeof(fin);

    memcpy(&seqNum, traverse + traverseIndex, sizeof(seqNum));
    traverseIndex += sizeof(seqNum);

    memcpy(&ackChecksum, traverse + traverseIndex, sizeof(ackChecksum));
    traverseIndex += sizeof(ackChecksum);

    memcpy(&datasize, traverse + traverseIndex, sizeof(datasize));
    traverseIndex += sizeof(datasize);

    memcpy(&seqNum, traverse + traverseIndex, sizeof(seqNum));
    traverseIndex += sizeof(seqNum);

    memcpy(&fileBuffer, traverse + traverseIndex, sizeof(datasize));

    return *datasize;
}