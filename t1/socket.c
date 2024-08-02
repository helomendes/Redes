#include <stdio.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <stdlib.h>
#include <unistd.h>

#include "socket.h"
#include "packet.h"

int create_raw_socket(char* interface)
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
    header.sequence = 1;
    int send_len = write_header(header, buffer);
    send_len += header.size;
    send_len += write_crc(buffer, send_len);

    struct sockaddr_ll addr = {0};
    addr.sll_family = AF_PACKET;
    addr.sll_protocol = htons(ETH_P_ALL);
    addr.sll_ifindex = ifindex;

    if (sendto(sockfd, buffer, send_len, 0, (struct sockaddr *) &addr, sizeof(struct sockaddr_ll)) < 0) {
        perror("sendto");
        exit(1);
    }
}

int expect_response( int sockfd, char *buffer, int buffer_size )
{
    // TODO: incluir timeout depois
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
                // erro no crc, quem chamou essa funcao manda um nack
                fprintf(stderr, "Erro detectado pelo crc\n");
                return 10;
            } else {
                if (header.type == ACK) return 0;
                else if (header.type == NACK) return 1;
                else if (header.type == ERROR) return 2;
                else {
                    printf("Resposta recebida: %d\n", (int) header.type);
                    return 3;
                }
            }
        }
    }

    return 0;
}
