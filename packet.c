#include "packet.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// packet_t* makePacket(unsigned char *data, int size, int sequence, int type) {
//     packet_t* p = malloc(sizeof(packet_t));
//     p->startDelimiter = START_DELIMITER;
//     p->size = size;
//     p->sequence = sequence;
//     p->type = type;
//     memcpy(p->data, data, size);
//     p->crc = 0x00;
    
//     return p;
// }

packet_t* makePacket(char *data, int size, int sequence, int type) {
    packet_t* p = malloc(sizeof(packet_t));

    p->startDelimiter = START_DELIMITER;
    p->size = size;
    p->sequence = sequence;
    p->type = type;
    memcpy(p->data, data, size);
    p->crc = 0x00;

    return p;
}

unsigned char* packetToBuffer(packet_t *p){
    unsigned char *message = malloc(sizeof(1024));

    message[0] = START_DELIMITER;

    unsigned char temp;
    temp = (p->size << 2) | (p->sequence >> 4);
    message[1] = temp;

    temp = ((p->sequence & 0x0F) << 4) | (p->type);
    message[2] = temp;
    
    memcpy(message + 3, p->data, MESSAGE_SIZE);

    message[3 + MESSAGE_SIZE] = p->crc;

    return message;
}