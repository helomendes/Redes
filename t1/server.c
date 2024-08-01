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

void is_dir( char *path );
int get_index( char *interface );
void send_video_list( char *buffer, struct packet_header_t header, char *videos_dir );

int main (int argc, char **argv) {
    if (argc != 3) {
        printf("Erro: execucao incorreta\n");
        printf("Exemplo: sudo ./server interface_de_rede diretorio_de_videos\n");
        exit(1);
    }

    int received_len, read_len, send_len;
    char interface[8], videos_dir[PATH_MAX], garbage[16];

    strncpy(interface, argv[1], 8);
    int ifindex = get_index(interface);
    strncpy(videos_dir, argv[2], PATH_MAX);
    is_dir(videos_dir);

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

        if (received_len >= SIZEOF_SMALLEST_PACKET) {
            read_len = read_header(&header, buffer);
            if (! valid_crc(buffer, read_len + header.size)) {
                // erro no crc
                // mandar um nack
                printf("Erro detectado pelo crc");
            } else {
                //printf("Pacote recebido com sucesso\n");
                //print_header(header);
                memcpy(&data, buffer + read_len, header.size);
                data[header.size] = '\0';
                //printf("%s\n", data);

                if (header.type == LIST) {
                    send_video_list(buffer, header, videos_dir);
                }

                printf("respondendo...\n");
                header.size = sizeof(garbage);
                header.type = ACK;
                header.sequence = 2;
                send_len = write_header(header, buffer);
                memcpy(buffer + send_len, garbage, header.size);
                send_len += header.size;
                send_len += write_crc(buffer, send_len);
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

void is_dir( char *interface ) {
    struct stat dir_stats;
    stat(interface, &dir_stats);
    if (!S_ISDIR(dir_stats.st_mode)) {
        fprintf(stderr, "Erro, caminho fornecido nao eh um diretorio\n");
        exit(1);
    }
}

void send_video_list( char *buffer, struct packet_header_t header, char *videos_dir )
{
    DIR *dp = opendir(videos_dir);
    struct dirent *ep;
    if (! dp) {
        fprintf(stderr, "Falha ao abrir o diretorio %s\n", videos_dir);
        exit(1);
    }
    while ((ep = readdir(dp)) != NULL)
        printf("%s\n", ep->d_name);

    closedir(dp);
}