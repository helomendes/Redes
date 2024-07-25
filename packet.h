#ifndef PACKET_H_
#define PACKET_H_

#define INIT_MARKER (unsigned char) 126
#define PACKET_HEADER_SIZE (unsigned short) 3

typedef union {
    // como fazer com o tamanho da mensagem sendo variavel??
    // tipo, o header vai ser recebido, e dai n bytes do buffer tem que ser
    // "colados" no espaco de uma estrutura packet_t, mas como fazer isso
    // com um tamanho de mensagem variado? todos os packet_t NA MAQUINA vao
    // ter tamanho maximo? nao funciona porque o crc vai estar no lugar errado
    // a menos que cole bytes - tamanho do crc e dps cole o crc na posicao
    // correta
    // ideia: fazer um header somente pro comeco, ja que essa parte eh imutavel
    struct {
        unsigned char init_marker;
        unsigned short size:6;
        unsigned short sequence:5;
        unsigned short type:5;
    };
    unsigned char header[3];
} packet_header_t;

packet_header_t cria_header();

int eh_header(packet_header_t p);

void imprime_header(packet_header_t p);

#endif