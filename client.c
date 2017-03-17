#include "helper.h"

// Main
int main(int argc, char* argv[]) {
    // Declare variables
    int sockfd; // socket
    int portno; // port number to listen on
    char* hostIP = malloc(50);

    // Server
    struct sockaddr_in serv_addr; // server's address
    socklen_t servlen; // byte size of server's address
    struct hostent *server; // server host info

    // Packet
    char buffer[PACKET_SIZE]; // buffer
    int size = 0; // size of packet
    int SEQ = 0; // sequence number
    int ret = 0; // retransmission flag
    int SYN = 0; // SYN flag
    int FIN = 0; // FIN flag
    unsigned int start = 0;

    // ACK
    int * ACKs = (int *) malloc(sizeof(int) * 10);
    for (int i = 0; i < 10; i++)
        ACKs[i] = -1;

    // While flags
    int handshakeSYN = 1;
    int request = 1;
    int request_break = 0;
    int handshakeFIN = 1;
    int handshakeFINACK = 1;

    // File stuff
    char* filename; // filename argument
    FILE* fp;
    char* fw = "data.log";

    // Time out stuff
    fd_set read_fds;
    struct timeval tv;
    int rv;

    // Validate args
    if (argc != 4) {
        printf("Usage: %s <hostname> <port> <filename>\n", argv[0]);
        exit(RC_SUCCESS);
    }

    struct timespec time_start, time_end;

    // Get host name
    if (strcmp(argv[1], "localhost") == 0)
        hostIP = "127.0.0.1";
    else
        strcpy(hostIP, argv[1]);

    // Get port number
    if (portno < 0)
        error("ERROR: Invalid port number\n");
    else
        portno = atoi(argv[2]);

    // Get file name
    filename = argv[3];

    printf("hostname: %s, portno: %d, filename: %s\n", hostIP, portno, filename);

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

        // Time-out
        FD_ZERO(&read_fds);
        FD_SET(sockfd, &read_fds);

        tv.tv_sec = 0;
        tv.tv_usec = TIME_OUT * 1000;
        rv = select(sockfd+1, &read_fds, NULL, NULL, &tv);
        if ((rv = select(sockfd+1, &read_fds, NULL, NULL, &tv)) == 0) {
            ret = 1;
            continue;
        }

        // Receive SYN-ACK
        if (recvFrom(sockfd, buffer, &size, (struct sockaddr *) &serv_addr, &servlen, &SEQ, &SYN, &FIN, &start) == RC_ERROR)
            error("ERROR: Could not send SYN-ACK\n");

        if (SYN) {
            printf("Receiving packet SYN-ACK\n");
            handshakeSYN = 0;
        }
        else
            ret = 1;
    }

    ret = 0;

    while (request) {
        if (sendTo(sockfd, filename, strlen(filename), (struct sockaddr *) &serv_addr, servlen, 1, 0, 0, 0) == RC_ERROR)
            error("ERROR: Could not send request packet\n");
        else {
            if (ret)
                printf("Sending packet 0 Retransmission %s of size %lu\n", filename, strlen(filename));
            else
                printf("Sending packet 0 %s\n", filename);
        }

        memset(buffer, 0, PACKET_SIZE);

        // Time-out
        FD_ZERO(&read_fds);
        FD_SET(sockfd, &read_fds);

        tv.tv_sec = 0;
        tv.tv_usec = TIME_OUT * 1000;
        rv = select(sockfd+1, &read_fds, NULL, NULL, &tv);
        if ((rv = select(sockfd+1, &read_fds, NULL, NULL, &tv)) == 0) {
            ret = 1;
            continue;
        }

        fp = fopen(fw, "wb"); // Using wb because we're not only opening text files
        if (fp == NULL)
            error("ERROR: Could not open write-to file\n");

        while (!FIN) {
            memset(buffer, 0, PACKET_SIZE);
            if (recvFrom(sockfd, buffer, &size, (struct sockaddr *) &serv_addr, &servlen, &SEQ, &SYN, &FIN, &start) == RC_ERROR)
                error("ERROR: Could not receive packets");

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

            int ACK = SEQ+size+HEADER_SIZE;
            ACK %= MAX_SEQ_NO;

            fseek(fp, start, SEEK_SET);
            fwrite(buffer, sizeof(char), size, fp);

            ret = add(ACKs, ACK);
            sendTo(sockfd, buffer, 0, (struct sockaddr *) &serv_addr, servlen, ACK, 0, 0, 0);
            if (ret)
                printf("Sending packet %i Retransmission\n", ACK);
            else
                printf("Sending packet %i\n", ACK);
        }

        if (request_break) {
            request_break = 0;
            continue;
        }
    }

    while (handshakeFIN) {
        // Send FIN
        sendTo(sockfd, buffer, 0, (struct sockaddr *) &serv_addr, servlen, SEQ, 0, 1, 0);
        if (ret)
            printf("Sending packet %i Retransmission FIN\n", SEQ);
        else
            printf("Sending packet %i FIN\n", SEQ);

        // Time-out
        FD_ZERO(&read_fds);
        FD_SET(sockfd, &read_fds);

        tv.tv_sec = 0;
        tv.tv_usec = TIME_OUT * 1000;
        rv = select(sockfd+1, &read_fds, NULL, NULL, &tv);
        if ((rv = select(sockfd+1, &read_fds, NULL, NULL, &tv)) == 0) {
            ret = 1;
            continue;
        }

        FIN = 0;
        SYN = 1;
        int retSEQ = -1;

        while (handshakeFINACK) {
            recvFrom(sockfd, buffer, &size, (struct sockaddr *) &serv_addr, &servlen, &retSEQ, &SYN, &FIN, &start);
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
            sendTo(sockfd, buffer, 0, (struct sockaddr *) &serv_addr, servlen, (SEQ+HEADER_SIZE)%MAX_SEQ_NO, 0, 0, 0);
            printf("Sending packet %i\n", SEQ);

            clock_gettime(CLOCK_MONOTONIC_RAW, &time_end);
            msec = (time_end.tv_sec-time_start.tv_sec)*1000 + (time_end.tv_nsec-time_start.tv_nsec)/1000000;
            int timeout = TIME_OUT*4-msec;

            if (timeout < 0)
                break;

            // Time-out
            FD_ZERO(&read_fds);
            FD_SET(sockfd, &read_fds);

            tv.tv_sec = 0;
            tv.tv_usec = TIME_OUT * 1000;
            rv = select(sockfd+1, &read_fds, NULL, NULL, &tv);
            if ((rv = select(sockfd+1, &read_fds, NULL, NULL, &tv)) == 0) {
                handshakeFIN = 0;
                break;
            }

            recvFrom(sockfd, buffer, &size, (struct sockaddr *) &serv_addr, &servlen, &SEQ, &SYN, &FIN, &start);
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