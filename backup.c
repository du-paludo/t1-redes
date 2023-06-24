#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "rawSocketConnection.h"
#include "fileHelper.h"
#include "packet.h"
#include "backup.h"

int makeBackup(int socket, char* fileName) {
    FILE* file = fopen(fileName, "r");
    unsigned char* data;

    if (!file) {
        printf("Erro ao abrir o arquivo.");
        return -1;
    }
    packet_t* packet = makePacket((unsigned char*) fileName, strlen(fileName) + 1, 0, 0);
    send(socket, packet, MESSAGE_SIZE, 0);  

    long fileSize = findFileSize(file);
    int numberOfMessages = findNumberOfMessages(fileSize);
    for (int i = 0; i < numberOfMessages-1; i++) {
        data = readFile(file);
        packet = makePacket(data, DATA_SIZE, 1, 8);
        send(socket, packet, MESSAGE_SIZE, 0);
    }
    data = readFile(file);
    packet = makePacket(data, (fileSize % DATA_SIZE), 1, 8);
    send(socket, packet, MESSAGE_SIZE, 0);

    packet = makePacket(NULL, 0, 1, 9);
    send(socket, packet, MESSAGE_SIZE, 0);
    fclose(file);

    return 0;
}

void changeDirectory(char* path) {
    if (chdir(path) != 0) {
        printf("Directory doesn't exist\n");
    }
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Current working directory: %s\n", cwd);
    } else {
        perror("getcwd() error");
    }
}
