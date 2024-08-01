#ifndef SOCKET_H_
#define SOCKET_H_

int create_raw_socket(char *interface);

void send_packet(int sockfd, char* buffer, int bytes, int ifindex);

#endif