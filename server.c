#include "helper.h"

// Main
int main(int argc, char* argv[]) {
    // Declare variables
    int sockfd; // socket
    int portno; // port number to listen on
    int opt; // setsockopt flag

    // Server
    struct sockaddr_in serv_addr; // server's address
    socklen_t servlen; // byte size of server's address

    // Client
    struct sockaddr_in cli_addr; // client's address
    socklen_t clilen; // byte size of client's address
    struct hostent *client; // client host info
    char *hostaddr; // client host address in dotted-decimal notation (IP address)
    
    // Packet
    char buffer[PACKET_SIZE]; // buffer
    int size = 0; // size of packet
    int SEQ = 0; // sequence number
    int ret = 0; // retransmission flag
    int SYN = 0; // SYN flag
    int FIN = 0; // FIN flag
    unsigned int start = 0;

    // Packet data
    struct packet packet[5];

    // Packet access
    int cur = 0; // current packet number
    int ACKsReceived = 0;
    int base = 0;
    int basefile = 0;
    unsigned int offset;
    int payload;
    int payloadLen;
    unsigned int temp;

    // Time
    int msec;
    int oldestIndex;
    int oldestTime;
    struct timespec current;

    // Loop/Jump flags stuff
    int handshakeSYNACK;
    int handshakeFIN = 1;
    int FINsent;
    int handshakeFINACK;
    int timeout;
    int i;

    // File stuff
    char filename[PACKET_SIZE]; // file name to open
    FILE* fp;
    long filesize;
    char* data;

    // Time out stuff
    fd_set read_fds;
    struct timeval tv;
    int rv;

    // Validate args
    if (argc != 2) {
        fprintf(stderr,"Usage: %s <port>\n", argv[0]);
        exit(0);
    }

    // Get port number
    if (portno < 0)
        error("ERROR: Invalid port number\n");
    else
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

    printf("Server listening on port %d...\n", portno);

    while (1) {
        // SYN/SYN-ACK Handshake
        // Receive packet from the client
        while (!SYN) {
            // Receive SYN
            if (recvFrom(sockfd, buffer, &size, (struct sockaddr *) &cli_addr, (socklen_t *) &clilen, &SEQ, &SYN, &FIN, &start) == RC_ERROR)
                error("ERROR: Could not receive SYN packet\n");
        }
        
        printf("Client: %s:%d\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
        printf("Receiving packet %i %i SYN\n", SEQ, WINDOW_SIZE);

        ret = 0;
        
        handshakeSYNACK = 1;
        // Send SYN-ACK
        while (handshakeSYNACK) {
            if (sendTo(sockfd, buffer, 0, (struct sockaddr *) &cli_addr, clilen, SEQ, 1, 0, 0) == RC_ERROR)
                error("ERROR: Could not send SYN-ACK\n");
            else {
                if (ret)
                    printf("Sending packet %i %i Retransmission SYN-ACK\n", SEQ, WINDOW_SIZE);
                else
                    printf("Sending packet %i %i SYN-ACK\n", SEQ, WINDOW_SIZE);
            }

            // Time-out
            FD_ZERO(&read_fds);
            FD_SET(sockfd, &read_fds);

            tv.tv_sec = 0;
            tv.tv_usec = TIME_OUT*1000;
            if ((rv = select(sockfd+1, &read_fds, NULL, NULL, &tv)) == 0) {
                ret = 1;
                continue;
            }

            if (recvFrom(sockfd, filename, &size, (struct sockaddr *) &cli_addr, (socklen_t *) &clilen, &SEQ, &SYN, &FIN, &start) == RC_ERROR)
                error("ERROR: Could not receive request packet\n");

            if (SYN || !size)
                continue;
            else
                handshakeSYNACK = 0;
        }

        // Open the file
        fp = fopen(filename, "rb"); // Using rb because we're not only opening text files
        if (fp  == NULL) {
            sendTo(sockfd, buffer, 0, (struct sockaddr *) &cli_addr, clilen, SEQ, 0, 1, 0);
            error("ERROR: Could not open file\n");
        }

        // Get filesize
        fseek(fp, 0, SEEK_END);
        filesize = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        // Empty file
        if (filesize == 0) {
            sendTo(sockfd, buffer, 0, (struct sockaddr *) &cli_addr, clilen, SEQ, 0, 1, 0);
            error("ERROR: File is empty\n");
        }

        // Store file contents into data
        data = malloc(filesize+1);
        fread(data, 1, filesize, fp);
        data[filesize] = '\0';

        for (i = 0; i < 5; i++) {
            packet[i].location = 0;
            packet[i].location_next = 0;
            packet[i].SEQ = -1;
            packet[i].ACK = -1;
            packet[i].ACKed = -1;
            packet[i].length = -1;
        }

        ret = 0;

        while (!FIN) {
            offset = basefile + cur * PAYLOAD_SIZE;
            // Send packets
            while (offset < filesize && cur < 5) {
                
                // Calculate sequence number
                SEQ = (base+cur*PACKET_SIZE) % MAX_SEQ_NO;

                payload = PAYLOAD_SIZE;
                if (filesize-offset < PAYLOAD_SIZE)
                    payload = filesize-offset;

                // Create packetSend buffer;
                char packetSend[payload];
                memset(packetSend, 0, payload);
                memcpy(packetSend, data+offset, payload);
                sendTo(sockfd, packetSend, payload, (struct sockaddr *) &cli_addr, clilen, SEQ, 0, 0, offset);
                printf("Sending packet %i %i\n", SEQ, WINDOW_SIZE);

                // Assign values to array
                clock_gettime(CLOCK_MONOTONIC_RAW, &packet[cur].timer);
                packet[cur].location = offset;
                packet[cur].location_next = offset+payload;
                packet[cur].SEQ = SEQ;
                packet[cur].ACK = (SEQ+payload+HEADER_SIZE) % MAX_SEQ_NO;
                packet[cur].ACKed = -1;
                packet[cur].length = payload;

                cur++;
                offset = basefile + cur * PAYLOAD_SIZE;
            }

            msec = 0;
            clock_gettime(CLOCK_MONOTONIC_RAW, &current);
            oldestIndex = -1;
            oldestTime = -1;

            for (i = 0; i < 5; i++) {
                if (packet[i].SEQ != -1 && packet[i].ACKed == -1) {
                    msec = (current.tv_sec-packet[i].timer.tv_sec)*1000 + (current.tv_nsec-packet[i].timer.tv_nsec)/1000000;

                    if (msec > oldestTime) {
                        oldestTime = msec;
                        oldestIndex = i;
                    }
                }
            }

            timeout = 0;
            if (oldestTime > TIME_OUT)
                timeout = 1;

            // Time-out
            FD_ZERO(&read_fds);
            FD_SET(sockfd, &read_fds);

            tv.tv_sec = 0;
            tv.tv_usec = TIME_OUT*1000;
            if ((rv = select(sockfd+1, &read_fds, NULL, NULL, &tv)) == 0)
                timeout = 1;

            if (timeout) {
                // Retransmit timed out packages
                msec = 0;
                clock_gettime(CLOCK_MONOTONIC_RAW, &current);

                for (i = 0; i < 5; i++) {
                    if (packet[i].SEQ != -1 && packet[i].ACKed == -1) {
                        msec = (current.tv_sec-packet[i].timer.tv_sec)*1000 + (current.tv_nsec-packet[i].timer.tv_nsec)/1000000;

                        if (msec >= TIME_OUT) {
                            SEQ = packet[i].SEQ;
                            temp = packet[i].location;
                            payload = packet[i].length;

                            char packetSend[payload];
                            memset(packetSend, 0, payload);
                            memcpy(packetSend, data+temp, payload);

                            sendTo(sockfd, packetSend, payload, (struct sockaddr *) &cli_addr, clilen, SEQ, 0, 0, temp);
                            printf("Sending packet %i %i Retransmission\n", SEQ, WINDOW_SIZE);

                            // Start the timer again
                            clock_gettime(CLOCK_MONOTONIC_RAW, &packet[i].timer);
                        }
                    }
                }
            }
            else {
                int retsize = 0;
                int retSEQ = 0;
                int retSYN = 0;
                int retFIN = 0;

                recvFrom(sockfd, buffer, &retsize, (struct sockaddr *) &cli_addr, &clilen, &retSEQ, &retSYN, &retFIN, &start);
                
                FIN = retFIN;
                SEQ = retSEQ;
                
                if (FIN) {
                    ret = 0;
                    printf("Receiving packet %i FIN\n", SEQ);
                    handshakeFIN = 0;
                    break;
                }
                else
                    printf("Receiving packet %i\n", retSEQ);

                for (i = 0; i < 5; i++) {
                    if (packet[i].ACK == SEQ) {
                        packet[i].ACKed = SEQ;
                        break;
                    }
                }

                while (packet[0].ACKed != -1) {
                    basefile = packet[0].location_next;
                    ACKsReceived++;
                    payloadLen = packet[0].length;
                    base = packet[0].ACK;
                    cur--;

                    for (i = 0; i < 5; i++) {
                        if (i == 4) {
                            packet[i].location = 0;
                            packet[i].location_next = 0;
                            packet[i].SEQ = -1;
                            packet[i].ACK = -1;
                            packet[i].ACKed = -1;
                            packet[i].length = -1;
                        }
                        else {
                            packet[i].location = packet[i+1].location;
                            packet[i].location_next = packet[i+1].location_next;
                            packet[i].SEQ = packet[i+1].SEQ;
                            packet[i].ACK = packet[i+1].ACK;
                            packet[i].ACKed = packet[i+1].ACKed;
                            packet[i].length = packet[i+1].length;
                            packet[i].timer = packet[i+1].timer;
                        }   
                    }
                }

                FINsent = 1;
                if (basefile >= filesize) {
                    FIN = 1;
                    sendTo(sockfd, buffer, 0, (struct sockaddr *) &cli_addr, clilen, base, 1, FIN, 0);
                    printf("Sending packet %i %i FIN\n", base, WINDOW_SIZE);
                    FINsent = 0;
                    ret = 0;
                }
            }
        }

        // Send FIN
        while (handshakeFIN) {
            if (FINsent) {
                sendTo(sockfd, buffer, 0, (struct sockaddr *) &cli_addr, clilen, base, 1, 1, 0);
                if (ret)
                    printf("Sending packet %i %i Retransmission FIN\n", base, WINDOW_SIZE);
                else
                    printf("Sending packet %i %i FIN\n", base, WINDOW_SIZE);
            }
                
            // Time-out
            FD_ZERO(&read_fds);
            FD_SET(sockfd, &read_fds);

            tv.tv_sec = 0;
            tv.tv_usec = TIME_OUT*1000;
            if ((rv = select(sockfd+1, &read_fds, NULL, NULL, &tv)) == 0) {
                ret = 1;
                continue;
            }

            if (recvFrom(sockfd, filename, &size, (struct sockaddr *) &cli_addr, (socklen_t *) &clilen, &SEQ, &SYN, &FIN, &start) == RC_ERROR)
                error("ERROR: Could not receive FIN packet\n");

            if (FIN && SEQ == (base+HEADER_SIZE)%MAX_SEQ_NO) {
                handshakeFIN = 0;
                ret = 0;
                printf("Receiving packet %i FIN\n", SEQ);
            }
            else
                ret = 1;
        }

        ret = 0;

        handshakeFINACK = 1;
        // Send FIN-ACK
        while (handshakeFINACK) {
            int SEQnew = (base+HEADER_SIZE)%MAX_SEQ_NO;
            sendTo(sockfd, buffer, 0, (struct sockaddr *) &cli_addr, clilen, SEQnew, 0, 1, 0);
            
            if (ret)
                printf("Sending packet %i %i Retransmission FIN-ACK\n", SEQnew, WINDOW_SIZE);
            else
                printf("Sending packet %i %i FIN-ACK\n", SEQnew, WINDOW_SIZE);

            // Time-out
            FD_ZERO(&read_fds);
            FD_SET(sockfd, &read_fds);

            tv.tv_sec = 0;
            tv.tv_usec = TIME_OUT*1000;
            if ((rv = select(sockfd+1, &read_fds, NULL, NULL, &tv)) == 0) {
                ret = 1;
                continue;
            }

            // Last ACK
            recvFrom(sockfd, buffer, &size, (struct sockaddr *) &cli_addr, &clilen, &SEQ, &SYN, &FIN, &start);
            if (SEQ == (SEQnew+HEADER_SIZE)%MAX_SEQ_NO)
                handshakeFINACK = 0;
        }

        printf("Receiving packet %i\n", SEQ);
        printf("\n\nAwaiting new connection...\n\n");
    }

    printf("\n\nClosing connection\n\n");
    // Free variables
    free(data);

    // Close everything
    fclose(fp);
    close(sockfd);

    return RC_SUCCESS;
}