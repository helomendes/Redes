#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <net/if.h>
#include <string.h>

#include "packet.h"
#include "socket.h"

#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define FRAME_LEN 128
#define DATA_SIZE 63

int get_index( char *interface );
int read_message( char *message );

int main ( int argc, char **argv ) {
    if (argc != 2) {
        printf("Erro: execucao incorreta\n");
        printf("Exemplo: sudo ./client interface_de_rede\n");
        exit(1);
    }

    int send_len, received_len, read_len;
    char buffer[FRAME_LEN];
    char interface[8];

    strncpy(interface, argv[1], 8);
    int ifindex = get_index(interface);
    struct packet_header_t header = create_header();
    header.type = DATA;

    int sockfd = create_raw_socket(interface);

    char data[DATA_SIZE];
    while (1) {
        header.size = read_message(data);
        header.sequence = header.sequence + 1;

        send_len = write_header(header, buffer);
        strcpy(buffer + send_len, data);
        send_len += strlen(data);
        send_len += write_crc(buffer, send_len);

        if (send_len < 14) {
            fprintf(stderr, "Mensagem curta demais para ser enviada (tamanho minimo: %d, tamanho da mensagem: %d)\n", 14, send_len);
            exit(1);
        }

        send_packet(sockfd, buffer, send_len, ifindex);

        while (1) {
            received_len = recvfrom(sockfd, buffer, FRAME_LEN, 0, NULL, NULL);
            if (received_len < 0) {
                perror("erro em recvfrom");
                close(sockfd);
                exit(1);
            }

            if (is_packet(buffer, received_len)) {
                read_len = read_header(&header, buffer);
                if (! valid_crc(buffer, read_len + header.size)) {
                    // erro no crc
                    printf("Erro detectado pelo crc");
                    exit(1);
                } else {
                    //printf("Pacote recebido com sucesso\n");
                    //print_header(header);
                    memcpy(&data, buffer + read_len, header.size);
                    data[header.size] = '\0';
                    printf(ANSI_COLOR_YELLOW "server: %s" ANSI_COLOR_RESET "\n", data);
                    break;
                }
            }
        }
    }

    close(sockfd);
    return 0;
}

int get_index( char *interface )
{
    int index = if_nametoindex(interface);
    if (!index) {
        fprintf(stderr, "Erro, interface desconhecida: %s\n", interface);
        exit(1);
    }

    return index;
}

int read_message( char *message )
{
    printf(ANSI_COLOR_BLUE "client: " ANSI_COLOR_RESET);
    scanf("%[^\n]", message);
    getchar();
    return strlen(message);
}