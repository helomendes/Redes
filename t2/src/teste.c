#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "blocos.h"

int main (void) {

  info_t *blocos = NULL;
  unsigned int k;

  blocos = info_blocos(&k);

  printf("O grafo consiste de %u blocos\n", k);

  for (unsigned int i = 0; i < k; i++) {

    printf("  Bloco %3u: %4u vértices, %4u arestas\n", i+1, blocos[i].vertices, blocos[i].arestas);
  }

  free(blocos);

  return errno;
}
