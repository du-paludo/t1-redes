#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ConexaoRawSocket.h"
#include "rawSocketConnection.h"
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

    socket = rawSocketConnection(ETHERNET);

    while (1) {
        if (recv(socket, &packet, 67, 0)) {
            data = packetToBuffer(&packet);
            type = packet.type;
            printf("%d\n", type);
            switch (type) {
                case 0:
                    file = openFile(data);
                    break;
                case 8:
                    fprintf(file, data);
                    break;
            }
        }
    }
    return 0;
}
// https://github.com/ludersGabriel/t1-redes
// https://github.com/tikaradate/redes-1
// https://github.com/JoaoPicolo/UFPR-Redes1
