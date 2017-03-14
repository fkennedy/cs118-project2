#define PACKET_SIZE 1024

struct packet {
    /* type of packet
        0 - DATA
        1 - ACK
        2 - SYN
        3 - FIN
        4 - REQUEST
    */
    int type;
    int SEQ; // sequence number flag
    int ACK; // ack number flag
    int size; // size of packet
    char data[PACKET_SIZE]; // data contained in packet
};