#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <arpa/inet.h>
#include <linux/if_packet.h>

#define FRAME_LEN 1024
#define INTERFACE "enp1s0"

int main() {
    int sockfd;
    char buffer[FRAME_LEN];
    struct sockaddr_ll addr;
    socklen_t addr_len = sizeof(struct sockaddr_ll);

    if ((sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) == -1) {
        perror("socket");
        exit(1);
    }

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_name, "enp1s0", IFNAMSIZ - 1); // Change "eth0" to your interface
    if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr)) < 0) {
        perror("SO_BINDTODEVICE");
        close(sockfd);
        exit(1);
    }

    while (1) {
        memset(buffer, 0, FRAME_LEN);
        int numbytes = recvfrom(sockfd, buffer, FRAME_LEN, 0, (struct sockaddr *)&addr, &addr_len);
        if (numbytes < 0) {
            perror("recvfrom");
            close(sockfd);
            exit(1);
        }

        //struct ether_header *eh = (struct ether_header *) buffer;
        //if (eh->ether_dhost[0]) {
        if (buffer[0] != '\0') {
            printf("Received packet:\n");
            printf("Payload: %s\n", buffer);
            printf("Payload length: %d bytes\n", numbytes);
        }
        //printf("Destination MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
        //       eh->ether_dhost[0], eh->ether_dhost[1], eh->ether_dhost[2],
        //       eh->ether_dhost[3], eh->ether_dhost[4], eh->ether_dhost[5]);
        //printf("Source MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
        //       eh->ether_shost[0], eh->ether_shost[1], eh->ether_shost[2],
        //       eh->ether_shost[3], eh->ether_shost[4], eh->ether_shost[5]);
        //printf("EtherType: %04x\n", ntohs(eh->ether_type));
        //}
    }

    close(sockfd);
    return 0;
}
