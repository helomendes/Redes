#ifndef SOCKET_H_
#define SOCKET_H_

typedef enum {
    RECEIVED_ACK = 0,
    RECEIVED_NACK,
    RECEIVED_ERROR,
    INVALID_CRC,
    TIMEOUT,
    UNEXPECTED_TYPE,
} response_status;

int create_raw_socket( char *interface );

void send_packet( int sockfd, char *buffer, int bytes, int ifindex );

void send_command( int sockfd, char *buffer, int ifindex, unsigned char command );

void send_error( int sockfd, char *buffer, int ifindex, unsigned char error );

int expect_response( int sockfd, char *buffer, int buffer_size, int timeout_ms );

#endif