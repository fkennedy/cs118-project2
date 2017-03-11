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
#define RC_SUCCESS  1
#define RC_ERROR    -1

// Constants
#define BUFFER_SIZE 1024
#define PACKET_SIZE 1024
#define MAX_SEQ_NO  30720
#define WINDOW_SIZE 5120
#define TIME_OUT    500

// Packet structure
struct packet {
    int seq;
    int ack;
    int size;
    char data[PACKET_SIZE];
};

// Function headers
void error(char * msg);

// Main
int main(int argc, char* argv[]) {
    // Declare variables
    int sockfd; // socket
    int portno; // port number to listen on
    struct sockaddr_in serv_addr; // server's address
    int servlen; // byte size of server's address
    char* hostname; // hostname
    char* filename; // filename
    struct hostent *server; // server host info
    char buffer[BUFFER_SIZE]; // message buffer

    // Validate args
    if (argc != 3) {
         fprintf(stderr, "Usage: %s <hostname> <port>\n", argv[0]);
         exit(RC_SUCCESS);
    }

    // Validate args
    // if (argc != 4) {
    //      fprintf(stderr, "Usage: %s <hostname> <port> <filename>\n", argv[0]);
    //      exit(RC_SUCCESS);
    // }

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
    // Send a message to the server
    memset(buffer, 0, BUFFER_SIZE);
    fprintf(stdout, "Enter message: ");
    fgets(buffer, BUFFER_SIZE, stdin);
    if (sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *) &serv_addr, servlen) == RC_ERROR)
        error("ERROR: Could not send message\n");

    // Receive message from the server
    memset(buffer, 0, BUFFER_SIZE);
    if (recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *) &serv_addr, (socklen_t *) &servlen) == RC_ERROR)
        error("ERROR: Could not receive message\n");
    fprintf(stdout, "Message: %s\n", buffer);

    return RC_SUCCESS;
}

// Helper functions
void error(char *msg) {
    perror(msg);
    exit(RC_SUCCESS);
}