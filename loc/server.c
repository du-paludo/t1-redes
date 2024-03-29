#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include "rawSocketConnection.h"
#include "fileHelper.h"
#include "packet.h"
#include "backup.h"
#include <openssl/md5.h>

#ifdef LOOPBACK
#define ETHERNET "lo"
#else
#define ETHERNET "enp3s0"
#endif
#define ID 1

int main(int argc, char **argv)
{
    unsigned char* data = malloc(sizeof(unsigned char) * DATA_SIZE);
    FILE* file;

    int socket = rawSocketConnection(ETHERNET);
    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (const char*) &tv, sizeof(tv));

    packet_t* sentMessage = malloc(sizeof(packet_t));
    #ifdef LOOPBACK
    sentMessage->origin = 1;
    #endif
    packet_t* receivedMessage = malloc(sizeof(packet_t));

    int sequence = -1;

    unsigned char* serverMD5;
    unsigned char* path;
    unsigned char* fileName;
    glob_t* globbuf;

    int nackSequence = 0;

    while (1) {
        receiveMessage(socket, sentMessage, receivedMessage, &sequence, ID, &nackSequence);
        switch (receivedMessage->type) {
            case 0:
                file = openFile(receivedMessage->data, "wb");
                sendResponse(socket, sentMessage, receivedMessage, 14);
                break;
            case 1:
                sendResponse(socket, sentMessage, receivedMessage, 14);
                break;
            case 2:
                sendResponse(socket, sentMessage, receivedMessage, 14);
                break;
            case 3:
                sendResponse(socket, sentMessage, receivedMessage, 14);
                globbuf = malloc(sizeof(glob_t));
                glob((const char*) receivedMessage->data, 0, NULL, globbuf);
                printf("globbuf->gl_pathc: %ld\n", globbuf->gl_pathc);
                for (int i = 0; i < globbuf->gl_pathc; i++) {
                    sendFile(socket, sentMessage, receivedMessage, globbuf->gl_pathv[i], &sequence);
                }
                free(globbuf);
                makePacket(sentMessage, NULL, 0, (++sequence % MAX_SEQUENCE), 10);
                sendMessage(socket, sentMessage, receivedMessage);
                break;
            case 4:
                path = malloc(sizeof(unsigned char) * (receivedMessage->size + 1));
                memset(path, 0, receivedMessage->size + 1);
                memcpy(path, receivedMessage->data, receivedMessage->size);
                printf("Path: %s\n", path);
                if (changeDirectory((char*) path)) {
                    sendResponse(socket, sentMessage, receivedMessage, 14);
                } else {
                    // Erro
                    sendResponse(socket, sentMessage, receivedMessage, 12);
                }
                sendResponse(socket, sentMessage, receivedMessage, 14);
                free(path);
                break;
            case 5:
                serverMD5 = malloc(sizeof(unsigned char)*MD5_DIGEST_LENGTH);
                fileName = malloc(sizeof(unsigned char) * (receivedMessage->size + 1));
                memset(fileName, 0, receivedMessage->size + 1);
                memcpy(fileName, receivedMessage->data, receivedMessage->size);
                if (getMD5Hash((char*) fileName, serverMD5)) {
                    sendResponse(socket, sentMessage, receivedMessage, 14);
                } else {
                    // Erro
                    sendResponse(socket, sentMessage, receivedMessage, 12);
                }
                makePacket(sentMessage, serverMD5, MD5_DIGEST_LENGTH, (++sequence % MAX_SEQUENCE), 7);
                sendMessage(socket, sentMessage, receivedMessage);
                printf("Sent message:\n");
                printPacket(sentMessage);
                free(serverMD5);
                free(fileName);
                break;
            case 8:
                for (int i = 0; i < receivedMessage->size; i++) {
                    // printf("%c", data[i]);
                }
                saveFile(file, receivedMessage->data, receivedMessage->size);
                sendResponse(socket, sentMessage, receivedMessage, 14);
                break;
            case 9:
                if (file) {
                    fclose(file);
                    file = NULL;
                }
                sendResponse(socket, sentMessage, receivedMessage, 14);
                break;
            case 10:
                sendResponse(socket, sentMessage, receivedMessage, 14);
                break;
        }
    }
    free(data);
    free(sentMessage);
    free(receivedMessage);

    return 0;
}