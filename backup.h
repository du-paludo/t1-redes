#ifndef BACKUP_H
#define BACKUP_H

#define PATH_MAX 128

int makeBackup(int socket, char* fileName, int* sequence);

void changeDirectory(char* path);

unsigned char* getMD5Hash(char* fileName);

int verifyBackup(char* fileName, unsigned char* serverMD5);

#endif