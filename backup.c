#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "rawSocketConnection.h"
#include "fileHelper.h"
#include "packet.h"
#include "backup.h"
#include <glob.h>
#include <openssl/md5.h>

int makeBackup(int socket, packet_t* packet, packet_t* response, char* fileName, int* sequence) {
    FILE* file = fopen(fileName, "rb");
    unsigned char* data = malloc(sizeof(unsigned char)*DATA_SIZE);

    if (!file) {
        printf("Erro ao abrir o arquivo.");
        return -1;
    }
    makePacket(packet, (unsigned char*) fileName, strlen(fileName) + 1, (++(*sequence) % MAX_SEQUENCE), 0);
    sendMessage(socket, packet, response);

    long fileSize = findFileSize(file);
    // printf("%ld\n", fileSize);
    int numberOfMessages = findNumberOfMessages(fileSize);
    // printf("%d\n", numberOfMessages);
    for (int i = 0; i < numberOfMessages-1; i++) {
        readFile(file, data, DATA_SIZE);
        // printf("\n");
        makePacket(packet, data, DATA_SIZE, (++(*sequence) % MAX_SEQUENCE), 8);
        sendMessage(socket, packet, response);
    }
    readFile(file, data, fileSize % DATA_SIZE);
    // printf("%s\n", data);
    // printf("%ld\n", fileSize % DATA_SIZE);

    makePacket(packet, data, (fileSize % DATA_SIZE), (++(*sequence) % MAX_SEQUENCE), 8);
    sendMessage(socket, packet, response);

    makePacket(packet, NULL, 0, (++(*sequence) % MAX_SEQUENCE), 9);
    sendMessage(socket, packet, response);

    fclose(file);
    free(data);
    return 0;
}

void makeMultipleBackup(int socket, packet_t* sentMessage, packet_t* receivedMessage, glob_t* globbuf, int* sequence) {
    // Envia mensagem de início de grupo de arquivos
    makePacket(sentMessage, NULL, 0, (++(*sequence) % MAX_SEQUENCE), 1);
    sendMessage(socket, sentMessage, receivedMessage);

    for (int i = 0; i < globbuf->gl_pathc; i++) {
        // Para cada arquivo, faz backup
        char* fileName = globbuf->gl_pathv[i];
        printf("%s\n", fileName);
        makeBackup(socket, sentMessage, receivedMessage, fileName, sequence);
    }

    // Envia mensagem de fim de grupo de arquivos
    makePacket(sentMessage, NULL, 0, (++(*sequence) % MAX_SEQUENCE), 10);
    sendMessage(socket, sentMessage, receivedMessage);
}

void restoreBackup(int socket, packet_t* sentMessage, packet_t* receivedMessage, char* fileName, int* sequence) {
    unsigned char* buffer = malloc(sizeof(unsigned char)*MESSAGE_SIZE);

    FILE* file = fopen(fileName, "wb");
    if (!file) {
        printf("Erro ao abrir o arquivo.");
        return;
    }
    
    makePacket(sentMessage, (unsigned char*) fileName, strlen(fileName) + 1, (++(*sequence) % MAX_SEQUENCE), 2);
    sendMessage(socket, sentMessage, receivedMessage);

    unsigned char* data = malloc(sizeof(unsigned char)*DATA_SIZE);
    while (1) {
        receiveMessage(socket, sentMessage, receivedMessage, sequence, 0);
        switch (receivedMessage->type) {
        case 8:
            printf("Data received\n");
            saveFile(file, receivedMessage->data, receivedMessage->size);
            sendResponse(socket, sentMessage, receivedMessage, 14);
            break;
        case 9:
            printf("End of file\n");
            sendResponse(socket, sentMessage, receivedMessage, 14);
            fclose(file);
            free(data);
            return;
        }
    }
    free(buffer);
}

// void restoreMultipleBackup(int socket, packet_t* packet, packet_t* response, glob_t* globbuf, int* sequence) {
//     // Envia mensagem de início de grupo de arquivos
//     makePacket(packet, NULL, 0, (++(*sequence) % MAX_SEQUENCE), 3);
//     sendMessage(socket, packet, response);
    
