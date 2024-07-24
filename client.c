#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <errno.h>
#include <string.h>

#define FRAME_LEN 1024
#define INTERFACE "enp2s0"

int main () {
    //int soquete = cria_raw_socket("INTERFACE");
    int soquete;
    char send_buf[FRAME_LEN];
    int send_len = 0;

    if ((soquete = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) == -1) {
        perror("Erro ao criar socket");
        exit(1);
    }

    strcpy(send_buf, "Testando nova versao");
    send_len += strlen("Testando nova versao");

    int ifindex = if_nametoindex(INTERFACE);

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
        perror("sendto");
        exit(1);
    }

    close(soquete);
    return 0;
}