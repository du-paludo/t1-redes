#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "rawSocketConnection.h"
#include "fileHelper.h"
#include "packet.h"
#include "backup.h"
#include <openssl/md5.h>

#define ETHERNET "enp3s0"
#define ID 1

int main(int argc, char** argv) {
    unsigned char* data = malloc(sizeof(unsigned char)*DATA_SIZE);
    FILE* file;

    int socket = rawSocketConnection(ETHERNET);
    packet_t* sentMessage = malloc(69);
    #ifdef LOOPBACK
    sentMessage->origin = 1;
    #endif
    packet_t* receivedMessage = malloc(69);

    int sequence = -1;

    unsigned char* serverMD5 = malloc(sizeof(unsigned char)*MD5_DIGEST_LENGTH);
    unsigned char* buffer = malloc(sizeof(unsigned char)*MESSAGE_SIZE);

    while (1) {
        if (recv(socket, buffer, MESSAGE_SIZE, 0)) {
            bufferToPacket(receivedMessage, buffer);
            if (checkIntegrity(socket, sentMessage, receivedMessage, &sequence, ID)) {
                for (int i = 0; i < receivedMessage->size; i++) {
                    printf("%c", buffer[i+4]);
                    printf("%c", data[i]);
                }
                sequence = receivedMessage->sequence;

                switch (receivedMessage->type) {
                    case 0:
                        file = openFile(receivedMessage->data);
                        sendAck(socket, sentMessage, receivedMessage);
                        break;
                    case 1:
                        sendAck(socket, sentMessage, receivedMessage);
                        break;
                    case 2:
                        sendAck(socket, sentMessage, receivedMessage);
                        sendFile(socket, sentMessage, receivedMessage, (char*) receivedMessage->data, &sequence);
                        break;
                    case 4:
                        changeDirectory((char*) receivedMessage->data);
                        sendAck(socket, sentMessage, receivedMessage);
                        break;
                    case 5:
                        #ifdef LOOPBACK
                        printf("Origin before: %d\n", sentMessage->origin);
                        #endif
                        getMD5Hash((char*) receivedMessage->data, serverMD5);
                        #ifdef LOOPBACK
                        printf("Origin after: %d\n", sentMessage->origin);
                        #endif
                        makePacket(sentMessage, serverMD5, MD5_DIGEST_LENGTH, receivedMessage->sequence, 7);
                        send(socket, sentMessage, MESSAGE_SIZE, 0);
                        printf("Sent message:\n");
                        printPacket(sentMessage);
                        break;
                    case 8:
                        for (int i = 0; i < receivedMessage->size; i++) {
                            // printf("%c", data[i]);
                        }
                        saveFile(file, receivedMessage->data, receivedMessage->size);
                        sendAck(socket, sentMessage, receivedMessage);
                        break;
                    case 9:
                        if (file) {
                            fclose(file);
                            file = NULL;
                        }
                        sendAck(socket, sentMessage, receivedMessage);
                        break;
                    case 10:
                        sendAck(socket, sentMessage, receivedMessage);
                        break;
                }
            }
        }
    }
    free(buffer);
    free(data);
    free(sentMessage);
    free(receivedMessage);
    free(serverMD5);

    return 0;
}