#ifndef SOCKET_H_
#define SOCKET_H_

int create_raw_socket( char *interface );

void send_packet( int sockfd, char *buffer, int bytes, int ifindex );

void send_command( int sockfd, char *buffer, int ifindex, unsigned char command );

int expect_response( int sockfd, char *buffer, int buffer_size );

//void catch_loopback( int sockfd, char *buffer, int buffer_size, unsigned char type );

#endif