//     for (int i = 0; i < globbuf->gl_pathc; i++) {
//         // Para cada arquivo, faz backup
//         char* fileName = globbuf->gl_pathv[i];
//         printf("%s\n", fileName);
//         restoreBackup(socket, fileName, sequence);
//     }

//     // Envia mensagem de fim de grupo de arquivos
//     makePacket(packet, NULL, 0, (++(*sequence) % MAX_SEQUENCE), 10);
//     sendMessage(socket, packet, response);
// }

int changeDirectory(char* path) {
    if (chdir(path) != 0) {
        printf("Directory %s doesn't exist\n", path);
        return 0;
    }
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Current working directory: %s\n", cwd);
    } else {
        perror("getcwd() error");
    }
    return 1;
}

int verifyBackup(char* fileName, unsigned char* serverMD5) {
    unsigned char* clientMD5 = malloc(sizeof(unsigned char)*(MD5_DIGEST_LENGTH + 1));
    memset(clientMD5, 0, MD5_DIGEST_LENGTH + 1);
    if (!getMD5Hash(fileName, clientMD5)) {
        printf("Erro ao abrir o arquivo.");
        return -1;
    }

    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        printf("%02x", clientMD5[i]);
    }
    printf("\n");
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        printf("%02x", serverMD5[i]);
    }
    printf("\n");

    if (strcmp((char*) clientMD5, (char*) serverMD5) == 0) {
        printf("Arquivo no servidor está igual ao arquivo no cliente.\n");
        free(clientMD5);
        return 1;
    } else {
        printf("Arquivo no servidor está diferente do arquivo no cliente.\n");
        free(clientMD5);
        return 0;
    }
}

int getMD5Hash(char* fileName, unsigned char* MD5Hash) {
    FILE* file = fopen(fileName, "rb");
    if (!file) {
        printf("Erro ao abrir o arquivo.");
        return 0;
    }
    
    unsigned char* data = malloc(sizeof(unsigned char) * BUFFER_SIZE);
    memset(data, 0, BUFFER_SIZE);
    MD5_CTX* md5CTX = malloc(sizeof(MD5_CTX));
    MD5_Init(md5CTX);
    size_t bytes_read;

    while ((bytes_read = fread(data, 1, BUFFER_SIZE, file)) != 0) {
        MD5_Update(md5CTX, data, bytes_read);
    }

    MD5_Final(MD5Hash, md5CTX);

    fclose(file);
    free(md5CTX);
    free(data);

    return 1;
}

int sendFile(int socket, packet_t* sentMessage, packet_t* receivedMessage, char* fileName, int* sequence) {
    FILE* file = fopen(fileName, "rb");
    unsigned char* data = malloc(sizeof(unsigned char)*DATA_SIZE);

    if (!file) {
        printf("Erro ao abrir o arquivo.");
        return -1;
    }

    long fileSize = findFileSize(file);
    printf("%ld\n", fileSize);
    int numberOfMessages = findNumberOfMessages(fileSize);
    printf("%d\n", numberOfMessages);
    for (int i = 0; i < numberOfMessages-1; i++) {
        readFile(file, data, DATA_SIZE);
        // printf("\n");
        makePacket(sentMessage, data, DATA_SIZE, (++(*sequence) % MAX_SEQUENCE), 8);
        sendMessage(socket, sentMessage, receivedMessage);
    }
    readFile(file, data, fileSize % DATA_SIZE);

    makePacket(sentMessage, data, (fileSize % DATA_SIZE), (++(*sequence) % MAX_SEQUENCE), 8);
    sendMessage(socket, sentMessage, receivedMessage);

    makePacket(sentMessage, NULL, 0, (++(*sequence) % MAX_SEQUENCE), 9);
    sendMessage(socket, sentMessage, receivedMessage);

    fclose(file);
    free(data);
    return 0;
}