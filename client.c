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
#define ID 0

// CHECAR MAIS ERROS
// RECUPERAR VÁRIOS ARQUIVOS

void freeInputParsed(char** inputParsed, int capacity) {
    for (int i = 0; i < capacity; i++) {
        free(inputParsed[i]);
    }
    free(inputParsed);
}

int getCommand(char* input, char** inputParsed) {
    int capacity = 0;

    char* token = strtok(input, " ");
    inputParsed[0] = malloc(sizeof(char) * (strlen(token)+1));
    strcpy(inputParsed[0], token);
    capacity++;

    token = strtok(NULL, " ");
    if (token != NULL) {
        inputParsed[1] = malloc(sizeof(char) * (strlen(token)+1));
        strcpy(inputParsed[1], token);
        capacity++;
    }
    free(input);

    return capacity;
}

int main(int argc, char** argv) {
    int socket;
    socket = rawSocketConnection(ETHERNET);

    char* input;
    char** inputParsed;
    int capacity;
    glob_t* globbuf;

    int sequence = -1;
    
    packet_t* sentMessage = malloc(sizeof(packet_t));
    packet_t* receivedMessage = malloc(sizeof(packet_t));
    #ifdef LOOPBACK
    sentMessage->origin = 0;
    #endif

    unsigned char* clientMD5 = malloc(MD5_DIGEST_LENGTH * sizeof(unsigned char));
    unsigned char* serverPath = calloc(TAM_INPUT, sizeof(unsigned char));

    while (1) {
        input = malloc(TAM_INPUT * sizeof(char));
        fgets(input, TAM_INPUT, stdin);
        input[strcspn(input, "\n")] = '\0';
        inputParsed = malloc(2 * sizeof(char*));
        capacity = getCommand(input, inputParsed);
        // capacity = split(input, ' ', &inputParsed);
        if (inputParsed[0] == NULL) {
            freeInputParsed(inputParsed, capacity);
            continue;
        }
        else if (!(strcmp(inputParsed[0], "cd"))) {
            changeDirectory(inputParsed[1]);
        }
        else if (!(strcmp(inputParsed[0], "backup"))) {
            globbuf = malloc(sizeof(glob_t));
            glob(inputParsed[1], 0, NULL, globbuf);
            if (globbuf->gl_pathc == 1) {
                makeBackup(socket, sentMessage, receivedMessage, globbuf->gl_pathv[0], &sequence);
            } else {
                makeMultipleBackup(socket, sentMessage, receivedMessage, globbuf, &sequence);
            }
            free(globbuf);
        }
        else if (!(strcmp(inputParsed[0], "restore"))) {
            char* fileName = inputParsed[1];
            strcat(serverPath, fileName);
            globuff = malloc(sizeof(glob_t));
            glob((const char*) serverPath, 0, NULL, globbuf);
            if (globuff->gl_pathc == 1) {
                restoreBackup(socket, sentMessage, receivedMessage, globbuf->gl_pathv[0], &sequence);
            } else {
                restoreMultipleBackup(socket, sentMessage, receivedMessage, globbuf, &sequence);
            }
            free(globbuf);
        }
        else if (!(strcmp(inputParsed[0], "verify"))) {
            char* fileName = inputParsed[1];
            makePacket(sentMessage, (unsigned char*) fileName, strlen(fileName), (++sequence % MAX_SEQUENCE), 5);
            sendMessage(socket, sentMessage, receivedMessage);
            while (1) {
                receiveMessage(socket, sentMessage, receivedMessage, &sequence, ID);
                if (receivedMessage->type == 7) {
                    sendResponse(socket, sentMessage, receivedMessage, 14);
                    unsigned char* serverMD5 = malloc(sizeof(unsigned char) * (receivedMessage->size + 1));
                    memset(serverMD5, 0, receivedMessage->size + 1);
                    memcpy(serverMD5, receivedMessage->data, receivedMessage->size);
                    verifyBackup(fileName, serverMD5);
                    free(serverMD5);
                    break;
                }
            }
        }
        else if (!(strcmp(inputParsed[0], "setdir"))) {
            char* path = inputParsed[1];
            makePacket(sentMessage, (unsigned char*) path, strlen(path), (++sequence % MAX_SEQUENCE), 4);
            strcat(serverPath, path);
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
    free(serverPath);

    return 0;
}