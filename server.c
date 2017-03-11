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
    struct sockaddr_in cli_addr; // client's address
    int clilen; // byte size of client's address
    char buffer[BUFFER_SIZE]; // message buffer
    int opt; // setsockopt flag
    struct hostent *client; // client host info
    char *hostaddr; // client host address in dotted-decimal notation (IP address)

    // Validate args
    if (argc != 2) {
         fprintf(stderr,"Usage: %s <port>\n", argv[0]);
         exit(RC_SUCCESS);
    }

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
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    // Bind socket
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == RC_ERROR)
        error("ERROR: Could not bind\n");

    clilen = sizeof(cli_addr);
    while (1) {
        // Receive message from the client
        memset(buffer, 0, BUFFER_SIZE);
        if (recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *) &cli_addr, (socklen_t *) &clilen) == RC_ERROR)
            error("ERROR: Could not receive message\n");

        // Get the client information
        client = gethostbyaddr((const void *) &cli_addr.sin_addr.s_addr, sizeof(cli_addr.sin_addr.s_addr), AF_INET);
        if (client == NULL)
            error("ERROR: Could not get client host information\n");
        hostaddr = inet_ntoa(cli_addr.sin_addr);
        if (hostaddr == NULL)
            error("ERROR: Could not change to IP address");
        fprintf(stdout, "Received message from %s (%s)\n", client->h_name, hostaddr);
        fprintf(stdout, "Received %lu bytes\n", strlen(buffer));
        fprintf(stdout, "Message: %s\n", buffer);

        // Echo the message back to the client
        if (sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *) &cli_addr, clilen) == RC_ERROR)
            error("ERROR: Could not send message back\n");
    }

    return RC_SUCCESS;
}

// Helper functions
void error(char *msg) {
    perror(msg);
    exit(RC_SUCCESS);
}