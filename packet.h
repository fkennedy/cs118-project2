#define PACKET_SIZE 1024

struct packet {
    /* type of packet
        d - DATA
        a - ACK
        s - SYN
        f - FIN
        r - REQUEST
    */
    char type;
    int RET; // 
    int SEQ; // sequence number
    int ACK; // ack number
    int size; // size of packet
    char data[PACKET_SIZE]; // data contained in packet
};