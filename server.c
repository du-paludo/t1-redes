#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ConexaoRawSocket.h"
#include "rawSocketConnection.h"
#include "packet.h"

#define ETHERNET "lo"

int main(int argc, char** argv) {
    char* command = malloc(sizeof(char) * 100);
    char* path = malloc(sizeof(char) * 100);
    unsigned char* buffer = malloc(sizeof(unsigned char)*1024);

    int socket; 
    packet_t packet;

    socket = rawSocketConnection(ETHERNET);

    while (1) {
        if (recv(socket, &packet, sizeof(packet), 0)) {
            buffer = packetToBuffer(&packet);
            printf("teste: %s\n", packet.data);
            printf("%s\n", buffer);
        }
    }
    return 0;
}
