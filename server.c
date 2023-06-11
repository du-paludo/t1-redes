#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ConexaoRawSocket.h"

#define ETHERNET "lo"

int main(int argc, char** argv) {
    char* command = malloc(sizeof(char) * 100);
    char* path = malloc(sizeof(char) * 100);

    int socket;
    char* buffer = malloc(sizeof(char) * 1024);

    socket = CriaRawSocket(ETHERNET);

    while (1) {
        if (recv(socket, buffer, sizeof(buffer), 0)) {
            printf("%s\n", buffer);
        }
    }
    return 0;
}
