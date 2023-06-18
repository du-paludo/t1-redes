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

    int socket, i;
    char* data = "Abajur";
    packet_t* packet = makePacket(data, strlen(data) + 1, 1, 1);
    unsigned char *buffer = malloc(sizeof(unsigned char)*1024);

    socket = rawSocketConnection(ETHERNET);
    printf("%s\n", packet->data);

    buffer = packetToBuffer(packet);

    while (1) {
        send(socket, buffer, sizeof(packet), 0);
        
        // scanf("%s", command);
        // if (strcmp(command, "cd") == 0) {
        //     scanf("%s", path);
        //     chdir(path);
        // }
    }

    /* if (argc > 1) {
        command = argv[1];
        if (strcmp(command, "cd") == 0) {
            if (argc > 2) {
                path = argv[2];
                chdir(path);
            }
        }
    } */

    /* int sock_r;
    sock_r = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sock_r < 0) {
        print("error in socket \n");
        return -1;
    }

    unsigned char *buffer = (unsigned char *) malloc(65536); //to receive data
    memset(buffer,0,65536);
    struct sockaddr saddr;
    int saddr_len = sizeof (saddr);
    
    // Receive a network packet and copy in to buffer
    buflen = recvfrom(sock_r,buffer,65536,0,&saddr,(socklen_t *)&saddr_len);
    if (buflen < 0) {
        printf("error in reading recvfrom function\n");
        return -1;
    }

    return 0; */
}
