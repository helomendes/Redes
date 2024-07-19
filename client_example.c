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

#define ETHERNET_FRAME_LEN 1518

int main() {
    int sockfd;
    struct ifreq if_idx;
    struct ifreq if_mac;
    struct sockaddr_ll socket_address;
    char sendbuf[ETHERNET_FRAME_LEN];
    int send_len = 0;

    char dest_mac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}; // Broadcast address

    if ((sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) == -1) {
        perror("socket");
        exit(1);
    }

    memset(&if_idx, 0, sizeof(struct ifreq));
    strncpy(if_idx.ifr_name, "enp1s0", IFNAMSIZ - 1);
    if (ioctl(sockfd, SIOCGIFINDEX, &if_idx) < 0) {
        perror("SIOCGIFINDEX");
        exit(1);
    }

    memset(&if_mac, 0, sizeof(struct ifreq));
    strncpy(if_mac.ifr_name, "enp1s0", IFNAMSIZ - 1);
    if (ioctl(sockfd, SIOCGIFHWADDR, &if_mac) < 0) {
        perror("SIOCGIFHWADDR");
        exit(1);
    }

    struct ether_header *eh = (struct ether_header *) sendbuf;
    memset(sendbuf, 0, ETHERNET_FRAME_LEN);

    memcpy(eh->ether_dhost, dest_mac, 6);
    memcpy(eh->ether_shost, if_mac.ifr_hwaddr.sa_data, 6);
    eh->ether_type = htons(ETH_P_IP);

    send_len += sizeof(struct ether_header);
    strcpy(sendbuf + send_len, "Hello, Ethernet!");
    send_len += strlen("Hello, Ethernet!");

    socket_address.sll_ifindex = if_idx.ifr_ifindex;
    socket_address.sll_halen = ETH_ALEN;
    memcpy(socket_address.sll_addr, dest_mac, 6);

    if (sendto(sockfd, sendbuf, send_len, 0, (struct sockaddr *)&socket_address, sizeof(struct sockaddr_ll)) < 0) {
        perror("sendto");
        exit(1);
    }

    close(sockfd);
    return 0;
}
