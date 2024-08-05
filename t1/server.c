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

#define BUFFER_SIZE     128
#define DATA_SIZE       63

void is_dir( char *dir );
void preprocess_video_path( char *videos_dir );
void send_video_list( int sockfd, char *buffer, char *videos_dir, int ifindex );
void create_video_path( char *videos_dir, char *video_basename, char *video_path );

int get_index( char *interface );
int expect_filename( int sockfd, char *buffer, char *data, int buffer_size, int ifindex );
int send_descriptor( int sockfd, char *video_path, char *buffer, int buffer_size, int ifindex );
int send_video( int sockfd, char *video_path, char *data, char *buffer, int data_size, int buffer_size, int ifindex );

int main ( int argc, char **argv ) {
    if (argc != 3) {
        printf("Erro: execucao incorreta\n");
        printf("Exemplo: sudo ./server interface_de_rede diretorio_de_videos\n");
        exit(1);
    }

    int received_len, read_len;//, send_len;
    char interface[8], videos_dir[PATH_MAX], video_path[PATH_MAX];

    strncpy(interface, argv[1], 8);
    int ifindex = get_index(interface);
    strncpy(videos_dir, argv[2], PATH_MAX);
    is_dir(videos_dir);
    preprocess_video_path(videos_dir);

    int sockfd = create_raw_socket(interface);
    struct packet_header_t header;

    char buffer[BUFFER_SIZE], data[DATA_SIZE];
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
                printf("Erro detectado pelo crc\n");
                send_command(sockfd, buffer, ifindex, NACK);
            } else {

                if (header.type == LIST) {
                    send_command(sockfd, buffer, ifindex, ACK);
                    send_video_list(sockfd, buffer, videos_dir, ifindex);

                    if (expect_filename(sockfd, buffer, data, BUFFER_SIZE, ifindex)) {
                        fprintf(stderr, "Erro ao receber nome de arquivo\n");
                        continue;
                    }
                    printf("Nome de arquivo recebido: %s\n", data);
                    send_command(sockfd, buffer, ifindex, ACK);

                    create_video_path(videos_dir, data, video_path);
                    if (send_descriptor(sockfd, video_path, buffer, BUFFER_SIZE, ifindex)) {
                        fprintf(stderr, "Erro ao enviar descritor de arquivo\n");
                        continue;
                    }
                    printf("Enviando video...\n");
                    if (send_video(sockfd, video_path, data, buffer, DATA_SIZE, BUFFER_SIZE, ifindex)) {
                        printf("Erro ao enviar video, interrompendo transferencia\n");
                        continue;
                    }
                    printf("Video enviado com sucesso\n");
                }
                // else mandar um nack?
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

void is_dir( char *dir )
{
    struct stat dir_stats;
    stat(dir, &dir_stats);
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

void send_video_list( int sockfd, char *buffer, char *videos_dir, int ifindex )
{
    DIR *dp = opendir(videos_dir);
    struct dirent *ep;
    int filename_size;
    if (! dp) {
        fprintf(stderr, "Falha ao abrir o diretorio %s\n", videos_dir);
        exit(1);
    }

    int send_len;
    int timeout_ms = 500;
    struct packet_header_t header = create_header();
    header.type = SHOW;
    header.sequence = 0;
    while ((ep = readdir(dp)) != NULL) {
        filename_size = is_video(ep->d_name);
        if ((filename_size) && (filename_size <= 63)) {
            header.sequence++;

            if (filename_size < 10) header.size = 10;
            else header.size = filename_size;

            send_len = write_header(header, buffer);
            if (header.size != filename_size)
                buffer[send_len + filename_size] = '\0';

            strncpy(buffer + send_len, ep->d_name, filename_size);
            send_len += header.size;
            send_len += write_crc(buffer, send_len);
            send_packet(sockfd, buffer, send_len, ifindex);
            if (expect_response(sockfd, buffer, BUFFER_SIZE, timeout_ms)) {
                fprintf(stderr, "Nao recebeu ack, interrompendo transmissao\n");
                return;
            }
        }
    }

    // TODO: colocar o envio do end em um loop quer usa timeout tambem
    send_command(sockfd, buffer, ifindex, END);

    closedir(dp);
}

int expect_filename( int sockfd, char *buffer, char *data, int buffer_size, int ifindex )
{
    int received_len, read_len;
    struct packet_header_t header;
    while (1) {
        received_len = recvfrom(sockfd, buffer, buffer_size, 0, NULL, NULL);
        if (received_len < 0) {
            perror("erro em recvfrom");
            close(sockfd);
            exit(1);
        }

        if (is_packet(buffer, received_len)) {
            read_len = read_header(&header, buffer);
            if (! valid_crc(buffer, read_len + header.size)) {
                printf("Erro detectado pelo crc\n");
                send_command(sockfd, buffer, ifindex, NACK);
            } else {
                if (header.type == DOWNLOAD) {
                    strncpy(data, buffer + read_len, header.size);
                    data[header.size] = '\0';
                    return 0;
                } else {
                    return 1;
                }
            }
        }
    }

    return 0;
}

void preprocess_video_path( char *videos_dir )
{
    int null_char = 0;
    while (videos_dir[null_char] != '\0') null_char++;
    if (null_char == 0) {
        fprintf(stderr, "Erro ao processar caminho para os videos\n");
        exit(1);
    }
    if (videos_dir[null_char-1] != '/') {
        videos_dir[null_char] = '/';
        videos_dir[null_char+1] = '\0';
    }
}

void create_video_path( char *videos_dir, char *video_basename, char *video_path )
{
    int null_char = 0;
    while (videos_dir[null_char] != '\0') null_char++;
    if (null_char == 0) {
        fprintf(stderr, "Erro ao processar caminho para os videos\n");
        exit(1);
    }
    strncpy(video_path, videos_dir, null_char);
    strcpy(video_path + null_char, video_basename);
}

int send_descriptor( int sockfd, char *video_path, char *buffer, int buffer_size, int ifindex )
{
    struct packet_header_t header = create_header();
    header.type = DESCRIPTOR;

    // abrir o arquivo, coletar tamanho e data, escrever no buffer, enviar e coletar resposta

    //uint8_t day, month;
    //uint16_t year;
    uint32_t file_size;
    struct stat s;
    if (stat(video_path, &s)) {
        fprintf(stderr, "Erro ao acessar arquivo de video\n");
        // joga erro para o client
        return 1;
    }

    file_size = s.st_size;
    header.size = 10;

    int timeout_ms = 500;
    int send_len = write_header(header, buffer);
    memcpy(buffer + send_len, &file_size, sizeof(uint32_t));
    send_len += header.size;
    send_len += write_crc(buffer, send_len);
    send_packet(sockfd, buffer, send_len, ifindex);
    if (expect_response(sockfd, buffer, buffer_size, timeout_ms)) {
        fprintf(stderr, "Recebeu erro esperando ack do descritor\n");
        exit(1);
    }
    return 0;
}

int send_video( int sockfd, char *video_path, char *data, char *buffer, int data_size, int buffer_size, int ifindex )
{
    struct packet_header_t header = create_header();
    header.type = DATA;
    header.sequence = 0;

    FILE *video = fopen(video_path, "r");
    if (! video) {
        fprintf(stderr, "Falha ao abrir video '%s'\n", video_path);
        return 1;
    }

    short tries;
    int send_len, response, timeout_ms;
    int read_bytes = fread(data, 1, data_size, video);
    char receive_buffer[buffer_size];
    while (! feof(video)) {
        header.sequence++;
        header.size = read_bytes;
        send_len = write_header(header, buffer);
        memcpy(buffer + send_len, data, header.size);
        send_len += header.size;
        send_len += write_crc(buffer, send_len);

        tries = 3;
        send_packet(sockfd, buffer, send_len, ifindex);
        //printf("Pacote enviado, aguardando confirmacao de recebimento...\n");
        timeout_ms = 500;
        response = expect_response(sockfd, receive_buffer, buffer_size, timeout_ms);
        for (short try = 1; ((try <= 3) && (response != RECEIVED_ACK)); try++) {
            while ((response == TIMEOUT) && (timeout_ms < 4000)) {
                send_packet(sockfd, buffer, send_len, ifindex);
                timeout_ms = timeout_ms << 1;
                response = expect_response(sockfd, receive_buffer, buffer_size, timeout_ms);
            }

            switch (response) {
                case TIMEOUT:
                    printf("Timeout limite atingido\n");
                    return 1;
                    break;

                case RECEIVED_NACK:
                    printf("Recebeu um nack na tentativa %d de %d, tentando novamente\n", try, tries);
                    break;

                case RECEIVED_ERROR:
                    printf("Recebeu um erro\n");
                    return 1;
                    // interromper transmissao ?
                    break;

                case INVALID_CRC:
                    printf("Pacote chegou corrompido no server\n");
                    // o que fazer aqui? esperar client reenviar? mandar um nack?
                    return 1;
                    break;

                case UNEXPECTED_TYPE:
                    printf("Recebeu um pacote de tipo inesperado\n");
                    return 1;
                    // e o que fazer aqui?
                    break;
            }
        }

        read_bytes = fread(data, 1, data_size, video);
    }

    tries = 3;
    timeout_ms = 500;
    send_command(sockfd, buffer, ifindex, END);
    response = expect_response(sockfd, receive_buffer, buffer_size, timeout_ms);
    for (short try = 1; ((try <= 3) && (response != RECEIVED_ACK)); try++) {
        while ((response == TIMEOUT) && (timeout_ms < 4000)) {
            send_command(sockfd, buffer, ifindex, END);
            timeout_ms = timeout_ms << 1;
            response = expect_response(sockfd, receive_buffer, buffer_size, timeout_ms);
        }

        if (response == TIMEOUT) {
            printf("Timeout ao entregar o end\n");
            break;
        }
    }

    if (response != RECEIVED_ACK) {
        printf("Falhou em entregar o END\n");
        return 1;
    }

    fclose(video);
    return 0;
}
