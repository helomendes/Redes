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

// Auxiliary Functions
int add_vlan_bytes(char *send_buffer, char *buffer, int bytes)
{
    int shift = 0;
    for (int i = 0; i < bytes; i++) {
        send_buffer[i + shift] = buffer[i];
        if (((unsigned char)buffer[i] == 0x88) || ((unsigned char)buffer[i] == 0x81)) {
            shift++;
            send_buffer[i + shift] = (unsigned char)0xff;
        }
    }
    return shift;
}

int remove_vlan_bytes( char *buffer, char *receive_buffer, int buffer_size, int bytes )
{
    int shift = 0;
    for (int i = 0; (((i + shift) < bytes) && (i < buffer_size)); i++) {
        buffer[i] = receive_buffer[i + shift];
        if ((((unsigned char)receive_buffer[i + shift] == 0x88) || ((unsigned char)receive_buffer[i + shift] == 0x81)) && ((unsigned char)receive_buffer[i+shift+1] == 0xff)) shift++;
    }
    return shift;
}

// Lib Functions

uint64_t timestamp() {
    struct timeval t;
    gettimeofday(&t, NULL);
    return ((t.tv_sec*1000) + (t.tv_usec/1000));
}

int create_raw_socket( char* interface )
{
    int sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sockfd == -1) {
        fprintf(stderr, "Erro ao criar socket: Verifique se voce eh root!\n");
        close(sockfd);
        exit(2);
    }

    int ifindex = if_nametoindex(interface);

    struct sockaddr_ll addr = {0};
    addr.sll_family = AF_PACKET;
    addr.sll_protocol = htons(ETH_P_ALL);
    addr.sll_ifindex = ifindex;

    if (bind(sockfd, (struct sockaddr*) &addr, sizeof(addr)) == -1) {
        fprintf(stderr, "Erro ao fazer bind no socket\n");
        close(sockfd);
        exit(2);
    }

    struct packet_mreq mr = {0};
    mr.mr_ifindex = ifindex;
    mr.mr_type = PACKET_MR_PROMISC; // Modo promiscuo

    if (setsockopt(sockfd, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) == -1) {
        fprintf(stderr, "Erro ao fazer setsockopt: verifique se a interface de rede foi especificada corretamente\n");
        close(sockfd);
        exit(2);
    }

    return sockfd;
}

int expect_response( int sockfd, char *buffer, int buffer_size, int timeout_ms )
{
    int received_len, read_len;
    struct packet_header_t header;
    long long start = timestamp();
    do {
        received_len = receive_packet(sockfd, buffer, buffer_size);
        if (received_len < 0) {
            fprintf(stderr, "erro em recvfrom\n");
            close(sockfd);
            exit(2);
        }

        if (is_packet(buffer, received_len)) {
            read_len = read_header(&header, buffer);
            if (! valid_crc(buffer, read_len + header.size)) {
                return INVALID_CRC;
            } else {
                if (header.type == ACK) return RECEIVED_ACK;
                else if (header.type == NACK) return RECEIVED_NACK;
                else if (header.type == ERROR) return RECEIVED_ERROR;
                else return UNEXPECTED_TYPE;
            }
        }
    } while ((timestamp() - start) < timeout_ms);

    return TIMEOUT;
}

int receive_packet( int sockfd, char *buffer, int buffer_size )
{
    char receive_buffer[buffer_size * 2];
    int received_len = recvfrom(sockfd, receive_buffer, buffer_size * 2, 0, NULL, NULL);
    received_len -= remove_vlan_bytes(buffer, receive_buffer, buffer_size, received_len);
    return received_len;
}

void send_packet(int sockfd, char* buffer, int bytes, int ifindex)
{
    struct sockaddr_ll addr = {0};
    addr.sll_family = AF_PACKET;
    addr.sll_protocol = htons(ETH_P_ALL);
    addr.sll_ifindex = ifindex;

    char send_buffer[bytes * 2];
    bytes += add_vlan_bytes(send_buffer, buffer, bytes);

    if (sendto(sockfd, send_buffer, bytes, 0, (struct sockaddr *) &addr, sizeof(struct sockaddr_ll)) < 0) {
        fprintf(stderr, "Erro em sendto\n");
        close(sockfd);
        exit(2);
    }
}

void send_command( int sockfd, char *buffer, int ifindex, unsigned char command )
{
    if ((command != ACK) && (command != NACK) && (command != LIST) && (command != END)) {
        fprintf(stderr, "Comando desconhecido: %d\n", (int) command);
        close(sockfd);
        exit(2);
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

    char send_buffer[send_len * 2];
    send_len += add_vlan_bytes(send_buffer, buffer, send_len);

    if (sendto(sockfd, send_buffer, send_len, 0, (struct sockaddr *) &addr, sizeof(struct sockaddr_ll)) < 0) {
        fprintf(stderr, "erro em sendto\n");
        close(sockfd);
        exit(2);
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

    char send_buffer[send_len * 2];
    send_len += add_vlan_bytes(send_buffer, buffer, send_len);

    if (sendto(sockfd, send_buffer, send_len, 0, (struct sockaddr *) &addr, sizeof(struct sockaddr_ll)) < 0) {
        fprintf(stderr, "erro em sendto\n");
        close(sockfd);
        exit(2);
    }
}