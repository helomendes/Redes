#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "blocos.h"

//preciso definir esse tamanho
#define MAX 20

int *ler_grafo() {
	char *str = malloc(MAX*sizeof(char));
	scanf("%[^\n]%*c", str);

	int *vetor = malloc((MAX+1)*sizeof(int));
	int lidos = 0;
	unsigned int j = 0;
	for (unsigned int i = 0; i < MAX; i++) {
		if (str[i] < 48 || str[i] > 57) {
			continue;
		}
		if (str[i] != ' ') {
			vetor[j] = str[i] - '0';
			lidos++;
			j++;
		}
	}
	free(str);
	vetor[MAX] = lidos - 1;
	return vetor;
}

void print_arestas(struct aresta *head) {
	struct aresta *aux = head;
	while (aux) {
		printf("%d - %d\n", aux->v1, aux->v2);
		aux = aux->prox;
	}
}

struct aresta *criar_aresta(int v1, int v2) {
	struct aresta *new = malloc(sizeof(struct aresta));
	new->v1 = v1;
	new->v2 = v2;
	new->prox = NULL;
	new->ant = NULL;
	return new;
}


void free_arestas(struct aresta *head) {
	struct aresta *aux = head;
	while (aux->prox) {
		aux = aux->prox;
		free(aux->ant);
	}
	free(aux);
}


info_t *info_blocos(unsigned int *num_blocos) {

	// TESTE
	struct info_t *cobaia = malloc(sizeof(struct info_t));
	cobaia->vertices = 3;
	cobaia->arestas = 3;
	*num_blocos = 1;
	// 
	
	int *vetor = ler_grafo();
	int n = vetor[0];
	
	struct aresta *head = criar_aresta(vetor[1], vetor[2]);
	struct aresta *aux = head;
	for (int i = 1; i < vetor[MAX]/2; i++) {
		struct aresta *nova = criar_aresta(vetor[i*2+1], vetor[i*2+2]);
		nova->ant = aux;
		aux->prox = nova;
		aux = nova;
	}
	print_arestas(head);
	
	free_arestas(head);
	free(vetor);
	
	return cobaia;
}
