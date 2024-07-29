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

#define DATA_SIZE           63
#define BUFFER_SIZE         128

int cria_raw_socket( char* nome_interface_rede );
void eh_diretorio( char *path );
void eh_interface( char *interface );

int main (int argc, char **argv) {
    if (argc != 3) {
        printf("Erro: execucao incorreta\n");
        printf("Exemplo: sudo ./server interface_de_rede diretorio_de_videos\n");
        exit(1);
    }

    int bytes_recebidos;
    int bytes_lidos;
    char interface[8];
    char videos_dir[PATH_MAX];

    strncpy(interface, argv[1], 8);
    eh_interface(interface);
    strncpy(videos_dir, argv[2], PATH_MAX);
    eh_diretorio(videos_dir);

    int soquete = cria_raw_socket(interface);
    struct packet_header_t header;

    char buffer[BUFFER_SIZE];
    char data[DATA_SIZE];
    while (1) {
        bytes_recebidos = recvfrom(soquete, buffer, BUFFER_SIZE, 0, NULL, NULL);
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
                    printf("%s\n", data);
                }
            }
        }
    }

    close(soquete);
    return 0;
}

void eh_interface(char *path)
{
    if (!if_nametoindex(path)) {
        fprintf(stderr, "Erro, interface desconhecida");
        exit(1);
    }
}

void eh_diretorio(char *interface) {
    struct stat dir_stats;
    stat(interface, &dir_stats);
    if (!S_ISDIR(dir_stats.st_mode)) {
        fprintf(stderr, "Erro, caminho fornecido nao eh um diretorio\n");
        exit(1);
    }
}