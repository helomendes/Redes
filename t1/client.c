#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <net/if.h>
#include <string.h>

#include "packet.h"
#include "socket.h"

#define ANSI_COLOR_YELLOW   "\x1b[33m"
#define ANSI_COLOR_GREEN    "\x1b[32m"
#define ANSI_COLOR_BLUE     "\x1b[34m"
#define ANSI_COLOR_RESET    "\x1b[0m"

#define BUFFER_SIZE 128
#define DATA_SIZE   63

#define TEMP_VIDEO_PATH     "/home/memr22/Videos/Client/video_saida.mp4"

int get_index( char *interface );
int read_message( char *message );

void send_list( int sockfd, char *buffer, int buffer_size, int ifindex );
void expect_show( int sockfd, char *data, char *buffer, int data_size, int buffer_size, int ifindex );
void send_filename( int sockfd, char *data, char *buffer, int buffer_size, int ifindex );
void expect_descriptor( int sockfd, char *buffer, int buffer_size, int ifindex );
void expect_download( int sockfd, char *video_path, char *data, char *buffer, int data_size, int buffer_size, int ifindex );

int main ( int argc, char **argv ) {
    if (argc != 2) {
        printf("Erro: execucao incorreta\n");
        printf("Exemplo: sudo ./client interface_de_rede\n");
        exit(1);
    }

    char buffer[BUFFER_SIZE], data[DATA_SIZE];
    char interface[8];

    strncpy(interface, argv[1], 8);
    int ifindex = get_index(interface);

    int sockfd = create_raw_socket(interface);

    send_list(sockfd, buffer, BUFFER_SIZE, ifindex);
    printf("Enviou pedido de lista e recebeu ack\n");
    expect_show(sockfd, data, buffer, DATA_SIZE, BUFFER_SIZE, ifindex);
    send_filename(sockfd, data, buffer, BUFFER_SIZE, ifindex);
    expect_descriptor(sockfd, buffer, BUFFER_SIZE, ifindex);
    expect_download(sockfd, TEMP_VIDEO_PATH, data, buffer, DATA_SIZE, BUFFER_SIZE, ifindex);

    // receber descritor de arquivo
    // responder
    // receber bytes e ir escrevendo no arquivo

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

void send_list( int sockfd, char *buffer, int buffer_size, int ifindex ) {
    int timeout_ms, response;
    short tries = 3;
    for (short try = 1; try < tries; try++) {
        timeout_ms = 500;
        send_command(sockfd, buffer, ifindex, LIST);
        response = expect_response(sockfd, buffer, BUFFER_SIZE, timeout_ms);
        while((response == TIMEOUT) && (timeout_ms < 4000)) {
            timeout_ms = timeout_ms << 1;
            response = expect_response(sockfd, buffer, BUFFER_SIZE, timeout_ms);
        }

        if (response == RECEIVED_ACK) return;

        if (response == TIMEOUT) {
            printf("Timeout no pedido de lista para o servidor\n");
            exit(1);
        }
    }

}

void expect_show( int sockfd, char *data, char *buffer, int data_size, int buffer_size, int ifindex)
{
    struct packet_header_t header;
    int received_len, read_len;
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
                send_command(sockfd, buffer, ifindex, NACK);
                printf("Erro detectado pelo crc\n");
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
}

void send_filename( int sockfd, char *data, char *buffer, int buffer_size, int ifindex )
{
    struct packet_header_t header = create_header();
    printf("Insira o nome do video que deseja transmitir: ");
    scanf("%[^\n]", data);
    getchar();

    int filename_size = strlen(data);
    header.sequence = 1;
    header.type = DOWNLOAD;

    //int send_len = write_header(header, buffer);
    if (filename_size < 10) header.size = 10;
    else header.size = filename_size;

    int send_len = write_header(header, buffer);
    if (header.size != filename_size)
        buffer[send_len + filename_size] = '\0';
    strncpy(buffer + send_len, data, header.size);
    send_len += header.size;
    send_len += write_crc(buffer, send_len);

    int timeout_ms = 500;
    send_packet(sockfd, buffer, send_len, ifindex);
    if (expect_response(sockfd, buffer, buffer_size, timeout_ms)) {
        fprintf(stderr, "Recebeu erro\n");
        exit(1);
    }
}

void expect_descriptor( int sockfd, char *buffer, int buffer_size, int ifindex )
{
    struct packet_header_t header;
    int received_len, read_len;
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
                if (header.type == DESCRIPTOR) {
                    send_command(sockfd, buffer, ifindex, ACK);
                    return;
                } else {
                    // nack, erro ou outra coisa (que vai ser um erro)
                    // pode ser erro de file not found, encerrar o programa ou pedir o nome do arquivo de novo?
                    fprintf(stderr, "Erro ao receber descritor de arquivo. Tipo recebido: %d\n", header.type);
                    exit(1);
                }
            }
        }
    }
}

void expect_download( int sockfd, char *video_path, char *data, char *buffer, int data_size, int buffer_size, int ifindex )
{
    FILE *video = fopen(video_path, "w");
    if (! video) {
        fprintf(stderr, "Erro ao abrir arquivo '%s'\n", video_path);
        return;
    }

    struct packet_header_t header;
    int received_len, read_len;
    short last_sequence = 0;
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
                printf("Erro detectado pelo crc na recepcao de um pacote, mandando NACK\n");
                send_command(sockfd, buffer, ifindex, NACK);
            } else {
                if (header.type == END) {
                    send_command(sockfd, buffer, ifindex, ACK);
                    printf("Video recebido com sucesso, %s\n", video_path);
                    fclose(video);
                    break;
                }

                if (header.type == ERROR) {
                    send_command(sockfd, buffer, ifindex, ACK);
                    printf("Erro na transmissao, interrompendo download\n");
                    fclose(video);
                    close(sockfd);
                    exit(1);
                }

                if (header.type == DATA) {
                    if ((header.sequence  > last_sequence) || ((header.sequence == 0) && (header.sequence - 1 == last_sequence))) {
                        fwrite(buffer + read_len, header.size, 1, video);
                        last_sequence = header.sequence;
                    }
                    send_command(sockfd, buffer, ifindex, ACK);
                } else {
                    printf("Pacote de tipo inesperado recebido: %d\n", header.type);
                }
            }
        }
    }
}