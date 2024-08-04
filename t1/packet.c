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

uint8_t calculate_crc(char *buffer, int bytes) {
    uint8_t crc = 0;

    for (int i = 0; i < bytes; ++i) {
        crc ^= buffer[i];

        for (uint8_t bit = 0; bit < 8; ++bit) {
            if (crc & 0x80) crc = (crc << 1) ^ CRC_POLY;
            else crc <<= 1;
        }
    }

    return crc;
}


unsigned int write_crc( char *buffer, int bytes )
{
    uint8_t crc = calculate_crc(buffer, bytes);
    buffer[bytes] = crc;
    //memcpy(buffer + bytes, &crc, sizeof(unsigned char));
    return sizeof(uint8_t);
}

int valid_crc( char *buffer, int bytes )
{
    return (calculate_crc(buffer, bytes + sizeof(uint8_t)) == 0);
}
