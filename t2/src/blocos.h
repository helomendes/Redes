#ifndef BLOCOS_H
#define BLOCOS_H

//------------------------------------------------------------------------------
// representa as informações de um bloco: número de vértices e de arestas
//

typedef struct info_t {
  unsigned int vertices, arestas;
} info_t;

typedef struct aresta {
	unsigned int v1, v2;
	struct aresta *prox;
	struct aresta *ant;
} aresta;

int *le_grafo();

void print_arestas(struct aresta *head);

struct aresta *criar_aresta(int v1, int v2);

void free_arestas(struct aresta *head);

//------------------------------------------------------------------------------
// Computa os tamanhos dos blocos de um grafo lido de stdin.
//
// O formato do grafo lido é uma sequência de inteiros separados por
// "whitespace", que devem ser interpretados da seguinte maneira.
//
// - o primeiro número lido é o número n de vertices do grafo,
//
// - cada par de inteiros subsequentes representa uma aresta do grafo,
//
// - os vértices do grafo são inteiros de 1 a n 
//
// Por exemplo, a sequência abaixo representa um grafo com quatro vértices,
// onde três deles formam um triângulo e o quarto é um vértice isolado.
//
// 4 1 4 
// 1
// 3 4 3
//
// A função recebe como argumento um ponteiro para um 'unsigned int' em que
// escreve o número de blocos, k, do grafo lido.
// Além disso, é retornado um vetor de k 'struct info_t' em que cada posição
// corresponde a um bloco distinto do grafo e representa o número de vértices e
// arestas deste bloco.
//
// Para o grafo do exemplo acima, por exemplo, a função armazena o valor 2
// na variável apontada por num_blocos e retorna um vetor {(3, 3), (1, 0)}.
//
// Esta função não faz qualquer checagem de correção do formato de
// entrada. Seu resultado é indefinido caso a entrada não codifique
// corretamente um grafo conforme a especificação acima.

info_t *info_blocos(unsigned int *num_blocos);


#endif
