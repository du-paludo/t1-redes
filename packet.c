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

unsigned char* makePacket(char *data, int size, int sequence, int type) {
    packet_t* p = malloc(sizeof(packet_t));
    unsigned char *packet = malloc(sizeof(1024));

    p->startDelimiter = START_DELIMITER;
    p->size = size;
    p->sequence = sequence;
    p->type = type;
    memcpy(p->data, data, size);
    p->crc = 0x00;

    packet[0] = p->startDelimiter;

    unsigned char temp;
    temp = (p->size << 2) | (p->sequence >> 4);
    packet[1] = temp;

    temp = ((p->sequence & 0x0F) << 4) | (p->type);
    packet[2] = temp;
    
    memcpy(packet + 3, p->data, MESSAGE_SIZE);

    packet[3 + MESSAGE_SIZE] = p->crc;

    return packet;
}