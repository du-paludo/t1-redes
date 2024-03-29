#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <glob.h>
#include "rawSocketConnection.h"
#include "packet.h"
#include "fileHelper.h"
#include "backup.h"
#include <openssl/md5.h>

#define ETHERNET "enp3s0"
#define TAM_INPUT 200
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

    if (input[0] == '\0') {
        return capacity;
    }

    char* token = strtok(input, " ");
    inputParsed[0] = malloc(sizeof(char) * (strlen(token)+1));
    strcpy(inputParsed[0], token);
    capacity++;

    if (token == NULL) {
        return capacity;
    }

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

    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (const char*) &tv, sizeof(tv));

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
    strcpy((char*) serverPath, "/home/mint/t1-redes/");

    while (1) {
        input = malloc(TAM_INPUT * sizeof(char));
        fgets(input, TAM_INPUT, stdin);
        input[strcspn(input, "\n")] = '\0';
        inputParsed = malloc(2 * sizeof(char*));
        capacity = getCommand(input, inputParsed);
        if (capacity == 0) {
            freeInputParsed(inputParsed, capacity);
            continue;
        }
        else if (!(strcmp(inputParsed[0], "cd"))) {
            if (capacity == 1) {
                printf("Parâmetro nulo\n");
                freeInputParsed(inputParsed, capacity);
                continue;
            }
            changeDirectory(inputParsed[1]);
        }
        else if (!(strcmp(inputParsed[0], "backup"))) {
            if (capacity == 1) {
                printf("Parâmetro nulo\n");
                freeInputParsed(inputParsed, capacity);
                continue;
            }
            globbuf = malloc(sizeof(glob_t));
            glob(inputParsed[1], 0, NULL, globbuf);
            if (globbuf->gl_pathc == 0) {
                printf("Arquivo inexistente\n");
            }
            else if (globbuf->gl_pathc == 1) {
                makeBackup(socket, sentMessage, receivedMessage, globbuf->gl_pathv[0], &sequence);
            } else {
                makeMultipleBackup(socket, sentMessage, receivedMessage, globbuf, &sequence);
            }
            free(globbuf);
        }
        else if (!(strcmp(inputParsed[0], "restore"))) {
            if (capacity == 1) {
                printf("Parâmetro nulo\n");
                freeInputParsed(inputParsed, capacity);
                continue;
            }
            char* pattern = inputParsed[1];
            restoreBackup(socket, sentMessage, receivedMessage, pattern, &sequence);
        }
        else if (!(strcmp(inputParsed[0], "verify"))) {
            if (capacity == 1) {
                printf("Parâmetro nulo\n");
                freeInputParsed(inputParsed, capacity);
                continue;
            }
            char* fileName = inputParsed[1];
            makePacket(sentMessage, (unsigned char*) fileName, strlen(fileName), (++sequence % MAX_SEQUENCE), 5);
            sendMessage(socket, sentMessage, receivedMessage);
            while (1) {
                receiveMessage(socket, sentMessage, receivedMessage, &sequence, ID, 0);
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
            if (capacity == 1) {
                printf("Parâmetro nulo\n");
                freeInputParsed(inputParsed, capacity);
                continue;
            }
            char* path = inputParsed[1];
            makePacket(sentMessage, (unsigned char*) path, strlen(path), (++sequence % MAX_SEQUENCE), 4);
            strcat((char*) serverPath, path);
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