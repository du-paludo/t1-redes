#ifndef BACKUP_H
#define BACKUP_H

#define PATH_MAX 128

int makeBackup(int socket, char* fileName);

void changeDirectory(char* path);

#endif