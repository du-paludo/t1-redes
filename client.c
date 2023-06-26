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

#define ETHERNET "enp3s0"
#define TAM_INPUT 100
#define MAX_SIZE 6
#define ID 0

void freeInputParsed(char** inputParsed, int capacity) {
    for (int i = 0; i < capacity; i++) {
        free(inputParsed[i]);
    }
    free(inputParsed);
}

int main(int argc, char** argv) {
    int socket;
    socket = rawSocketConnection(ETHERNET);

    char* input;
    char** inputParsed = NULL;
    int capacity;
    glob_t globbuf;

    int sequence = -1;
    
    packet_t* sentMessage = calloc(1, sizeof(packet_t)+1);
    packet_t* receivedMessage = calloc(1, sizeof(packet_t)+1);
    #ifdef LOOPBACK
    sentMessage->origin = 0;
    #endif

    unsigned char* clientMD5 = calloc(MD5_DIGEST_LENGTH, sizeof(unsigned char));

    while (1) {
        input = calloc(TAM_INPUT, sizeof(char));
        fgets(input, TAM_INPUT, stdin);
        input[strcspn(input, "\n")] = 0;
        capacity = split(input, ' ', &inputParsed);
        free(input);
        if (inputParsed[0] == NULL) {
            freeInputParsed(inputParsed, capacity);
            continue;
        }
        else if (!(strcmp(inputParsed[0], "cd"))) {
            changeDirectory(inputParsed[1]);
        }
        else if (!(strcmp(inputParsed[0], "backup"))) {
            glob(inputParsed[1], 0, NULL, &globbuf);
            if (globbuf.gl_pathc == 1) {
                makeBackup(socket, sentMessage, receivedMessage, globbuf.gl_pathv[0], &sequence);
            } else {
                makeMultipleBackup(socket, sentMessage, receivedMessage, &globbuf, &sequence);
            }
        }
        else if (!(strcmp(inputParsed[0], "restore"))) {
            char* fileName = inputParsed[1];
            restoreBackup(socket, sentMessage, receivedMessage, fileName, &sequence);
        }
        else if (!(strcmp(inputParsed[0], "verify"))) {
            char* fileName = inputParsed[1];
            makePacket(sentMessage, (unsigned char*) fileName, strlen(fileName), (++sequence % MAX_SEQUENCE), 5);
            sendMessage(socket, sentMessage, receivedMessage);
            // packetToBuffer(receivedMessage, data);
            unsigned char* serverMD5 = receivedMessage->data;
            verifyBackup(fileName, clientMD5, serverMD5);
        }
        else if (!(strcmp(inputParsed[0], "setdir"))) {
            char* path = inputParsed[1];
            makePacket(sentMessage, (unsigned char*) path, strlen(path), (++sequence % MAX_SEQUENCE), 4);
            sendMessage(socket, sentMessage, receivedMessage);
        }
        else if (!(strcmp(inputParsed[0], "exit"))) {
            freeInputParsed(inputParsed, capacity);
            break;
        }
        else {
            printf("Comando inválido\n");
        }
        freeInputParsed(inputParsed, capacity);
    }

    free(sentMessage);
    free(receivedMessage);
    free(clientMD5);

    return 0;
}