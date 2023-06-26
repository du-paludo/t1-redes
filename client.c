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

int split (const char *str, char c, char ***arr) {
    int count = 1;
    int token_len = 1;
    int i = 0;
    char *p;
    char *t;

    p = str;
    while (*p != '\0')
    {
        if (*p == c)
            count++;
        p++;
    }

    *arr = (char**) malloc(sizeof(char*) * count);
    if (*arr == NULL)
        exit(1);

    p = str;
    while (*p != '\0')
    {
        if (*p == c)
        {
            (*arr)[i] = (char*) malloc( sizeof(char) * token_len );
            if ((*arr)[i] == NULL)
                exit(1);

            token_len = 0;
            i++;
        }
        p++;
        token_len++;
    }
    (*arr)[i] = (char*) malloc( sizeof(char) * token_len );
    if ((*arr)[i] == NULL)
        exit(1);

    i = 0;
    p = str;
    t = ((*arr)[i]);
    while (*p != '\0')
    {
        if (*p != c && *p != '\0')
        {
            *t = *p;
            t++;
        }
        else
        {
            *t = '\0';
            i++;
            t = ((*arr)[i]);
        }
        p++;
    }

    return count;
}

int main(int argc, char** argv) {
    int socket;
    socket = rawSocketConnection(ETHERNET);

    char* input = malloc(sizeof(char) * TAM_INPUT);
    char** inputParsed = NULL;
    int capacity;
    char* command;

    int sequence = -1;
    
    packet_t* sentMessage = malloc(69);
    #ifdef LOOPBACK
    sentMessage->origin = 0;
    #endif
    packet_t* receivedMessage = malloc(69);

    unsigned char* data = malloc(sizeof(unsigned char)*DATA_SIZE);

    unsigned char* clientMD5 = malloc(sizeof(unsigned char)*MD5_DIGEST_LENGTH);

    while (1) {
        fgets(input, TAM_INPUT, stdin);
        input[strcspn(input, "\n")] = 0;
        capacity = split(input, ' ', &inputParsed);
        command = inputParsed[0];

        if (!(strcmp(command, "cd"))) {
            char* path = inputParsed[1];
            changeDirectory(path);
        }
        else if (!(strcmp(command, "backup"))) {
            char* pattern = inputParsed[1];
            glob_t globbuf;
            glob(pattern, 0, NULL, &globbuf);
            if (globbuf.gl_pathc == 1) {
                char* fileName = globbuf.gl_pathv[0];
                makeBackup(socket, sentMessage, receivedMessage, fileName, &sequence);
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