#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "fileHelper.h"
#include "packet.h"

int findFileSize(FILE* file) {
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    return size;
}

int findNumberOfMessages(long fileSize) {
    int numberOfMessages = ceil((double)fileSize/DATA_SIZE);
    // printf("%ld %d %d\n", fileSize, DATA_SIZE, numberOfMessages);
    return numberOfMessages;
}

void readFile(FILE* file, unsigned char *buffer) {
    fread(buffer, sizeof(unsigned char), DATA_SIZE, file);
    // printf("%s", buffer);
}

FILE* openFile(unsigned char* name, const char* mode) {
    printf("%s\n", name);
    FILE* file = fopen((const char*) name, mode);
    if (!file) {
        printf("Erro ao abrir o arquivo.");
        return NULL;
    }
    return file;
}

void saveFile(FILE* file, unsigned char* data, int size) {
    fwrite(data, sizeof(unsigned char), size, file);
}