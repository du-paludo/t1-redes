#ifndef FILE_HELPER_H
#define FILE_HELPER_H

void readFile(FILE* file, unsigned char *buffer);

int findFileSize(FILE* file);

int findNumberOfMessages(long fileSize);

FILE* openFile();

void saveFile(FILE* file, unsigned char* data, int size);

#endif