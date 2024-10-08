#ifndef PACKET_H_
#define PACKET_H_

#include <stdint.h>

#define INIT_MARKER 0b01111110
#define ACK         0b00000
#define NACK        0b00001
#define LIST        0b01010
#define DOWNLOAD    0b01011
#define SHOW        0b10000
#define DESCRIPTOR  0b10001
#define DATA        0b10010
#define END         0b11110
#define ERROR       0b11111

typedef enum {
    ACCESS_DENIED = 1,
    NOT_FOUND,
    DISK_FULL,
    ATTEMPT_LIMIT,
    PACKET_TIMEOUT
} ERRORS;

#define SIZEOF_INITMARKER 1
#define SIZEOF_SMALLEST_PACKET 14

#define COMMAND_SEQUENCE 1

#define MAX_SIZE_VALUE      0b111111
#define MAX_SEQUENCE_VALUE  0b11111

#define CRC_POLY 0b111010101

struct packet_header_t {
    unsigned char init_marker;
    unsigned char size:6;
    unsigned char sequence:5;
    unsigned char type:5;
};

struct packet_header_t create_header();

int is_packet( char *buffer, int received_len );

int read_header( struct packet_header_t *header, char* buffer );

int valid_crc( char *buffer, int bytes );

int write_crc( char *buffer, int bytes );

int write_header( struct packet_header_t header, char* buffer );

void print_header( struct packet_header_t header );

#endif