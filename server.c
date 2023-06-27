#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "rawSocketConnection.h"
#include "fileHelper.h"
#include "packet.h"
#include "backup.h"
#include <openssl/md5.h>

#define ETHERNET "lo"
#define ID 1

int main(int argc, char **argv)
{
    unsigned char *data = malloc(sizeof(unsigned char) * DATA_SIZE);
    FILE *file;

    int socket = rawSocketConnection(ETHERNET);
    packet_t *sentMessage = malloc(69);
    #ifdef LOOPBACK
    sentMessage->origin = 1;
    #endif
    packet_t *receivedMessage = malloc(69);

    int sequence = -1;

    unsigned char *serverMD5;

    while (1) {
        receiveMessage(socket, sentMessage, receivedMessage, &sequence, ID);
        switch (receivedMessage->type) {
            case 0:
                file = openFile(receivedMessage->data);
                sendResponse(socket, sentMessage, receivedMessage, 14);
                break;
            case 1:
                sendResponse(socket, sentMessage, receivedMessage, 14);
                break;
            case 2:
                sendResponse(socket, sentMessage, receivedMessage, 14);
                sendFile(socket, sentMessage, receivedMessage, (char*) receivedMessage->data, &sequence);
                break;
            case 4:
                if (changeDirectory((char*) receivedMessage->data)) {
                    sendResponse(socket, sentMessage, receivedMessage, 14);
                } else {
                    // Erro
                    sendResponse(socket, sentMessage, receivedMessage, 12);
                }
                sendResponse(socket, sentMessage, receivedMessage, 14);
                break;
            case 5:
                serverMD5 = malloc(sizeof(unsigned char)*MD5_DIGEST_LENGTH);
                if (getMD5Hash((char*) receivedMessage->data, serverMD5)) {
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
    free(serverMD5);

    return 0;
}