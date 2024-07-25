#include <stdio.h>

#include "packet.h"

packet_header_t cria_header()
{
    packet_header_t p = {0};
    p.init_marker = INIT_MARKER;
    p.size = 0;
    p.sequence = 0;
    p.type = 0;

    return p;
}

int eh_header(packet_header_t p) {
    return (p.init_marker == INIT_MARKER);
}

void imprime_header(packet_header_t p)
{
    printf("size: %d\n", p.size);
    printf("sequence: %d\n", p.sequence);
    printf("type: %d\n", p.type);
}