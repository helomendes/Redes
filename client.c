#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <net/if.h>
#include <string.h>

#include "packet.h"
#include "socket.h"

#define FRAME_LEN 128
#define DATA_SIZE 63

int eh_interface(char *path);

int main (int argc, char **argv) {
    if (argc != 2) {
        printf("Erro: execucao incorreta\n");
        printf("Exemplo: sudo ./client interface_de_rede\n");
        exit(1);
    }

    int send_len = 0;
    int bytes_recebidos, bytes_lidos;
    char buffer[FRAME_LEN];
    char interface[8];

    int ifindex = eh_interface(interface);
    strncpy(interface, argv[1], 8);
    struct packet_header_t header = cria_header();
    header.type = DADOS;

    int soquete = cria_raw_socket(interface);

    char data[DATA_SIZE];
    printf("Insira uma mensagem para ser enviada: ");
    scanf("%[^\n]", data);
    getchar();
    header.size = strlen(data);

    send_len += escreve_header(header, buffer);
    strcpy(buffer + send_len, data);
    send_len += strlen(data);
    send_len += escreve_crc(buffer, send_len);

    send_packet(soquete, buffer, send_len, ifindex);

    while (1) {
        bytes_recebidos = recvfrom(soquete, buffer, FRAME_LEN, 0, NULL, NULL);
        if (bytes_recebidos < 0) {
            perror("erro em recvfrom");
            close(soquete);
            exit(1);
        }

        if (bytes_recebidos >= sizeof(struct packet_header_t)) {
            bytes_lidos = le_header(&header, buffer);
            if (bytes_lidos) {
                if (!crc_valido(buffer, bytes_lidos + header.size)) {
                    // erro no crc
                    printf("Erro detectado pelo crc");
                } else {
                    printf("Pacote recebido com sucesso\n");
                    imprime_header(header);
                    memcpy(&data, buffer + bytes_lidos, header.size);
                    data[header.size] = '\0';
                }
            }
        }
    }

    close(soquete);
    return 0;
}

int eh_interface(char *path)
{
    int index = if_nametoindex(path);
    if (!index) {
        fprintf(stderr, "Erro, interface desconhecida");
        exit(1);
    }

    return index;
}

