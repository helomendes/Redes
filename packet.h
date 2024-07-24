#ifndef PACKET_H_
#define PACKET_H_

struct packet_t {
    char start[10];
    int size;
    char* message; // como fazer com o tamanho da mensagem sendo variavel??
    // tipo, o pacote vai ser recebido, e dai n bytes do buffer tem que ser
    // "colados" no espaco de uma estrutura packet_t, mas como fazer isso
    // com um tamanho de mensagem variado? todos os packet_t NA MAQUINA vao
    // ter tamanho maximo? nao funciona porque o crc vai estar no lugar errado
    // a menos que cole bytes - tamanho do crc e dps cole o crc na posicao
    // correta
};

#endif