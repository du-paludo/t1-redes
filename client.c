#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ConexaoRawSocket.h"
#include "rawSocketConnection.h"
#include "packet.h"
#include "fileHelper.h"

#define ETHERNET "lo"


char** parseCommand(char* command) {
    int capacity = 2;
    char** commandParsed = (char**) malloc(capacity * sizeof(char*));
    char* copy = malloc(sizeof(char)*200);
    strcpy(copy, command);
    char* token;
    token = strtok(copy, " ");
    printf("fssfs", copy);
    int numberOfParts = 0;

    while (token != NULL) { 
        if (numberOfParts >= capacity) {
            capacity += 1;
            commandParsed = (char**) realloc(commandParsed, capacity * sizeof(char*));
        }

        commandParsed[numberOfParts] = strdup(token);
        printf("%s", commandParsed[numberOfParts]);
        numberOfParts++;
        token = strtok(NULL, " ");
    }

    free(copy);
    return commandParsed;
}

int main(int argc, char** argv) {
    char* command = malloc(sizeof(char) * 100);
    char** commandParsed;

    char* file_name = malloc(sizeof(char) * 100); 
    FILE* file;
    // char* path = malloc(sizeof(char) * 100);

    int socket;
    unsigned char* data;
    packet_t* packet;

    unsigned char* buffer;

    // socket = rawSocketConnection(ETHERNET);


    while (1) {
        scanf("%s", command);
        commandParsed = parseCommand(command);
        int numParts;
        // printf("%s\n %s\n", commandParsed[0], commandParsed[1]);

        // if (!(strcmp(command, "send"))) {
        //     // while (1){
        //         send(socket, packet, 67, 0);
        //     // }
        // } else if (!(strcmp(command, "backup"))) {
        //     file = fopen("teste.txt", "r");
        //     if (!file) {
        //         printf("Erro ao abrir o arquivo.");
        //         continue;
        //     }
        //     long fileSize = findFileSize(file);
        //     printf("%ld\n", fileSize);
        //     int numberOfMessages = findNumberOfMessages(fileSize);
        //     printf("%d\n", numberOfMessages);
        //     for (int i = 0; i < numberOfMessages; i++) {
        //         buffer = readFile(file);
        //         packet = makePacket(buffer, DATA_SIZE, 1, 1);
        //         printf("%s\n", packet);
        //         send(socket, packet, MESSAGE_SIZE, 0);
        //     }
        //     fclose(file);
        // } else if (!(strcmp(command, "exit"))) {
        //     break;
        // }
    }

    /* if (argc > 1) {
        command = argv[1];
        if (strcmp(command, "cd") == 0) {
            if (argc > 2) {
                path = argv[2];
                chdir(path);
            }
        }
    } */

    /* int sock_r;
    sock_r = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sock_r < 0) {
        print("error in socket \n");
        return -1;
    }

    unsigned char *buffer = (unsigned char *) malloc(65536); //to receive data
    memset(buffer,0,65536);
    struct sockaddr saddr;
    int saddr_len = sizeof (saddr);
    
    // Receive a network packet and copy in to buffer
    buflen = recvfrom(sock_r,buffer,65536,0,&saddr,(socklen_t *)&saddr_len);
    if (buflen < 0) {
        printf("error in reading recvfrom function\n");
        return -1;
    }

    return 0; */
}
