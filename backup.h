#ifndef BACKUP_H
#define BACKUP_H

#define PATH_MAX 128

int makeBackup(int socket, char* fileName, int* sequence);

void changeDirectory(char* path);

#endif