#ifndef PACKET_H_
#define PACKET_H_

#include <stdint.h>

#define INIT_MARKER 0b01111110
#define ACK         0b00000
#define NACK        0b00001
#define LISTA       0b01010
#define BAIXAR      0b01011
#define MOSTRAR     0b10000
#define DESCRITOR   0b10001
#define DADOS       0b10010
#define FIM         0b11110
#define ERRO        0b11111

typedef enum {
    ACESSO_NEGADO = 1,
    NAO_ENCONTRADO,
    DISCO_CHEIO
} ERROS;

#define SIZEOF_INITMARKER 1
#define MAX_SIZE_VALUE (unsigned char) 63
#define MAX_SEQUENCE_VALUE (unsigned char) 31
#define MAX_TYPE_VALUE (unsigned char) 31

struct packet_header_t {
    unsigned char init_marker;
    unsigned char size:6;
    unsigned char sequence:5;
    unsigned char type:5;
};

struct packet_header_t cria_header();

int eh_header(struct packet_header_t p);

void imprime_header(struct packet_header_t p);

unsigned int escreve_header(struct packet_header_t p, char* buffer);

int le_header(struct packet_header_t *p, char* buffer);

unsigned int escreve_crc(char *buffer, int bytes);

int crc_valido(char *buffer, int bytes);

#endif