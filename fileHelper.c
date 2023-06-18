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
    int numberOfMessages = ceil(fileSize/63);
    return numberOfMessages;
}

unsigned char* readFile(FILE* file) {
    unsigned char* buffer = malloc(sizeof(unsigned char) * DATA_SIZE);
    fread(buffer, sizeof(unsigned char), DATA_SIZE, file);
    return buffer;
}

FILE* openFile() {
    FILE* file = fopen("teste.txt", "w");
    return file;
}

void saveFile(FILE* file, unsigned char* data) {
    fprintf(file, data);
}