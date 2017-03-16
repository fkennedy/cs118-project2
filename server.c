#include <stdio.h>
#include <sys/types.h>  // definitions of a number of data types used in socket.h and netinet/in.h
#include <sys/socket.h> // definitions of structures needed for sockets, e.g. sockaddr
#include <netinet/in.h> // constants and structures needed for internet domain addresses, e.g. sockaddr_in
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/time.h>

// Return codes
#define RC_SUCCESS  0
#define RC_EXIT     1
#define RC_ERROR    -1

// Constants
#define MAX_SEQ_NO      30720
#define WINDOW_SIZE     5120
#define TIME_OUT        500
#define HEADER_SIZE     20
#define PACKET_SIZE     1024
#define TIMEOUT_SECS    2
#define TIMEOUT_USECS   10000

// Function headers
void error(char* msg);

int recvfromwithheaders(int sockfd, void *buf, size_t len, int flags, \
        struct sockaddr *src_addr, socklen_t *addrlen, char* fileBuffer, int* ack, int* fin, int* seqNum, int* ackChecksum, int* datasize);

// Main
int main(int argc, char* argv[]) {
    // Validate args
    if (argc != 2) {
        fprintf(stderr,"Usage: %s <port>\n", argv[0]);
        exit(RC_EXIT);
    }

    // Declare variables
    int sockfd; // socket
    int portno; // port number to listen on
    int opt; // setsockopt flag
    char buf[PACKET_SIZE];
    char* fileBuffer;

    // Server
    struct sockaddr_in serv_addr; // server's address
    int servlen; // byte size of server's address

    // Client
    struct sockaddr_in cli_addr; // client's address
    socklen_t clilen = sizeof(cli_addr); // byte size of client's address
    struct hostent *client; // client host info
    char *hostaddr; // client host address in dotted-decimal notation (IP address)

    // File vars
    char filename[PACKET_SIZE]; // file name to open
    FILE* fp;
    long filesize;

    // Get port number
    portno = atoi(argv[1]);

    // Create parent socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == RC_ERROR)
        error("ERROR: Could not open socket\n");

    // This is to prevent "Error: Address already in use"
    opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *) &opt, sizeof(int));

    // Get server address
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(portno);

    // Bind socket
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == RC_ERROR)
        error("ERROR: Could not bind\n");

    printf("Server listening on port %d!\n", portno);

    // Receive packet from the client
    if ((recvfrom(sockfd, filename, sizeof(filename), 0, (struct sockaddr *) &cli_addr, &clilen)) == RC_ERROR)
        error("ERROR: Could not receive packet\n");

    printf("\nFile Requested: %s\n", filename);

    // Open the file
    // using rb because we're not only opening text files
    // if ((fp = fopen(filename, "rb")) == NULL)
    //     // TODO: respond with seq = 0;

    // // Get filesize
    // fseek(fp, 0, SEEK_END);
    // filesize = ftell(fp);
    // fseek(fp, 0, SEEK_SET);

    // Vars
    int expectedAck = 0;
    int windowPos = 0;
    int numPacketsSent = 0;
    int numAcksReceived = 0;

    // Headers
    int fin = 0;
    int ack = 0;
    int ackChecksum = 0;
    int datasize = 0;
    int seqNum = 0;

    while (!fin) {
        numPacketsSent = 0;

        // while ()
        // TODO: don't forget to increment numPacketsSent after sending a packet

        // Wait for ACKs from sockets
        numAcksReceived = 0;

        while (numAcksReceived < numPacketsSent) {
            fd_set set;
            struct timeval timeoutInterval;
            int currAcksReceived = 0;
        
            FD_ZERO(&set);
            FD_SET(sockfd, &set);

            timeoutInterval.tv_sec = TIMEOUT_SECS;
            timeoutInterval.tv_usec = TIMEOUT_USECS;

            if ((currAcksReceived = select(sockfd + 1, &set, NULL, NULL, &timeoutInterval)) < 1)
                error("ERROR: time out waiting for ACK from client");

            // Get the ACK
            if (recvfromwithheaders(sockfd, buf, PACKET_SIZE, 0, (struct sockaddr *) &cli_addr, &clilen, \
                    fileBuffer, &ack, &fin, &seqNum, &ackChecksum, &datasize) < 0)
                error("ERROR: getting ACK");

            // Corrupted ack
            if (ack != ~ackChecksum)
                fprintf(stderr, "Received corrupted ACK%d or ACK checksum %d\n", ack, ackChecksum);

            // Finished
            else if (fin) {
                fprintf(stderr, "Received ACK%d\n", ack);
                break;
            }

            // ACK is within window, move window forward
            else if (ack > expectedAck && ack <= expectedAck + PACKET_SIZE) {
                fprintf(stderr, "Received ACK%d\n", ack);

                numAcksReceived++;
                expectedAck += PACKET_SIZE;
                windowPos += PACKET_SIZE;
            }
        }
    }

    fprintf(stdout, "Connection closed\n");
    fclose(fp);
    close(sockfd);

    return RC_SUCCESS;
}

// Helper functions
void error(char *msg) {
    perror(msg);
    exit(RC_EXIT);
}

int recvfromwithheaders(int sockfd, void *buf, size_t len, int flags, \
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