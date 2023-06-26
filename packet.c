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
    memcpy(packet->data, data, size);
    packet->vrc = calculateVRC(packet);
}

void bufferToPacket(packet_t* packet, unsigned char* buffer) {
    packet->startDelimiter = buffer[0];
    #ifdef LOOPBACK
    packet->origin = buffer[1];
    packet->size = buffer[2] >> 2;
    packet->sequence = buffer[2] << 4 | buffer[3] >> 4;
    packet->type = buffer[3] & 0x0F;
    for (int i = 0; i < packet->size; i++) {
        packet->data[i] = buffer[i + 4];
    }
    packet->vrc = buffer[packet->size + 4];
    #else
    packet->size = buffer[1] >> 2;
    packet->sequence = buffer[1] << 4 | buffer[2] >> 4;
    packet->type = buffer[2] & 0x0F;
    for (int i = 0; i < packet->size; i++) {
        packet->data[i] = buffer[i + 3];
    }
    packet->vrc = buffer[packet->size + 3];
    #endif
}

unsigned char* packetToBuffer(packet_t* packet) {
    #ifdef LOOPBACK
    unsigned char* buffer = malloc(sizeof(unsigned char) * (packet->size+5));
    buffer[0] = packet->startDelimiter;
    buffer[1] = packet->origin;
    buffer[2] = packet->size << 2 | packet->sequence >> 4;
    buffer[3] = packet->sequence << 4 | packet->type;
    for (int i = 0; i < packet->size; i++) {
        buffer[i + 4] = packet->data[i];
    }
    buffer[packet->size + 4] = packet->vrc;
    #else
    unsigned char* buffer = malloc(sizeof(unsigned char) * (packet->size+5));
    buffer[0] = packet->startDelimiter;
    buffer[1] = packet->size << 2 | packet->sequence >> 4;
    buffer[2] = packet->sequence << 4 | packet->type;
    for (int i = 0; i < packet->size; i++) {
        buffer[i + 3] = packet->data[i];
    }
    buffer[packet->size + 3] = packet->vrc;
    #endif
    // printf("\nBuffer:\n");
    // for (int i = 0; i < packet->size + 4; i++) {
    //     printf("%02x", buffer[i]);
    // }
    return buffer;
}


void sendAck(int socket, packet_t* sentMessage, packet_t* receivedMessage) {
    makePacket(sentMessage, NULL, 0, receivedMessage->sequence, 14);
    printf("\nSent message:\n");
    printPacket(sentMessage);
    unsigned char* buffer = packetToBuffer(sentMessage);
    send(socket, buffer, MESSAGE_SIZE, 0);
    free(buffer);
}

void sendNack(int socket, packet_t* sentMessage, packet_t* receivedMessage) {
    makePacket(sentMessage, NULL, 0, receivedMessage->sequence, 15);
    printf("\nSent message:\n");
    printPacket(sentMessage);
    unsigned char* buffer = packetToBuffer(sentMessage);
    send(socket, buffer, MESSAGE_SIZE, 0);
    free(buffer);
}

int waitResponseTimeout(int socket, packet_t* sentMessage, packet_t* receivedMessage) {
    time_t start;
    unsigned char* sendBuffer = packetToBuffer(sentMessage);
    unsigned char* receivedBuffer = malloc(sizeof(unsigned char) * MESSAGE_SIZE);

    while (1) {
        start = time(NULL);
        while (time(NULL) - start < 1) {
            recv(socket, receivedBuffer, MESSAGE_SIZE, 0);
            bufferToPacket(receivedMessage, receivedBuffer);
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
                            free(sendBuffer);
                            free(receivedBuffer);
                            return 1;
                        case 14:
                            printf("ACK received\n");
                            free(sendBuffer);
                            free(receivedBuffer);
                            return 1;
                        case 15:
                            printf("NACK received\n");
                            send(socket, sendBuffer, MESSAGE_SIZE, 0);
                            printPacket(sentMessage);
                            break;
                    }
                }
            }
        }
        if (time(NULL) - start >= 1) {
            printf("Timeout\n");
        }
        send(socket, sendBuffer, MESSAGE_SIZE, 0);
        free(sendBuffer);
        free(receivedBuffer);
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
    printf("Origin: %d ", packet->origin);
    #endif
    printf("VRC: %d ", packet->vrc);
    printf("Sequence: %d ", packet->sequence);
    printf("Type: %d ", packet->type);
    printf("Size: %d\n", packet->size);
}

void sendMessage(int socket, packet_t* sentMessage, packet_t* receivedMessage) {
    unsigned char* buffer = packetToBuffer(sentMessage);
    send(socket, buffer, MESSAGE_SIZE, 0);
    free(buffer);
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