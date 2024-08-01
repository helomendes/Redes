#include <stdio.h>
#include <stdlib.h>
#include <net/if.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>

#include "packet.h"
#include "socket.h"

#define DATA_SIZE       63
#define BUFFER_SIZE     128

int get_index( char *interface );
int read_message( char *message );

int main (int argc, char **argv) {
    if (argc != 2) {
        printf("Erro: execucao incorreta\n");
        printf("Exemplo: sudo ./server interface_de_rede\n");
        exit(1);
    }

    int received_len, read_len, send_len;
    char interface[8], garbage[16];

    strncpy(interface, argv[1], 8);
    int ifindex = get_index(interface);

    int sockfd = create_raw_socket(interface);
    struct packet_header_t header;

    char buffer[BUFFER_SIZE];
    char data[DATA_SIZE];
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
                // erro no crc
                printf("Erro detectado pelo crc");
            } else {
                printf("Pacote recebido com sucesso\n");
                print_header(header);
                memcpy(&data, buffer + read_len, header.size);
                data[header.size] = '\0';
                printf("%s\n", data);

                header.size = read_message(data);
                header.type = DATA;
                header.sequence = header.sequence+1;
                send_len = write_header(header, buffer);
                memcpy(buffer + send_len, garbage, header.size);
                send_len += header.size;
                send_len += write_crc(buffer, send_len);

                if (send_len < 14) {
                    fprintf(stderr, "Mensagem curta demais para ser enviada (tamanho minimo: %d, tamanho da mensagem: %d)\n", 14, send_len);
                    exit(1);
                }

                send_packet(sockfd, buffer, send_len, ifindex);
            }
        }
    }

    close(sockfd);
    return 0;
}

int get_index( char *interface )
{
    int index = if_nametoindex(interface);
    if (! index) {
        fprintf(stderr, "Erro, interface desconhecida");
        exit(1);
    }

    return index;
}

int read_message( char *message )
{
    printf("Insira uma mensagem para ser enviada: ");
    scanf("%[^\n]", message);
    getchar();
    return strlen(message);
}
