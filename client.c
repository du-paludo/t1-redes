#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <glob.h>
#include "rawSocketConnection.h"
#include "packet.h"
#include "fileHelper.h"
#include "backup.h"

#define ETHERNET "lo"
#define TAM_INPUT 100

char* strdup(const char *c)
{
    char *dup = malloc(strlen(c) + 1);

    if (dup != NULL)
       strcpy(dup, c);

    return dup;
}

char** parseInput(char* input) {
    int capacity = 2;
    char** inputParsed = (char**) malloc(capacity * sizeof(char*));
    char* copy = strdup((const char*) input);
    char* token;
    token = strtok(copy, " ");
    int numberOfParts = 0;

    while (token != NULL) { 
        if (numberOfParts >= capacity) {
            capacity += 1;
            inputParsed = (char**) realloc(inputParsed, capacity * sizeof(char*));
        }
        inputParsed[numberOfParts] = strdup(token);
        numberOfParts++;
        token = strtok(NULL, " ");
    }

    free(copy);
    return inputParsed;
}

int main(int argc, char** argv) {
    int socket;
    socket = rawSocketConnection(ETHERNET);

    char* input = malloc(sizeof(char) * TAM_INPUT);
    char** inputParsed;
    char* command;

    int sequence = -1;
    int hasReceivedAck = 0;

    packet_t* packet = malloc(sizeof(packet_t));
    packet_t* response = malloc(sizeof(packet_t));
    // char* path = malloc(sizeof(char) * 100);

    while (1) {
        fgets(input, TAM_INPUT, stdin);
        input[strcspn(input, "\n")] = 0;
        inputParsed = parseInput(input);
        command = inputParsed[0];

        if (!(strcmp(command, "cd"))) {
            char* path = inputParsed[1];
            changeDirectory(path);
        }
        else if (!(strcmp(command, "backup"))) {
            char* pattern = inputParsed[1];
            glob_t globbuf;
            glob(pattern, 0, NULL, &globbuf);
            for (int i = 0; i < globbuf.gl_pathc; i++) {
                // Envia mensagem de início de grupo de arquivos
                makePacket(packet, NULL, 0, (++sequence % MAX_SEQUENCE), 1);
                send(socket, packet, MESSAGE_SIZE, 0);
                waitResponse(socket, packet, response, sequence);

                // Para cada arquivo, faz backup
                char* fileName = globbuf.gl_pathv[i];
                printf("%s\n", fileName);
                makeBackup(socket, fileName, &sequence);
            }
        } else if (!(strcmp(command, "restore"))) {
            char* pattern = inputParsed[1];
            glob_t globbuf;
            glob(pattern, 0, NULL, &globbuf);
            for (int i = 0; i < globbuf.gl_pathc; i++) {
                // char* fileName = globbuf.gl_pathv[i];
                // restoreBackup();
            }
        }
        else if (!(strcmp(command, "setdir"))) {
            char* path = inputParsed[1];
            makePacket(packet, (unsigned char*) path, strlen(path), (++sequence % MAX_SEQUENCE), 4);
            send(socket, packet, MESSAGE_SIZE, 0);
            waitResponse(socket, response, packet, sequence);
        }
        else if (!(strcmp(command, "exit"))) {
            break;
        } else {
            printf("Comando inválido\n");
        }
    }

    free(packet);
    free(response);
    return 0;
}