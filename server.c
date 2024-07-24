#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define NETWORK_INTERFACE "enp1s0"
#define BUFFER_SIZE 1024

int cria_raw_socket( char* nome_interface_rede );

int main () {
    int soquete = cria_raw_socket(NETWORK_INTERFACE);

    char* buffer = malloc(BUFFER_SIZE);

    ssize_t bytes_recebidos;

    while (1) {
        bytes_recebidos = recvfrom(soquete, buffer, BUFFER_SIZE, 0, NULL, NULL);
        if (bytes_recebidos < 0) {
            perror("erro em recvfrom");
            close(soquete);
            exit(1);
        }

        if (buffer[0] != '\0') {
            printf("Bytes recebidos: %ld\n", bytes_recebidos);
            printf("Payload: %s\n", buffer);
        }
    }

    free(buffer);
    close(soquete);

    return 0;
}

int cria_raw_socket(char* nome_interface_rede)
{
    int soquete = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (soquete == -1) {
        fprintf(stderr, "Erro ao criar socket: Verifique se voce eh root!\n");
        exit(-1);
    }

    int ifindex = if_nametoindex(nome_interface_rede);

    struct sockaddr_ll endereco = {0};
    endereco.sll_family = AF_PACKET;
    endereco.sll_protocol = htons(ETH_P_ALL);
    endereco.sll_ifindex = ifindex;

    if (bind(soquete, (struct sockaddr*) &endereco, sizeof(endereco)) == -1) {
        fprintf(stderr, "Erro ao fazer bind no socket\n");
        exit(1);
    }

	struct packet_mreq mr = {0};
    mr.mr_ifindex = ifindex;
    mr.mr_type = PACKET_MR_PROMISC; // Modo promiscuo

    if (setsockopt(soquete, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) == -1) {
        fprintf(stderr, "Erro ao fazer setsockopt: verifique se a interface de rede foi especificada corretamente\n");
        exit(1);
    }

	return soquete;
}
