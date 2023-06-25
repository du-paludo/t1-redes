#ifndef BACKUP_H
#define BACKUP_H

#include <glob.h>

#define PATH_MAX 128

int makeBackup(int socket, char* fileName, int* sequence);

void makeMultipleBackup(int socket, packet_t* packet, packet_t* response, glob_t* globbuf, int* sequence);

void changeDirectory(char* path);

unsigned char* getMD5Hash(char* fileName);

int verifyBackup(char* fileName, unsigned char* serverMD5);

#endif