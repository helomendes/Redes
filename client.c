#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <errno.h>
#include <string.h>

#include "packet.h"

#define FRAME_LEN 128
#define DATA_SIZE 63

int main (int argc, char **argv) {
    if (argc != 2) {
        printf("Erro: execucao incorreta\n");
        printf("Exemplo: sudo ./client interface_de_rede\n");
        exit(1);
    }

    int soquete;
    int send_len = 0;
    unsigned int bytes_escritos;
    char send_buf[FRAME_LEN];
    char interface[8];
    strncpy(interface, argv[1], 8);
    struct packet_header_t header = cria_header();
    header.type = DADOS;

    if ((soquete = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) == -1) {
        perror("Erro ao criar socket");
        exit(1);
    }

    char data[DATA_SIZE];
    printf("Insira uma mensagem para ser enviada: ");
    scanf("%[^\n]", data);
    getchar();
    header.size = strlen(data);

    bytes_escritos = escreve_header(header, send_buf);
    send_len += bytes_escritos;
    strcpy(send_buf + send_len, data);
    send_len += strlen(data);
    escreve_crc(send_buf, send_len);

    int ifindex = if_nametoindex(interface);

    struct sockaddr_ll sock_addr = {0};
    sock_addr.sll_family = AF_PACKET;
    sock_addr.sll_protocol = htons(ETH_P_ALL);
    sock_addr.sll_ifindex = ifindex;

    struct packet_mreq mr = {0};
    mr.mr_ifindex = ifindex;
    mr.mr_type = PACKET_MR_PROMISC;

    if (setsockopt(soquete, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) == -1) {
        fprintf(stderr, "Erro ao fazer setsockopt: verifique se a interface de rede foi especificada corretamente");
        exit(1);
    }

    if (sendto(soquete, send_buf, send_len, 0, (struct sockaddr *) &sock_addr, sizeof(struct sockaddr_ll)) < 0) {
        printf("%d\n", errno);
        perror("sendto");
        exit(1);
    }

    close(soquete);
    return 0;
}
