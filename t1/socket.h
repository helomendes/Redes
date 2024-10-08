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

uint64_t timestamp();

int create_raw_socket( char *interface );

int expect_response( int sockfd, char *buffer, int buffer_size, int timeout_ms );

int receive_packet( int sockfd, char *buffer, int buffer_size );

void send_command( int sockfd, char *buffer, int ifindex, unsigned char command );

void send_error( int sockfd, char *buffer, int ifindex, unsigned char error );

void send_packet( int sockfd, char *buffer, int bytes, int ifindex );

#endif