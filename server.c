#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "rawSocketConnection.h"
#include "fileHelper.h"
#include "packet.h"
#include "backup.h"
#include <openssl/md5.h>

#define ETHERNET "lo"

// arrumar arquivo a mais sendo criado
// implementar para-e-espera
// arrumar ack

int main(int argc, char** argv) {
    // char* command = malloc(sizeof(char) * 100);
    // char* path = malloc(sizeof(char) * 100);
    unsigned char* data = malloc(sizeof(unsigned char)*63);
    FILE* file;

    int socket; 
    packet_t* packet = malloc(sizeof(packet_t));
    packet_t* response = malloc(sizeof(packet_t));
    unsigned int type;
    unsigned int size;
    unsigned char vrc;
    int sequence = -1;
    unsigned char startDelimiter;

    unsigned char* serverMD5;

    socket = rawSocketConnection(ETHERNET);

    while (1) {
        if (recv(socket, packet, MESSAGE_SIZE, 0)) {
            startDelimiter = packet->startDelimiter;
            if (startDelimiter == '~') {
                data = packetToBuffer(packet);
                size = packet->size;
                type = packet->type;
                vrc = calculateVRC(packet);

                // Condição loopback
                if (packet->sequence == sequence || packet->type == 14 || packet->type == 15) {
                    continue;
                }

                if ((packet->sequence == sequence-1) || (packet->sequence == 63 && sequence == 0)) {
                    printf("%d %d\n", packet->sequence, sequence);
                    sendAck(socket, packet);
                    continue;
                }
                if ((packet->sequence > (sequence + 1) % 64) || (packet->vrc != vrc)) {
                    sendNack(socket, packet);
                    continue;
                }

                printPacket(packet);

                sequence = packet->sequence;

                switch (type) {
                    case 0:
                        file = openFile(data);
                        sendAck(socket, packet);
                        break;
                    case 1:
                        sendAck(socket, packet);
                        break;
                    case 4:
                        changeDirectory((char*) data);
                        sendAck(socket, packet);
                        break;
                    case 5:
                        printf("OK\n");
                        serverMD5 = getMD5Hash((char*) data);
                        for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
                            printf("%02x", serverMD5[i]);
                        }
                        // printf("OK\n");
                        makePacket(packet, serverMD5, MD5_DIGEST_LENGTH, 0, 7);
                        send(socket, packet, MESSAGE_SIZE, 0);
                        break;
                    case 8:
                        for (int i = 0; i < size; i++) {
                            // printf("%c", data[i]);
                        }
                        saveFile(file, data, size);
                        sendAck(socket, packet);
                        break;
                    case 9:
                        if (file) {
                            fclose(file);
                            file = NULL;
                        }
                        sendAck(socket, packet);
                        break;
                    case 10:
                        sendAck(socket, packet);
                        break;
                }
            }
        }
    }

    return 0;
}