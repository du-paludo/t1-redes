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

FILE* openFile(unsigned char* name) {
    printf("%s\n", name);
    FILE* file = fopen((const char*) name, "wb");
    return file;
}

void saveFile(FILE* file, unsigned char* data, int size) {
    fwrite(data, sizeof(unsigned char), size, file);
}

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