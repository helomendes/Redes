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
void send_video_list( int sockfd, char *buffer, struct packet_header_t header, char *videos_dir, int ifindex );

int main (int argc, char **argv) {
    if (argc != 3) {
        printf("Erro: execucao incorreta\n");
        printf("Exemplo: sudo ./server interface_de_rede diretorio_de_videos\n");
        exit(1);
    }

    int received_len, read_len, send_len;
    char interface[8], videos_dir[PATH_MAX];

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

        if (is_packet(buffer, received_len)) {
            read_len = read_header(&header, buffer);
            if (! valid_crc(buffer, read_len + header.size)) {
                // erro no crc
                // mandar um nack
                printf("Erro detectado pelo crc\n");
            } else {
                //printf("Pacote recebido com sucesso\n");
                //print_header(header);
                //memcpy(&data, buffer + read_len, header.size);
                //data[header.size] = '\0';
                //printf("%s\n", data);

                if (header.type == LIST) {
                    send_video_list(sockfd, buffer, header, videos_dir, ifindex);
                }

                //printf("respondendo...\n");
                //header.size = sizeof(garbage);
                //header.type = ACK;
                //header.sequence = 2;
                //send_len = write_header(header, buffer);
                //memcpy(buffer + send_len, garbage, header.size);
                //send_len += header.size;
                //send_len += write_crc(buffer, send_len);
                //send_packet(sockfd, buffer, send_len, ifindex);
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

void is_dir( char *interface )
{
    struct stat dir_stats;
    stat(interface, &dir_stats);
    if (!S_ISDIR(dir_stats.st_mode)) {
        fprintf(stderr, "Erro, caminho fornecido nao eh um diretorio\n");
        exit(1);
    }
}

int is_video(char* filename)
{
    int size = 0;

    while(filename[size] != '\0') size++;

    if (size < 5) return 0;

    if ((! strncmp(filename + size - 4, ".avi", 4)) || (! strncmp(filename + size - 4, ".mp4", 4))) {
        return size;
    }
    return 0;
}

void send_video_list( int sockfd, char *buffer, struct packet_header_t header, char *videos_dir, int ifindex )
{
    DIR *dp = opendir(videos_dir);
    struct dirent *ep;
    int filename_size;
    if (! dp) {
        fprintf(stderr, "Falha ao abrir o diretorio %s\n", videos_dir);
        exit(1);
    }

    int send_len;
    header.type = DATA;
    header.sequence = 1;
    while ((ep = readdir(dp)) != NULL) {
        filename_size = is_video(ep->d_name);
        if ((filename_size) && (filename_size <= 63)) {
            header.size = filename_size;
            send_len = write_header(header, buffer);
            strncpy(buffer + send_len, ep->d_name, filename_size);
            send_len += filename_size;
            write_crc(buffer, send_len);
            send_packet(sockfd, buffer, send_len, ifindex);
        }
    }
    header.size = 10;
    header.type = END;
    send_len = write_header(header, buffer);
    send_len += 10;
    write_crc(buffer, send_len);
    send_packet(sockfd, buffer, send_len, ifindex);

    closedir(dp);
}