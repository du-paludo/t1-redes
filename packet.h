#ifndef PACKET_H
#define PACKET_H

#define START_DELIMITER '~'
#define MESSAGE_SIZE 16

struct packet {
    unsigned char startDelimiter:8;  // 011111110
    unsigned char size:6;
    unsigned char sequence:6;
    unsigned char type:4;
    unsigned char data[MESSAGE_SIZE];
    unsigned char crc:8;
};
typedef struct packet packet_t;

unsigned char* makePacket(char *data, int size, int sequence, int type);

#endif