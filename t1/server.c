#include <stdio.h>
#include <stdlib.h>
#include <net/if.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>

#include "packet.h"
#include "socket.h"
#include "video.h"

#define BUFFER_SIZE     128
#define DATA_SIZE       63
#define INTERFACE_SIZE  12

void execute_list( int sockfd, char *videos_dir, char *data, char *buffer, int data_size, int buffer_size, int ifindex );

int get_index( char *interface );
int expect_filename( int sockfd, char *buffer, char *data, int buffer_size, int ifindex );
int send_video_list( int sockfd, char *buffer, char *videos_dir, int buffer_size, int ifindex );
int send_descriptor( int sockfd, char *video_path, char *buffer, int buffer_size, int ifindex );
int send_video( int sockfd, char *video_path, char *data, char *buffer, int data_size, int buffer_size, int ifindex );

int main ( int argc, char **argv ) {
    if (argc != 3) {
        fprintf(stderr, "Erro: execucao incorreta\n");
        fprintf(stderr, "Exemplo: sudo ./server interface_de_rede diretorio_de_videos\n");
        exit(1);
    }

    char interface[INTERFACE_SIZE], videos_dir[PATH_MAX];

    strncpy(interface, argv[1], INTERFACE_SIZE);
    int ifindex = get_index(interface);
    strncpy(videos_dir, argv[2], PATH_MAX);

    is_dir(videos_dir);
    preprocess_video_path(videos_dir);
    printf("Diretorio dos videos: %s\n", videos_dir);

    int sockfd = create_raw_socket(interface);
    struct packet_header_t header;

    int received_len, read_len;
    char buffer[BUFFER_SIZE], data[DATA_SIZE];
    while (1) {
        received_len = receive_packet(sockfd, buffer, BUFFER_SIZE);
        if (received_len < 0) {
            fprintf(stderr, "Erro em recvfrom\n");
            close(sockfd);
            exit(2);
        }

        if (is_packet(buffer, received_len)) {
            read_len = read_header(&header, buffer);
            if (! valid_crc(buffer, read_len + header.size)) {
                printf("Erro detectado pelo crc\n");
                send_command(sockfd, buffer, ifindex, NACK);
            } else {
                if (header.type == LIST) {
                    printf("Conexao estabelecida...\n");
                    send_command(sockfd, buffer, ifindex, ACK);
                    execute_list(sockfd, videos_dir, data, buffer, DATA_SIZE, BUFFER_SIZE, ifindex);
                } else {
                    printf("Comunicacao inesperada recebida, ignorando...\n");
                }
            }
        }
    }

    close(sockfd);
    return 0;
}

void execute_list( int sockfd, char *videos_dir, char *data, char *buffer, int data_size, int buffer_size, int ifindex )
{
    printf("Enviando lista de videos...\n");
    if (send_video_list(sockfd, buffer, videos_dir, buffer_size, ifindex)) {
        printf("Erro ao enviar lista de nomes de arquivo, interrompendo transferencia\n");
        return;
    }

    if (expect_filename(sockfd, buffer, data, buffer_size, ifindex)) {
        printf("Erro ao receber nome de arquivo, interrompendo transferencia\n");
        return;
    }

    printf("Nome de arquivo recebido: %s\n", data);

    char video_path[PATH_MAX];
    create_video_path(videos_dir, data, video_path);
    if (send_descriptor(sockfd, video_path, buffer, buffer_size, ifindex)) {
        printf("Erro ao enviar descritor de arquivo, interrompendo transferencia\n");
        return;
    }

    printf("Enviando video...\n");
    if (send_video(sockfd, video_path, data, buffer, data_size, buffer_size, ifindex)) {
        printf("Erro ao enviar video, interrompendo transferencia\n");
        return;
    }

    printf("Video enviado com sucesso, encerrando conexao...\n");
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

int send_video_list( int sockfd, char *buffer, char *videos_dir, int buffer_size, int ifindex )
{
    DIR *dp = opendir(videos_dir);
    struct dirent *ep;
    int filename_size;
    if (! dp) {
        fprintf(stderr, "Falha ao abrir o diretorio '%s'\n", videos_dir);
        exit(5);
    }

    struct packet_header_t header = create_header();
    header.type = SHOW;
    header.sequence = 0;

    int send_len, timeout_ms, response;
    short tries = 3;
    char receive_buffer[buffer_size];
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

            timeout_ms = 500;
            send_packet(sockfd, buffer, send_len, ifindex);
            response = expect_response(sockfd, receive_buffer, buffer_size, timeout_ms);
            for (short try = 1; ((try <= tries) && (response != RECEIVED_ACK)); try++) {
                while ((response == TIMEOUT) && (timeout_ms < 4000)) {
                    timeout_ms = timeout_ms << 1;
                    send_packet(sockfd, buffer, send_len, ifindex);
                    response = expect_response(sockfd, receive_buffer, buffer_size, timeout_ms);
                }

                if (response == TIMEOUT) {
                    printf("Timeout ao mandar lista de videos\n");
                    send_error(sockfd, buffer, ifindex, PACKET_TIMEOUT);
                    closedir(dp);
                    return 1;
                }

                if (response != RECEIVED_ACK) {
                    timeout_ms = 500;
                    send_packet(sockfd, buffer, send_len, ifindex);
                    response = expect_response(sockfd, receive_buffer, buffer_size, timeout_ms);
                }
            }
        }
    }

    send_command(sockfd, buffer, ifindex, END);

    closedir(dp);
    return 0;
}

