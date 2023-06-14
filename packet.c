#include "packet.h"

typedef struct {
    unsigned char startDelimiter:8;  // 011111110
    unsigned char size:6;
    unsigned char sequence:6;
    unsigned char type:4;
    unsigned char data[MESSAGE_SIZE];
    unsigned char crc:8;
} Pacote;

void makePacket(Pacote *p, unsigned char *dados, int tamanho, int sequencia, int tipo) {
    p->marcadorDeInicio = 0x7E;
    p->tamanho = tamanho;
    p->sequencia = sequencia;
    p->tipo = tipo;
    memcpy(p->dados, dados, tamanho);
    p->crc = 0x00;
}