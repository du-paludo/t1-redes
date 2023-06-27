#ifndef BACKUP_H
#define BACKUP_H

#include <glob.h>
#include "packet.h"

#define PATH_MAX 128

int makeBackup(int socket, packet_t* packet, packet_t* response, char* fileName, int* sequence);

void makeMultipleBackup(int socket, packet_t* packet, packet_t* response, glob_t* globbuf, int* sequence);

void restoreBackup(int socket, packet_t* sentMessage, packet_t* receivedMessage, char* pattern, int* sequence);

void restoreMultipleBackup(int socket, packet_t* sentMessage, packet_t* receivedMessage, glob_t* globbuf, int* sequence);

void makeMultipleBackup(int socket, packet_t* sentMessage, packet_t* receivedMessage, glob_t* globbuf, int* sequence);

int changeDirectory(char* path);

int getMD5Hash(char* fileName, unsigned char* MD5Hash);

int verifyBackup(char* fileName, unsigned char* serverMD5);

int sendFile(int socket, packet_t* sentMessage, packet_t* receivedMessage, char* fileName, int* sequence);

#endif