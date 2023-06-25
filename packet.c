#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "packet.h"
#include "time.h"
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

void makePacket(packet_t* p, unsigned char *data, int size, int sequence, int type) {
    p->startDelimiter = START_DELIMITER;
    p->size = size;
    p->sequence = sequence;
    p->type = type;
    // for (int i = 0; i < size; i++) {
    //     p->data[i] = data[i];
    // }
    memcpy(p->data, data, size);
    p->vrc = calculateVRC(p);

    if (type != 14 && type != 15) {
        printf("VRC: %d\n", p->vrc);
        printf("Sequence: %d\n", p->sequence);
        printf("Type: %d\n", p->type);
        printf("Size: %d\n", p->size);
    }
}

unsigned char* packetToBuffer(packet_t *p) {
    unsigned int size = p->size;
    unsigned char* message = malloc(sizeof(unsigned char) * size);

    for (int i = 0; i < size; i++) {
        message[i] = p->data[i];
    }
    // if (startDelimiter == '~') {
    //     printf("%s", message);
    // }

    return message;
}

void sendAck(int socket, packet_t* packet) {
    printf("Sending ACK\n");
    makePacket(packet, NULL, 0, packet->sequence, 14);
    send(socket, packet, MESSAGE_SIZE, 0);
}

void sendNack(int socket, packet_t* packet) {
    printf("Sending NACK\n");
    makePacket(packet, NULL, 0, packet->sequence, 15);
    send(socket, packet, MESSAGE_SIZE, 0);
}

void waitResponse(int socket, packet_t* packet, packet_t* response, int sequence) {
    while (1) {
        recv(socket, response, MESSAGE_SIZE, 0);
        if (response->startDelimiter == '~') {
            if (response->type == 14 && response->sequence == sequence) {
                printf("ACK received\n");
                break;
            } else if (response->type == 15) {
                printf("NACK received\n");
                send(socket, packet, MESSAGE_SIZE, sequence);
                continue;
            }
        }
    }
}

// wait response com timeout
int waitResponseTimeout(int socket, packet_t* packet, packet_t* response, int sequence) {
    time_t start;
    while (1) {
        start = time(NULL);
        while (time(NULL) - start < 1) {
            recv(socket, response, MESSAGE_SIZE, 0);
            if (response->startDelimiter == '~') {
                if (response->type == 7) {
                    printf("MD5 received\n");
                    return 1;
                } else if (response->type == 14 && response->sequence == sequence) {
                    printf("ACK received\n");
                    return 1;
                } else if (response->type == 15) {
                    printf("NACK received\n");
                    break;
                }
            }
        }
        if (time(NULL) - start >= 1) {
            printf("Timeout\n");
        }
        send(socket, packet, MESSAGE_SIZE, 0);
        printPacket(packet);
        printf("%d\n", sequence);
    }
    return 0;
}

unsigned char calculateVRC(packet_t* packet) {
    unsigned char vrc = 0;
    for (int i = 0; i < packet->size; i++) {
        vrc ^= packet->data[i];
    }
    return vrc;
}

void printPacket(packet_t* p) {
    printf("VRC: %d\n", p->vrc);
    printf("Sequence: %d\n", p->sequence);
    printf("Type: %d\n", p->type);
    printf("Size: %d\n", p->size);
}

void sendMessage(int socket, packet_t* packet, packet_t* response) {
    send(socket, packet, MESSAGE_SIZE, 0);
    waitResponseTimeout(socket, packet, response, packet->sequence);
}