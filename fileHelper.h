#ifndef FILE_HELPER_H
#define FILE_HELPER_H

unsigned char* readFile(FILE* file);

int findFileSize(FILE* file);

int findNumberOfMessages(long fileSize);

FILE* openFile();

void saveFile(FILE* file, unsigned char* data, int size);

#endif