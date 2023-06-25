#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "rawSocketConnection.h"
#include "fileHelper.h"
#include "packet.h"
#include "backup.h"
#include <openssl/md5.h>

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
    waitResponseTimeout(socket, packet, response, ((*sequence) % MAX_SEQUENCE));

    long fileSize = findFileSize(file);
    // printf("%ld\n", fileSize);
    int numberOfMessages = findNumberOfMessages(fileSize);
    // printf("%d\n", numberOfMessages);
    for (int i = 0; i < numberOfMessages-1; i++) {
        data = readFile(file);
        // printf("\n");
        makePacket(packet, data, DATA_SIZE, (++(*sequence) % MAX_SEQUENCE), 8);
        send(socket, packet, MESSAGE_SIZE, 0);
        waitResponseTimeout(socket, packet, response, ((*sequence) % MAX_SEQUENCE));
    }
    data = readFile(file);
    // printf("%s\n", data);
    // printf("%ld\n", fileSize % DATA_SIZE);

    makePacket(packet, data, (fileSize % DATA_SIZE), (++(*sequence) % MAX_SEQUENCE), 8);
    send(socket, packet, MESSAGE_SIZE, 0);
    waitResponseTimeout(socket, packet, response, ((*sequence) % MAX_SEQUENCE));

    makePacket(packet, NULL, 0, (++(*sequence) % MAX_SEQUENCE), 9);
    send(socket, packet, MESSAGE_SIZE, 0);
    waitResponseTimeout(socket, packet, response, ((*sequence) % MAX_SEQUENCE));

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

int verifyBackup(char* fileName, unsigned char* serverMD5) {
    unsigned char* clientMD5 = malloc(sizeof(unsigned char) * MD5_DIGEST_LENGTH);
    clientMD5 = getMD5Hash(fileName);
    printf("MD5 do arquivo no servidor: %s\n", clientMD5);
    printf("MD5 do arquivo no cliente: %s\n", serverMD5);

    if (strcmp((char*) clientMD5, (char*) serverMD5) == 0) {
        printf("Arquivo no servidor está igual ao arquivo no cliente.\n");
        return 1;
    } else {
        printf("Arquivo no servidor está diferente do arquivo no cliente.\n");
        return 0;
    }
}

unsigned char* getMD5Hash(char* fileName) {
    FILE* file = fopen(fileName, "r");
    
    unsigned char* MD5Hash = malloc(sizeof(unsigned char) * MD5_DIGEST_LENGTH);
    unsigned char* data = malloc(sizeof(unsigned char) * BUFFER_SIZE);

    MD5_CTX* md5CTX;
    MD5_Init(md5CTX);
    size_t bytes_read;

    while ((bytes_read = fread(data, 1, BUFFER_SIZE, file)) != 0) {
        MD5_Update(md5CTX, data, bytes_read);
    }

    MD5_Final(MD5Hash, md5CTX);

    return MD5Hash;
}