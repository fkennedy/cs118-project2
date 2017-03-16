#include "helper.h"

// Helper functions
void error(char *msg) {
    perror(msg);
    exit(RC_EXIT);
}

int sendTo(int sockfd, char* buffer, size_t size, struct sockaddr *dest_addr, socklen_t destlen, int SEQ, int syn, int fin, unsigned int start) {
    int result = -1;
    int packetSize = HEADER_SIZE+size;

    if (packetSize > PACKET_SIZE)
        return result;

    char* packetSend = malloc(packetSize);
    memset(packetSend, 0, packetSize);

    memcpy(packetSend + sizeof(int)*0, &size, sizeof(int));
    memcpy(packetSend + sizeof(int)*1, &SEQ, sizeof(int));
    memcpy(packetSend + sizeof(int)*2, &syn, sizeof(int));
    memcpy(packetSend + sizeof(int)*3, &fin, sizeof(int));
    memcpy(packetSend + sizeof(int)*4, &start, sizeof(int));
    memcpy(packetSend + HEADER_SIZE, buffer, size);

    if ((result = sendto(sockfd, packetSend, packetSize, 0, dest_addr, destlen)) == RC_ERROR) 
        perror("ERROR: Could not sendto\n");

    free(packetSend);

    return result;
}

int recvFrom(int sockfd, char* buffer, size_t* size, struct sockaddr *src_addr, socklen_t *srclen, int* SEQ, int* syn, int* fin, unsigned int* start) {
    int result = -1;
    int packetSize = PACKET_SIZE;

    char* packetReceived = malloc(packetSize);
    memset(packetReceived, 0, packetSize);
    memset(buffer, 0, packetSize);

    if ((result = recvfrom(sockfd, packetReceived, packetSize, 0, src_addr, srclen)) == RC_ERROR)
        perror("ERROR: Could not recvfrom\n");

    memcpy(size, packetReceived + sizeof(int)*0, sizeof(int));
    memcpy(SEQ, packetReceived + sizeof(int)*1, sizeof(int));
    memcpy(syn, packetReceived + sizeof(int)*2, sizeof(int));
    memcpy(fin, packetReceived + sizeof(int)*3, sizeof(int));
    memcpy(start, packetReceived + sizeof(int)*4, sizeof(int));
    memcpy(buffer, packetReceived + HEADER_SIZE, sizeof(int));

    free(packetReceived);

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