int expect_filename( int sockfd, char *buffer, char *data, int buffer_size, int ifindex )
{
    int received_len, read_len;
    struct packet_header_t header;
    uint64_t last_packet = timestamp();
    unsigned short timeout_ms = 2000;

    do {
        received_len = receive_packet(sockfd, buffer, buffer_size);
        if (received_len < 0) {
            fprintf(stderr, "Erro em recvfrom\n");
            close(sockfd);
            exit(2);
        }

        if (is_packet(buffer, received_len)) {
            last_packet = timestamp();
            read_len = read_header(&header, buffer);
            if (! valid_crc(buffer, read_len + header.size)) {
                send_command(sockfd, buffer, ifindex, NACK);
            } else {
                if (header.type == DOWNLOAD) {
                    strncpy(data, buffer + read_len, header.size);
                    data[header.size] = '\0';
                    send_command(sockfd, buffer, ifindex, ACK);
                    return 0;
                } else {
                    return 1;
                }
            }
        }
    } while (timestamp() - last_packet < timeout_ms);

    return 1;
}

int send_descriptor( int sockfd, char *video_path, char *buffer, int buffer_size, int ifindex )
{
    struct packet_header_t header = create_header();
    header.type = DESCRIPTOR;

    uint8_t day, month;
    uint16_t year;
    uint32_t file_size;
    struct stat s;
    if (stat(video_path, &s)) {
        fprintf(stderr, "Erro ao acessar arquivo de video: %s\n", video_path);
        return 1;
    }

    file_size = s.st_size;
    struct tm *timeinfo = localtime(&s.st_mtime);
    day = timeinfo->tm_mday;
    month = timeinfo->tm_mon+1;
    year = timeinfo->tm_year+1900;
    header.size = 10;

    int send_len = write_header(header, buffer);

    header.size = 0;
    memcpy(buffer + send_len, &file_size, sizeof(uint32_t));
    header.size += sizeof(uint32_t);
    memcpy(buffer + send_len + header.size, &day, sizeof(uint8_t));
    header.size += sizeof(uint8_t);
    memcpy(buffer + send_len + header.size, &month, sizeof(uint8_t));
    header.size += sizeof(uint8_t);
    memcpy(buffer + send_len + header.size, &year, sizeof(uint16_t));

    header.size = 10;
    send_len += header.size;
    send_len += write_crc(buffer, send_len);

    int response;
    char receive_buffer[buffer_size];
    for (short try = 1; try <= 3; try++) {
        send_packet(sockfd, buffer, send_len, ifindex);
        response = expect_response(sockfd, receive_buffer, buffer_size, 2000);

        if (response == RECEIVED_ACK) {
            return 0;
        } else if (response == TIMEOUT) {
            fprintf(stderr, "Timeout ao enviar descritor do arquivo\n");
            close(sockfd);
            exit(3);
        } else if (response == RECEIVED_ERROR) {
            fprintf(stderr, "Recebeu erro ao enviar descritor do arquivo\n");
            close(sockfd);
            exit(4);
        }
    }

    return 1;
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
    uint32_t transfered_bytes = 0;
    char receive_buffer[buffer_size];
    while (! feof(video)) {
        header.sequence++;
        header.size = read_bytes;
        send_len = write_header(header, buffer);
        memcpy(buffer + send_len, data, header.size);
        send_len += header.size;
        send_len += write_crc(buffer, send_len);

        tries = 3;
        timeout_ms = 500;
        send_packet(sockfd, buffer, send_len, ifindex);
        response = expect_response(sockfd, receive_buffer, buffer_size, timeout_ms);
        for (short try = 1; ((try <= tries) && (response != RECEIVED_ACK)); try++) {
            while ((response == TIMEOUT) && (timeout_ms < 2000)) {
                timeout_ms = timeout_ms << 1;
                send_packet(sockfd, buffer, send_len, ifindex);
                response = expect_response(sockfd, receive_buffer, buffer_size, timeout_ms);
            }

            if (response == RECEIVED_ACK) {
                break;
            } else if (response == TIMEOUT) {
                printf("Timeout limite atingido\n");
                fclose(video);
                return 1;
            } else if (response == RECEIVED_NACK) {
                printf("Recebeu um nack na tentativa %d de %d\n", try, tries);
            } else if (response == INVALID_CRC) {
                printf("CRC invalido na resposta do client na tentativa %d de %d\n", try, tries);
            } else if (response == RECEIVED_ERROR) {
                printf("Recebeu um erro\n");
                fclose(video);
                return 1;
            } else if  (response == UNEXPECTED_TYPE) {
                printf("Recebeu um pacote de tipo inesperado\n");
                fclose(video);
                return 1;
            }

            timeout_ms = 500;
            send_packet(sockfd, buffer, send_len, ifindex);
            response = expect_response(sockfd, receive_buffer, buffer_size, timeout_ms);
        }

        if (response == RECEIVED_NACK) {
            printf("Limite de NACKs recebido pelo servidor, encerrando transmissao\n");
            break;
        }

        transfered_bytes += read_bytes;
        read_bytes = fread(data, 1, data_size, video);
    }

    if (read_bytes > 0) {
        header.sequence++;

        if (read_bytes < 10) header.size = 10;
        else header.size = read_bytes;

        send_len = write_header(header, buffer);
        memcpy(buffer + send_len, data, read_bytes);
        send_len += header.size;
        send_len += write_crc(buffer, send_len);

        tries = 5;
        timeout_ms = 500;
        send_packet(sockfd, buffer, send_len, ifindex);
        response = expect_response(sockfd, receive_buffer, buffer_size, timeout_ms);
        for (short try = 1; ((try <= tries) && (response != RECEIVED_ACK)); try++) {
            while ((response == TIMEOUT) && (timeout_ms < 4000)) {
                timeout_ms = timeout_ms << 1;
                send_packet(sockfd, buffer, send_len, ifindex);
                response = expect_response(sockfd, receive_buffer, buffer_size, timeout_ms);
            }

            if (response == RECEIVED_ACK) {
                break;
            } else if (response == TIMEOUT) {
                printf("Timeout limite atingido\n");
                fclose(video);
                return 1;
            } else if (response == RECEIVED_NACK) {
                printf("Recebeu um nack na tentativa %d de %d\n", try, tries);
            } else if (response == INVALID_CRC) {
                printf("CRC invalido na resposta do client na tentativa %d de %d\n", try, tries);
            } else if (response == RECEIVED_ERROR) {
                printf("Recebeu um erro\n");
                fclose(video);
                return 1;
            } else if  (response == UNEXPECTED_TYPE) {
                printf("Recebeu um pacote de tipo inesperado\n");
                fclose(video);
                return 1;
            }

            timeout_ms = 500;
            send_packet(sockfd, buffer, send_len, ifindex);
            response = expect_response(sockfd, receive_buffer, buffer_size, timeout_ms);
        }

        if (response == RECEIVED_NACK) {
            printf("Limite de NACKs recebido pelo servidor, encerrando transmissao\n");
        } else if (response == RECEIVED_ACK) {
            transfered_bytes += read_bytes;
        }
    }

    fclose(video);

    tries = 3;
    timeout_ms = 500;
    if (response != RECEIVED_ACK) {
        send_error(sockfd, buffer, ifindex, ATTEMPT_LIMIT);
        response = expect_response(sockfd, receive_buffer, buffer_size, timeout_ms);
        for (short try = 1; ((try <= tries) && (response != RECEIVED_ACK)); try++) {
            while ((response == TIMEOUT) && (timeout_ms < 4000)) {
                send_error(sockfd, buffer, ifindex, ATTEMPT_LIMIT);
                timeout_ms = timeout_ms << 1;
                response = expect_response(sockfd, receive_buffer, buffer_size, timeout_ms);
            }

            if (response == TIMEOUT) {
                printf("Timeout ao entregar o erro na transmissao\n");
                break;
            }

            timeout_ms = 500;
        }

        printf("Bytes transferidos: %u\n", transfered_bytes);
        return 1;

    } else {
        send_command(sockfd, buffer, ifindex, END);
        response = expect_response(sockfd, receive_buffer, buffer_size, timeout_ms);
        for (short try = 1; ((try <= tries) && (response != RECEIVED_ACK)); try++) {
            while ((response == TIMEOUT) && (timeout_ms < 4000)) {
                send_command(sockfd, buffer, ifindex, END);
                timeout_ms = timeout_ms << 1;
                response = expect_response(sockfd, receive_buffer, buffer_size, timeout_ms);
            }

            if (response == TIMEOUT) {
                printf("Timeout ao entregar o end\n");
                break;
            }

            timeout_ms = 500;
        }
    }

    printf("Bytes transferidos: %u\n", transfered_bytes);
    return 0;
}
