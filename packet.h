#ifndef PACKET_H
#define PACKET_H

#define START_DELIMITER '~'

#ifdef LOOPBACK
#define MESSAGE_SIZE 68
#define HEADER_SIZE 5
#else
#define MESSAGE_SIZE 67
#define HEADER_SIZE 4
#endif

#define DATA_SIZE 63
#define MAX_SEQUENCE 64
#define BUFFER_SIZE 1024


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
    // unsigned int origin:8;
    unsigned int size:6;
    unsigned int sequence:6;
    unsigned int type:4;
    unsigned char data[DATA_SIZE];
    unsigned char vrc:8;
};
typedef struct packet packet_t;

void makePacket(packet_t* packet, unsigned char *data, int size, int sequence, int type);

void sendResponse(int socket, packet_t* sentMessage, packet_t* receivedMessage, int type);

int waitResponseTimeout(int socket, packet_t* sentMessage, packet_t* receivedMessage);

void bufferToPacket(packet_t* packet, unsigned char* buffer);

unsigned char* packetToBuffer(packet_t* packet);

unsigned char calculateVRC(packet_t* packet);

void printPacket(packet_t* packet);

void sendMessage(int socket, packet_t* sentMessage, packet_t* receivedMessage);

int checkIntegrity(int socket, packet_t* sentMessage, packet_t* receivedMessage, int* sequence, int id);

void receiveMessage(int socket, packet_t* sentMessage, packet_t* receivedMessage, int* sequence, int id);

#endif