#include "helper.h"

// Main
int main(int argc, char* argv[]) {
    /*------ Variables -------*/

    // Client
    int sockfd; // socket
    int portno; // port number to listen on

    // Server
    char* hostIP = malloc(50);
    struct sockaddr_in serv_addr; // server's address
    socklen_t servlen; // byte size of server's address
    struct hostent *server; // server host info

    // Packet data
    char buffer[PACKET_SIZE]; // buffer
    int size = 0; // size of packet
    int SEQ = 0; // sequence number
    int ret = 0; // retransmission flag
    int SYN = 0; // SYN flag
    int FIN = 0; // FIN flag
    unsigned int start = 0;

    // ACKs
    int i;
    int ACKs[10];
    for (i = 0; i < 10; i++)
        ACKs[i] = -1;

    // While flags
    int handshakeSYN;
    int request;
    int request_break;
    int handshakeFIN;
    int handshakeFINACK;

    // File stuff
    char* filename; // filename argument
    FILE* fp;
    char* fw = "received.data";

    // Timeout intervals
    fd_set read_fds;
    struct timeval tv;
    int rv;

    struct timespec time_start, time_end;

    /*----- Validate args ------*/

    // Validate args
    if (argc != 4) {
        printf("Usage: %s <hostname> <port> <filename>\n", argv[0]);
        exit(0);
    }

    // Get arguments
    portno = atoi(argv[2]);
    filename = argv[3];

    if (portno < 0)
        error("ERROR: invalid port number");

    /*----- Setup server socket -----*/

    // Get host name
    hostIP = strcmp(argv[1], "localhost") == 0 ? "127.0.0.1" : argv[1];

    fprintf(stderr, "hostname: %s, portno: %d, filename: %s\n", hostIP, portno, filename);

    // Create parent socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == RC_ERROR)
        error("ERROR: Could not open socket\n");
    
    // Build server's internet address
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    if (inet_aton(hostIP, &serv_addr.sin_addr) == 0)
        error("ERROR: Could not inet_aton\n");
    servlen = sizeof(serv_addr);

    ret = 0;

    /*----- Transmit packets -----*/

    handshakeSYN = 1;
    // SYN/SYN-ACK Handshake
    while (handshakeSYN) {
        // Send SYN
        if (sendTo(sockfd, buffer, 0, (struct sockaddr *) &serv_addr, servlen, 0, 1, 0 ,0) == RC_ERROR)
            error("ERROR: Could not initiate handshake (SYN/SYN-ACK)\n");
        else {
            if (ret)
                printf("Sending packet Retransmission SYN\n");
            else
                printf("Sending packet SYN\n");
        }

        // Create a timeout interval
        FD_ZERO(&read_fds);
        FD_SET(sockfd, &read_fds);

        tv.tv_sec = 0;
        tv.tv_usec = TIME_OUT * 1000;

        // Wait for message to come in from socket
        if ((rv = select(sockfd+1, &read_fds, NULL, NULL, &tv)) == 0) {
            fprintf(stderr, "TIMEOUT: packet timed out.");
            ret = 1; // Retransmit packet if timeoute
        }
        // Receive SYN-ACK
        else if (recvFrom(sockfd, buffer, &size, (struct sockaddr *) &serv_addr, &servlen, &SEQ, &SYN, &FIN, &start) == RC_ERROR)
            error("ERROR: Could not receive SYN-ACK\n");

        if (SYN && !SEQ && !size) {
            printf("Receiving packet SYN-ACK\n");
            handshakeSYN = 0;
        }
        else {
            fprintf(stderr, "ERROR: Did not receive expected packet, retransmitting.");
            ret = 1;
        }
    }

    ret = 0;
    request = 1;
    while (request) {
        if (sendTo(sockfd, filename, strlen(filename), (struct sockaddr *) &serv_addr, servlen, 1, 0, 0, 0) == RC_ERROR)
            error("ERROR: Could not send request packet\n");

        else {
            if (ret)
                printf("Sending packet 0 Retransmission\n");
            else
                printf("Sending packet 0\n");
        }

        ret = 1;
        memset(buffer, 0, PACKET_SIZE);

        // Timeout interval
        FD_ZERO(&read_fds);
        FD_SET(sockfd, &read_fds);

        tv.tv_sec = 0;
        tv.tv_usec = TIME_OUT * 1000;

        // Wait for message from socket
        if ((rv = select(sockfd+1, &read_fds, NULL, NULL, &tv)) == 0) {
            fprintf(stderr, "TIMEOUT: packet timed out.");
            ret = 1;
            continue;
        }

        // Open file for writing
        if ((fp = fopen(fw, "wb")) == NULL)
            error("ERROR: Could not open write-to file\n");

        while (!FIN) {
            memset(buffer, 0, PACKET_SIZE);

            if (recvFrom(sockfd, buffer, &size, (struct sockaddr *) &serv_addr, &servlen, &SEQ, &SYN, &FIN, &start) == RC_ERROR)
                error("ERROR: Could not receive packets");

            request_break = 0;
            if (SYN && !FIN) {
                request_break = 1;
                break;
            
            }
            else if (!FIN)
                printf("Receiving packet %i\n", SEQ);
            else {
                printf("Receiving packet %i FIN\n", SEQ);
                request = 0;
                SEQ += HEADER_SIZE;
                SEQ %= MAX_SEQ_NO;
                break;
            }

            // Change next ACK
            int ACK = SEQ + size + HEADER_SIZE;
            ACK %= MAX_SEQ_NO;

            fseek(fp, start, SEEK_SET);
            fwrite(buffer, sizeof(char), size, fp);

            ret = add(ACKs, ACK);

            if((sendTo(sockfd, buffer, 0, (struct sockaddr *) &serv_addr, servlen, ACK, 0, 0, 0)) == RC_ERROR)
                error("ERROR: could not send to socket.");

            if (ret)
                printf("Sending packet %i Retransmission\n", ACK);
            else
                printf("Sending packet %i\n", ACK);
        }

        if (request_break) {
            request = 0;
            continue;
        }
    }

    ret = 0;
    handshakeFIN = 1;
    while (handshakeFIN) {
        // Send FIN
        if((sendTo(sockfd, buffer, 0, (struct sockaddr *) &serv_addr, servlen, SEQ, 0, 1, 0)) == RC_ERROR)
            error("ERROR: could not send to socket.");        
        if (ret)
            printf("Sending packet %i Retransmission FIN\n", SEQ);
        else
            printf("Sending packet %i FIN\n", SEQ);

        // Timeout interval
        FD_ZERO(&read_fds);
        FD_SET(sockfd, &read_fds);

        tv.tv_sec = 0;
        tv.tv_usec = TIME_OUT * 1000;
        
        // Wait for message from socket
        if ((rv = select(sockfd+1, &read_fds, NULL, NULL, &tv)) == 0) {
            fprintf(stderr, "TIMEOUT: packet timed out");
            ret = 1;
            continue;
        }

        FIN = 0;
        SYN = 1;
        int retSEQ = -1;

        handshakeFINACK = 1;
        while (handshakeFINACK) {
            if((recvFrom(sockfd, buffer, &size, (struct sockaddr *) &serv_addr, &servlen, &retSEQ, &SYN, &FIN, &start)) == RC_ERROR)
                error("ERROR: could not recvfrom socket");
            
            if (FIN && !SYN && retSEQ == SEQ)
                handshakeFIN = 0;
            else {
                printf("Receiving packet %i\n", retSEQ);
                ret = 1;
            }

            handshakeFINACK = 0;
        }

        if (handshakeFIN)
            continue;

        printf("Receiving packet %i FIN-ACK\n", retSEQ);
        int msec = 0;
        clock_gettime(CLOCK_MONOTONIC_RAW, &time_start);

        while (msec < TIME_OUT*4) {
            // ACK the FIN
            if((sendTo(sockfd, buffer, 0, (struct sockaddr *) &serv_addr, servlen, (SEQ+HEADER_SIZE)%MAX_SEQ_NO, 0, 0, 0)) == RC_ERROR)
                error("ERROR: could not send to socket");

            printf("Sending packet %i\n", SEQ);

            clock_gettime(CLOCK_MONOTONIC_RAW, &time_end);
            msec = (time_end.tv_sec-time_start.tv_sec)*1000 + (time_end.tv_nsec-time_start.tv_nsec)/1000000;
            int timeout = TIME_OUT*4-msec;

            if (timeout < 0)
                break;

            // Timeout interval
            FD_ZERO(&read_fds);
            FD_SET(sockfd, &read_fds);

            tv.tv_sec = 0;
            tv.tv_usec = TIME_OUT * 1000;
            
            // Wait for message from socket
            if ((rv = select(sockfd+1, &read_fds, NULL, NULL, &tv)) == 0) {
                handshakeFIN = 0;
                break;
            }

            if ((recvFrom(sockfd, buffer, &size, (struct sockaddr *) &serv_addr, &servlen, &SEQ, &SYN, &FIN, &start)) == RC_ERROR)
                error("ERROR: could not recvfrom socket");

            if (FIN && SYN && SEQ == retSEQ) {
                printf("Receiving packet %i FIN-ACK\n", retSEQ);
                handshakeFINACK = 0;
            }
            else if (FIN) {
                printf("Receiving packet %i FIN\n", SEQ);
                handshakeFIN = 0;
            }
            else
                printf("Receiving packet %i\n", SEQ);
        }
    }

    printf("Closing connection\n");

    // Close everything
    fclose(fp);
    close(sockfd);

    return RC_SUCCESS;
}
