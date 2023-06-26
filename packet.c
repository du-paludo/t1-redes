#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "packet.h"
#include "time.h"
#include "rawSocketConnection.h"

void makePacket(packet_t* packet, unsigned char *data, int size, int sequence, int type) {
    packet->startDelimiter = START_DELIMITER;
    packet->size = size;
    packet->sequence = sequence;
    packet->type = type;
    // for (int i = 0; i < size; i++) {
    //     p->data[i] = data[i];
    // }
    memcpy(packet->data, data, size);
    packet->vrc = calculateVRC(packet);
}

void packetToBuffer(packet_t *packet, unsigned char *data) {
    for (int i = 0; i < packet->size; i++) {
        data[i] = packet->data[i];
    }
}

void sendAck(int socket, packet_t* sentMessage, packet_t* receivedMessage) {
    makePacket(sentMessage, NULL, 0, receivedMessage->sequence, 14);
    printf("\nSent message:\n");
    printPacket(sentMessage);
    send(socket, sentMessage, MESSAGE_SIZE, 0);
}

void sendNack(int socket, packet_t* sentMessage, packet_t* receivedMessage) {
    makePacket(sentMessage, NULL, 0, receivedMessage->sequence, 15);
    printf("\nSent message:\n");
    printPacket(sentMessage);
    send(socket, sentMessage, MESSAGE_SIZE, 0);
}

int waitResponseTimeout(int socket, packet_t* sentMessage, packet_t* receivedMessage) {
    time_t start;
    while (1) {
        start = time(NULL);
        while (time(NULL) - start < 1) {
            recv(socket, receivedMessage, MESSAGE_SIZE, 0);
            #ifdef LOOPBACK
            if (receivedMessage->startDelimiter == '~' && receivedMessage->origin != sentMessage->origin)
            #else
            if (receivedMessage->startDelimiter == '~')
            #endif
            {
                if (receivedMessage->sequence == sentMessage->sequence) {
                    printf("\nReceived message:\n");
                    printPacket(receivedMessage);
                    switch (receivedMessage->type) {
                        case 7:
                            printf("MD5 received\n");
                            return 1;
                        case 14:
                            printf("ACK received\n");
                            return 1;
                        case 15:
                            printf("NACK received\n");
                            send(socket, sentMessage, MESSAGE_SIZE, 0);
                            printPacket(sentMessage);
                            break;
                    }
                }
            }
        }
        if (time(NULL) - start >= 1) {
            printf("Timeout\n");
        }
        send(socket, sentMessage, MESSAGE_SIZE, 0);
        printPacket(sentMessage);
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

void printPacket(packet_t* packet) {
    #ifdef LOOPBACK
    printf("Origin: %d\n", packet->origin);
    #endif
    printf("VRC: %d\n", packet->vrc);
    printf("Sequence: %d\n", packet->sequence);
    printf("Type: %d\n", packet->type);
    printf("Size: %d\n", packet->size);
}

void sendMessage(int socket, packet_t* sentMessage, packet_t* receivedMessage) {
    send(socket, sentMessage, MESSAGE_SIZE, 0);
    printf("\nSent message:\n");
    printPacket(sentMessage);
    waitResponseTimeout(socket, sentMessage, receivedMessage);
}

int checkIntegrity(int socket, packet_t* sentMessage, packet_t* receivedMessage, int* sequence, int id) {
    if (receivedMessage->startDelimiter == '~') {
        unsigned char vrc = calculateVRC(receivedMessage);

        // Condição loopback
        #ifdef LOOPBACK
        if (receivedMessage->origin == id) {
            return 0;
        }
        #endif

        // Duplicate message
        if (receivedMessage->sequence == *sequence) {
            return 0;
        }

        // Sends an ACK if the sequence number received is lower than the expected
        if ((receivedMessage->sequence == (*sequence - 1)) || (receivedMessage->sequence == 63 && sequence == 0)) {
            sendAck(socket, sentMessage, receivedMessage);
            return 0;
        }
        // Sends an ACK if the sequence number received is higher than the expected
        else if ((receivedMessage->sequence > (*sequence + 1) % 64) || (receivedMessage->vrc != vrc)) {
            sendNack(socket, sentMessage, receivedMessage);
            return 0;
        }

        printf("\nReceived message:\n");
        printPacket(receivedMessage);

        return 1;
    }
    return 0;
}