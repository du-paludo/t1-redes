#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "rawSocketConnection.h"
#include "fileHelper.h"
#include "packet.h"
#include "backup.h"

int makeBackup(int socket, char* fileName, int* sequence) {
    FILE* file = fopen(fileName, "r");
    unsigned char* data;

    if (!file) {
        printf("Erro ao abrir o arquivo.");
        return -1;
    }
    packet_t* packet = malloc(sizeof(packet_t));
    packet_t* response = malloc(sizeof(packet_t));
    makePacket(packet, (unsigned char*) fileName, strlen(fileName) + 1, (++(*sequence) % MAX_SEQUENCE), 0);
    send(socket, packet, MESSAGE_SIZE, 0);
    waitResponse(socket, response, packet, *sequence);

    long fileSize = findFileSize(file);
    // printf("%ld\n", fileSize);
    int numberOfMessages = findNumberOfMessages(fileSize);
    // printf("%d\n", numberOfMessages);
    for (int i = 0; i < numberOfMessages-1; i++) {
        data = readFile(file);
        // printf("\n");
        makePacket(packet, data, DATA_SIZE, (++(*sequence) % MAX_SEQUENCE), 8);
        send(socket, packet, MESSAGE_SIZE, 0);
        waitResponse(socket, response, packet, *sequence);
    }
    data = readFile(file);
    // printf("%s\n", data);
    // printf("%ld\n", fileSize % DATA_SIZE);

    makePacket(packet, data, (fileSize % DATA_SIZE), (++(*sequence) % MAX_SEQUENCE), 8);
    send(socket, packet, MESSAGE_SIZE, 0);
    waitResponse(socket, response, packet, *sequence);

    makePacket(packet, NULL, 0, (++(*sequence) % MAX_SEQUENCE), 9);
    send(socket, packet, MESSAGE_SIZE, 0);
    waitResponse(socket, response, packet, *sequence);

    fclose(file);
    free(packet);
    free(response);
    return 0;
}

void restoreBackup(int socket, char* fileName, int* sequence) {

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
