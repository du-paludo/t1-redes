#ifndef MESSAGE_H
#define MESSAGE_H

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <linux/if.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#define BIT_INICIO 0x7E // 01111110 sequencia para começar a ler a mensagem

#define TAM_BUFFER_DADOS   32

struct mensagem{
    unsigned char ini:8;
    unsigned char tam:5;
    unsigned char sequencia:6;
    unsigned char tipo:5;
    char dados[TAM_BUFFER_DADOS]; //32 bytes
    char lixo[29];  //29 * sizeof(char) = 29 bytes para completar uma mensagem de 64 bytes
};
typedef struct mensagem mensagem_t;

int CriaRawSocket(char); //Cria socket

void CriaMensagem(struct mensagem, char, char*); //Cria a mensagem que será mandada por parametro

#endif