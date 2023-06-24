#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "rawSocketConnection.h"
#include "fileHelper.h"
#include "packet.h"
#include "backup.h"

#define ETHERNET "lo"

// arrumar arquivo a mais sendo criado
// implementar para-e-espera
// arrumar ack

int main(int argc, char** argv) {
    // char* command = malloc(sizeof(char) * 100);
    // char* path = malloc(sizeof(char) * 100);
    unsigned char* data = malloc(8*63);
    FILE* file;

    int socket; 
    packet_t* packet = malloc(sizeof(packet_t));
    int type;
    int size;
    int sequence = -1;
    unsigned char startDelimiter;

    socket = rawSocketConnection(ETHERNET);

    while (1) {
        if (recv(socket, packet, MESSAGE_SIZE, 0)) {
            startDelimiter = packet->startDelimiter;
            if (startDelimiter == '~') {
                data = packetToBuffer(packet);
                size = packet->size;
                type = packet->type;
                printf("%s %d %d %d\n", data, size, packet->sequence, type);

                // Condição loopback
                if (packet->sequence == sequence) {
                    continue;
                }
                if (packet->sequence != sequence + 1) {
                    sendNack(socket, packet);
                }
                printf("OK\n");
                sequence = packet->sequence;

                switch (type) {
                    case 0:
                        file = openFile(data);
                        break;
                    case 1:

                        break;
                    case 4:
                        changeDirectory((char*) data);
                        break;
                    case 8:
                        for (int i = 0; i < size; i++) {
                            printf("%c", data[i]);
                        }
                        saveFile(file, data, size);
                        break;
                    case 9:
                        if (file) {
                            fclose(file);
                            file = NULL;
                        }
                        break;
                }
                sendAck(socket, packet);
            }
        }
    }

    return 0;
}