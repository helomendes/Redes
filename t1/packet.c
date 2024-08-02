#include <stdio.h>
#include <string.h>

#include "packet.h"

struct packet_header_t create_header()
{
    struct packet_header_t header = {0};
    header.init_marker = INIT_MARKER;
    header.size = 0;
    header.sequence = 1;
    header.type = 1;

    return header;
}

int is_packet( char *buffer, int received_len )
{
    return ((buffer[0] == INIT_MARKER) && (received_len >= SIZEOF_SMALLEST_PACKET));
}

void print_header( struct packet_header_t header )
{
    printf("size: %d\n", header.size);
    printf("sequence: %d\n", header.sequence);
    printf("type: %d\n", header.type);
}

unsigned int write_header( struct packet_header_t header, char* buffer )
{
    unsigned short size_sequence_type = (header.size) + (header.sequence << 6) + (header.type << 11);
    memcpy(buffer, &(header.init_marker), sizeof(SIZEOF_INITMARKER));
    memcpy(buffer + SIZEOF_INITMARKER, &size_sequence_type, sizeof(unsigned short));
    return SIZEOF_INITMARKER + sizeof(unsigned short);
}

int read_header( struct packet_header_t *header, char* buffer )
{
    memcpy(&(header->init_marker), buffer, SIZEOF_INITMARKER);
    unsigned short size_sequence_type;
    memcpy(&size_sequence_type, buffer + SIZEOF_INITMARKER, sizeof(unsigned short));

    header->size = size_sequence_type;
    header->sequence = size_sequence_type >> 6;
    header->type = size_sequence_type >> 11;

    return SIZEOF_INITMARKER + sizeof(unsigned short);
}

unsigned int write_crc( char *buffer, int bytes )
{
    unsigned char crc = 15;
    memcpy(buffer + bytes, &crc, sizeof(unsigned char));
    return sizeof(unsigned char);
}

int valid_crc( char *buffer, int bytes )
{
    unsigned char crc;
    memcpy(&crc, buffer + bytes, sizeof(unsigned char));
    // implementar verificacao de validade do crc
    if (crc == 15)
        return 1;
    return 0;
}
