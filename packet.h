#ifndef PACKET_H
#define PACKET_H

#define START_DELIMITER '~'
#define MESSAGE_SIZE 67
#define DATA_SIZE 63

// types:
// 0000 - backup 1 arq
// 0001 - backup +1 arq
// 0010 - recupera 1 arq
// 0011 - recupera +1 arq
// 0100 - escolher diretório do servidor
// 0101 - verificar backup
// 0110 - nome de arquivo para recuperar
// 0111 - MD5 
// 1000 - dados
// 1001 - fim arquivo
// 1010 - fim grupo arq
// 1011 
// 1100 - erro
// 1101 - ok
// 1110 - ack
// 1111 - nack

struct packet {
    unsigned char startDelimiter:8;  // 011111110
    unsigned char size:6;
    unsigned char sequence:6;
    unsigned int type:4;
    unsigned char data[DATA_SIZE];
    unsigned char crc:8;
};
typedef struct packet packet_t;

packet_t* makePacket(unsigned char *data, int size, int sequence, int type);

unsigned char* packetToBuffer(packet_t *p);

#endif