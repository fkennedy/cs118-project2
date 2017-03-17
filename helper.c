#include "helper.h"

// Helper functions
void error(char* msg) {
    perror(msg);
    exit(0);
}

int sendTo(int sockfd, char* buffer, size_t size, struct sockaddr *dest_addr, socklen_t destlen, int SEQ, int SYN, int FIN, unsigned int start) {
    int result = -1;
    int packetSize = HEADER_SIZE+size;

    if (packetSize > PACKET_SIZE)
        return result;

    char* packetSend = malloc(packetSize);
    bzero(packetSend, packetSize);

    memcpy(packetSend + sizeof(int)*0, &size, sizeof(int));
    memcpy(packetSend + sizeof(int)*1, &SEQ, sizeof(int));
    memcpy(packetSend + sizeof(int)*2, &SYN, sizeof(int));
    memcpy(packetSend + sizeof(int)*3, &FIN, sizeof(int));
    memcpy(packetSend + sizeof(int)*4, &start, sizeof(int));
    memcpy(packetSend + HEADER_SIZE, buffer, size);

    if ((result = sendto(sockfd, packetSend, packetSize, 0, dest_addr, destlen)) == RC_ERROR) {
        error("ERROR: Could not sendto\n");
        free(packetSend);
        return -1;
    }

    free(packetSend);

    return result;
}

int recvFrom(int sockfd, char* buffer, int* size, struct sockaddr *src_addr, socklen_t *srclen, int* SEQ, int* SYN, int* FIN, unsigned int* start) {
    int result = -1;
    int len = PACKET_SIZE;

    char packetReceived[len];
    bzero(packetReceived, len);
    bzero(buffer, len);

    if ((result = recvfrom(sockfd, packetReceived, len, 0, src_addr, srclen)) == RC_ERROR)
        perror("ERROR: Could not recvfrom\n");

    memcpy(size, packetReceived + sizeof(int)*0, sizeof(int));
    memcpy(SEQ, packetReceived + sizeof(int)*1, sizeof(int));
    memcpy(SYN, packetReceived + sizeof(int)*2, sizeof(int));
    memcpy(FIN, packetReceived + sizeof(int)*3, sizeof(int));
    memcpy(start, packetReceived + sizeof(int)*4, sizeof(int));
    memcpy(buffer, packetReceived + sizeof(int)*5, PAYLOAD_SIZE);

    return result;
}

int add(int* list, int ACK) {
    int i;
    for (i = 0; i < 10; i++) {
        if (list[i] == -1) {
            list[i] = ACK;
            return 0;
        }
        else if (ACK == list[i])
            return 1;
    }

    for (i = 0; i < 9; i++)
        list[i] = list[i+1];
    list[9] = ACK;

    return 0;
}