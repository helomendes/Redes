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

#define BUFFER_SIZE 128
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
    char buffer[BUFFER_SIZE], data[DATA_SIZE];
    char interface[8];

    strncpy(interface, argv[1], 8);
    int ifindex = get_index(interface);
    struct packet_header_t header = create_header();
    header.type = LIST;
    header.size = 10;

    int sockfd = create_raw_socket(interface);

    send_len = write_header(header, buffer);
    send_len += 10;
    send_len += write_crc(buffer, send_len);
    send_packet(sockfd, buffer, send_len, ifindex);
    if (expect_response(sockfd, buffer, BUFFER_SIZE)) {
        printf("Recebeu erro\n");
        exit(1);
    }

    while (1) {
        received_len = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, NULL, NULL);
        if (received_len < 0) {
            perror("erro em recvfrom");
            close(sockfd);
            exit(1);
        }

        if (is_packet(buffer, received_len)) {
            read_len = read_header(&header, buffer);
            if (! valid_crc(buffer, read_len + header.size)) {
                // erro no crc, manda um nack
                fprintf(stderr, "Erro detectado pelo crc\n");
                exit(1);
            } else {
                if (header.type == END) break;

                if (header.type == SHOW) {
                    strncpy(data, buffer + read_len, header.size);
                    data[header.size] = '\0';
                    printf("%s\n", data);
                    send_command(sockfd, buffer, ifindex, ACK);
                }
            }
        }
    }

    printf("Insira o nome do video que deseja transmitir: ");
    scanf("%62[^\n]", data);
    getchar();

    header.size = strlen(data);
    header.sequence = 1;
    header.type = DOWNLOAD;

    send_len = write_header(header, buffer);
    strncpy(buffer + send_len, data, header.size);
    send_len += header.size;
    send_len += write_crc(buffer, send_len);
    send_packet(sockfd, buffer, send_len, ifindex);
    if (expect_response(sockfd, buffer, BUFFER_SIZE)) {
        printf("Recebeu erro\n");
        exit(1);
    }
    printf("Recebeu ack do download\n");

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