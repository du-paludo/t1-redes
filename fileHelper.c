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

unsigned char* readFile(FILE* file) {
    unsigned char* buffer = malloc(sizeof(unsigned char) * DATA_SIZE);
    fread(buffer, sizeof(unsigned char), DATA_SIZE, file);
    // printf("%s", buffer);
    return buffer;
}

FILE* openFile(unsigned char* name) {
    printf("%s\n", name);
    FILE* file = fopen((const char*) name, "w");
    return file;
}

void saveFile(FILE* file, unsigned char* data, int size) {
    fwrite(data, sizeof(unsigned char), size, file);
}