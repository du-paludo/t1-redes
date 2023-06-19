#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "rawSocketConnection.h"
#include "fileHelper.h"
#include "packet.h"

#define ETHERNET "lo"

int main(int argc, char** argv) {
    // char* command = malloc(sizeof(char) * 100);
    // char* path = malloc(sizeof(char) * 100);
    unsigned char* data = malloc(8*63);
    FILE* file;

    int socket; 
    packet_t packet;
    int type;
    unsigned char startDelimiter;

    socket = rawSocketConnection(ETHERNET);

    while (1) {
        if (recv(socket, &packet, 67, 0)) {
            data = packetToBuffer(&packet);
            startDelimiter = packet.startDelimiter;
            type = packet.type;
            if (startDelimiter == '~') {
                printf("%d\n", type);
            }

            switch (type) {
                case 0:
                    file = openFile();
                    break;
                case 8:
                    printf("%s \n", data);
                    saveFile(file, data);
                    break;
                case 9:
                    if (file) {
                        fclose(file);
                        file = NULL;
                    }
            }
        }
    }

    return 0;
}