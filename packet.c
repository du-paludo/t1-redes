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

packet_t* makePacket(unsigned char *data, int size, int sequence, int type) {
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
    unsigned char startDelimiter = p->startDelimiter;
    unsigned int size = p->size;
    unsigned int sequence = p->sequence;
    unsigned int type = p->type;
    unsigned char* message = malloc(sizeof(unsigned char) * 63);

    for (int i = 0; i < 63; i++) {
        message[i] = p->data[i];
    }

    if (startDelimiter == '~') {
        if (type == 1)
        printf("%s", message);
    }

    return message;
}