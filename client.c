#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <net/if.h>
#include <string.h>

#include "packet.h"
#include "socket.h"

#define FRAME_LEN 128
#define DATA_SIZE 63

int main (int argc, char **argv) {
    if (argc != 2) {
        printf("Erro: execucao incorreta\n");
        printf("Exemplo: sudo ./client interface_de_rede\n");
        exit(1);
    }

    int send_len = 0;
    unsigned int bytes_escritos;
    char send_buf[FRAME_LEN];
    char interface[8];
    strncpy(interface, argv[1], 8);
    struct packet_header_t header = cria_header();
    header.type = DADOS;

    int soquete = cria_raw_socket(interface);

    char data[DATA_SIZE];
    printf("Insira uma mensagem para ser enviada: ");
    scanf("%[^\n]", data);
    getchar();
    header.size = strlen(data);

    bytes_escritos = escreve_header(header, send_buf);
    send_len += bytes_escritos;
    strcpy(send_buf + send_len, data);
    send_len += strlen(data);
    send_len += escreve_crc(send_buf, send_len);

    int ifindex = if_nametoindex(interface);

    send_packet(soquete, send_buf, send_len, ifindex);

    close(soquete);
    return 0;
}
