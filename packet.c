#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "packet.h"
#include "rawSocketConnection.h"

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
    // unsigned int sequence = p->sequence;
    unsigned int type = p->type;
    unsigned char* message = malloc(sizeof(unsigned char) * 63);

    for (int i = 0; i < size; i++) {
        message[i] = p->data[i];
    }

    if (startDelimiter == '~') {
        printf("%s", message);
    }

    return message;
}

void sendAck(int socket) {
    packet_t* ack = makePacket(NULL, 0, 0, 14);
    send(socket, ack, MESSAGE_SIZE, 0);
}