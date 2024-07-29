#ifndef PACKET_H_
#define PACKET_H_

#include <stdint.h>

#define INIT_MARKER (unsigned char) 126
#define SIZEOF_INITMARKER 1

#define MAX_SIZE_VALUE (unsigned char) 63
#define MAX_SEQUENCE_VALUE (unsigned char) 31
#define MAX_TYPE_VALUE (unsigned char) 31

    // como fazer com o tamanho da mensagem sendo variavel??
    // tipo, o header vai ser recebido, e dai n bytes do buffer tem que ser
    // "colados" no espaco de uma estrutura packet_t, mas como fazer isso
    // com um tamanho de mensagem variado? todos os packet_t NA MAQUINA vao
    // ter tamanho maximo? nao funciona porque o crc vai estar no lugar errado
    // a menos que cole bytes - tamanho do crc e dps cole o crc na posicao
    // correta
    // ideia: fazer um header somente pro comeco, ja que essa parte eh imutavel
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

#endif