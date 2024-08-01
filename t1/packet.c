#include <stdio.h>
#include <string.h>

#include "packet.h"

struct packet_header_t create_header()
{
    struct packet_header_t p = {0};
    p.init_marker = INIT_MARKER;
    p.size = 0;
    p.sequence = 1;
    p.type = 1;

    return p;
}

int is_header(struct packet_header_t p)
{
    return (p.init_marker == INIT_MARKER);
}

void print_header(struct packet_header_t p)
{
    printf("size: %d\n", p.size);
    printf("sequence: %d\n", p.sequence);
    printf("type: %d\n", p.type);
}

unsigned int write_header(struct packet_header_t p, char* buffer)
{
    unsigned short size_sequence_type = (p.size) + (p.sequence << 6) + (p.type << 11);
    memcpy(buffer, &(p.init_marker), sizeof(SIZEOF_INITMARKER));
    memcpy(buffer + SIZEOF_INITMARKER, &size_sequence_type, sizeof(unsigned short));
    return SIZEOF_INITMARKER + sizeof(unsigned short);
}

int read_header(struct packet_header_t *p, char* buffer)
{
    memcpy(&(p->init_marker), buffer, SIZEOF_INITMARKER);
    if (! is_header(*p))
        return 0;
    unsigned short size_sequence_type;
    memcpy(&size_sequence_type, buffer + SIZEOF_INITMARKER, sizeof(unsigned short));

    p->size = size_sequence_type;
    p->sequence = size_sequence_type >> 6;
    p->type = size_sequence_type >> 11;

    return SIZEOF_INITMARKER + sizeof(unsigned short);
}

unsigned int write_crc(char *buffer, int bytes)
{
    unsigned char crc = 15;
    memcpy(buffer + bytes, &crc, sizeof(unsigned char));
    return sizeof(unsigned char);
}

int valid_crc(char *buffer, int bytes)
{
    unsigned char crc;
    memcpy(&crc, buffer + bytes, sizeof(unsigned char));
    // implementar verificacao de validade do crc
    if (crc == 15)
        return 1;
    return 0;
}
