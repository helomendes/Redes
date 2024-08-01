#ifndef SOCKET_H_
#define SOCKET_H_

int cria_raw_socket(char *nome_interface_rede);

void send_packet(int socket, char* buffer, int bytes, int ifindex);

#endif