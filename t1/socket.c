#include <stdio.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>

#include "socket.h"
#include "packet.h"

void catch_loopback( int sockfd, char *buffer, int buffer_size );

int create_raw_socket( char* interface )
{
    int sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sockfd == -1) {
        fprintf(stderr, "Erro ao criar socket: Verifique se voce eh root!\n");
        exit(1);
    }

    int ifindex = if_nametoindex(interface);

    struct sockaddr_ll addr = {0};
    addr.sll_family = AF_PACKET;
    addr.sll_protocol = htons(ETH_P_ALL);
    addr.sll_ifindex = ifindex;

    if (bind(sockfd, (struct sockaddr*) &addr, sizeof(addr)) == -1) {
        fprintf(stderr, "Erro ao fazer bind no socket\n");
        exit(1);
    }

    struct packet_mreq mr = {0};
    mr.mr_ifindex = ifindex;
    mr.mr_type = PACKET_MR_PROMISC; // Modo promiscuo

    if (setsockopt(sockfd, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) == -1) {
        fprintf(stderr, "Erro ao fazer setsockopt: verifique se a interface de rede foi especificada corretamente\n");
        exit(1);
    }

    return sockfd;
}

void send_packet(int sockfd, char* buffer, int bytes, int ifindex)
{
    struct sockaddr_ll addr = {0};
    addr.sll_family = AF_PACKET;
    addr.sll_protocol = htons(ETH_P_ALL);
    addr.sll_ifindex = ifindex;

    //for (int i = 0; i < bytes; i++) {
    //    if (((unsigned char)buffer[i] == 0x88) || ((unsigned char)buffer[i] == 0x81))
    //        printf("Byte problematico encontrado no pacote, pode causar erro\n");
    //}

    if (sendto(sockfd, buffer, bytes, 0, (struct sockaddr *) &addr, sizeof(struct sockaddr_ll)) < 0) {
        perror("sendto");
        exit(1);
    }
}

void send_command( int sockfd, char *buffer, int ifindex, unsigned char command )
{
    if ((command != ACK) && (command != NACK) && (command != LIST) && (command != END)) {
        fprintf(stderr, "Comando desconhecido: %d\n", (int) command);
        exit(1);
    }

    struct packet_header_t header = create_header();
    header.size = 10;
    header.type = command;
    int send_len = write_header(header, buffer);
    memcpy(buffer + send_len, "\0\0\0\0\0\0\0\0\0\0", header.size);
    send_len += header.size;
    send_len += write_crc(buffer, send_len);

    struct sockaddr_ll addr = {0};
    addr.sll_family = AF_PACKET;
    addr.sll_protocol = htons(ETH_P_ALL);
    addr.sll_ifindex = ifindex;

    //for (int i = 0; i < send_len; i++) {
    //    if (((unsigned char)buffer[i] == 0x88) || ((unsigned char)buffer[i] == 0x81))
    //        printf("Byte problematico encontrado no pacote, pode causar erro\n");
    //}

    if (sendto(sockfd, buffer, send_len, 0, (struct sockaddr *) &addr, sizeof(struct sockaddr_ll)) < 0) {
        perror("sendto");
        exit(1);
    }
}

void send_error( int sockfd, char *buffer, int ifindex, unsigned char error )
{
    struct packet_header_t header = create_header();
    header.size = 10;
    header.type = ERROR;
    int send_len = write_header(header, buffer);
    memcpy(buffer + send_len, &error, sizeof(unsigned char));
    memcpy(buffer + send_len + sizeof(unsigned char), "\0\0\0\0\0\0\0\0\0\0", header.size - sizeof(unsigned char));
    send_len += header.size;
    send_len += write_crc(buffer, send_len);

    struct sockaddr_ll addr = {0};
    addr.sll_family = AF_PACKET;
    addr.sll_protocol = htons(ETH_P_ALL);
    addr.sll_ifindex = ifindex;

    //for (int i = 0; i < send_len; i++) {
    //    if (((unsigned char)buffer[i] == 0x88) || ((unsigned char)buffer[i] == 0x81))
    //        printf("Byte problematico encontrado no pacote, pode causar erro\n");
    //}

    if (sendto(sockfd, buffer, send_len, 0, (struct sockaddr *) &addr, sizeof(struct sockaddr_ll)) < 0) {
        perror("sendto");
        exit(1);
    }
}

long long timestamp() {
    struct timeval t;
    gettimeofday(&t, NULL);
    return ((t.tv_sec*1000) + (t.tv_usec/1000));
}

int expect_response( int sockfd, char *buffer, int buffer_size, int timeout_ms )
{
    int received_len, read_len;
    struct packet_header_t header;
    long long start = timestamp();
    do {
        received_len = recvfrom(sockfd, buffer, buffer_size, 0, NULL, NULL);
        if (received_len < 0) {
            perror("erro em recvfrom");
            close(sockfd);
            exit(1);
        }

        if (is_packet(buffer, received_len)) {
            read_len = read_header(&header, buffer);
            if (! valid_crc(buffer, read_len + header.size)) {
                return INVALID_CRC;
            } else {
                if (header.type == ACK) return RECEIVED_ACK;
                else if (header.type == NACK) return RECEIVED_NACK;
                else if (header.type == ERROR) return RECEIVED_ERROR;
                else {
                    printf("Resposta de tipo inesperado recebida: %d\n", (int) header.type);
                    return UNEXPECTED_TYPE;
                }
            }
        }
    } while ((timestamp() - start) < timeout_ms);

    return TIMEOUT;
}