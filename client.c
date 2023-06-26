#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <glob.h>
#include "rawSocketConnection.h"
#include "packet.h"
#include "fileHelper.h"
#include "backup.h"
#include <openssl/md5.h>

#define ETHERNET "lo"
#define TAM_INPUT 100
#define MAX_SIZE 6
#define ID 0

int main(int argc, char** argv) {
    int socket;
    socket = rawSocketConnection(ETHERNET);

    char* input = malloc(sizeof(char) * TAM_INPUT);
    char** inputParsed = NULL;
    int capacity;
    char* command;
    glob_t globbuf;

    int sequence = -1;
    
    packet_t* sentMessage = malloc(sizeof(packet_t)+1);
    #ifdef LOOPBACK
    sentMessage->origin = 0;
    #endif
    packet_t* receivedMessage = malloc(sizeof(packet_t)+1);

    unsigned char* data = malloc(sizeof(unsigned char)*DATA_SIZE);

    unsigned char* clientMD5 = malloc(sizeof(unsigned char)*MD5_DIGEST_LENGTH);

    while (1) {
        fgets(input, TAM_INPUT, stdin);
        input[strcspn(input, "\n")] = 0;
        capacity = split(input, ' ', &inputParsed);
        command = inputParsed[0];

        if (!(strcmp(command, "cd"))) {
            changeDirectory(inputParsed[1]);
        }
        else if (!(strcmp(command, "backup"))) {
            glob(inputParsed[1], 0, NULL, &globbuf);
            if (globbuf.gl_pathc == 1) {
                makeBackup(socket, sentMessage, receivedMessage, globbuf.gl_pathv[0], &sequence);
            } else {
                makeMultipleBackup(socket, sentMessage, receivedMessage, &globbuf, &sequence);
            }
        } else if (!(strcmp(command, "restore"))) {
            char* fileName = inputParsed[1];
            restoreBackup(socket, sentMessage, receivedMessage, fileName, &sequence);
        }
        else if (!(strcmp(command, "verify"))) {
            char* fileName = inputParsed[1];
            makePacket(sentMessage, (unsigned char*) fileName, strlen(fileName), (++sequence % MAX_SEQUENCE), 5);
            sendMessage(socket, sentMessage, receivedMessage);
            packetToBuffer(receivedMessage, data);
            unsigned char* serverMD5 = receivedMessage->data;
            verifyBackup(fileName, clientMD5, serverMD5);
        }
        else if (!(strcmp(command, "setdir"))) {
            char* path = inputParsed[1];
            makePacket(sentMessage, (unsigned char*) path, strlen(path), (++sequence % MAX_SEQUENCE), 4);
            sendMessage(socket, sentMessage, receivedMessage);
        }
        else if (!(strcmp(command, "exit"))) {
            break;
        } else {
            printf("Comando inválido\n");
        }
    }

    free(sentMessage);
    free(receivedMessage);
    free(input);

    for (int i = 0; i < capacity; i++) {
        free(inputParsed[i]);
        i++;
    }
    free(inputParsed);

    free(clientMD5);
    free(data);

    return 0;
}