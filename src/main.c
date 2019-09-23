#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "utils.h"
#include "rselect.h"


void imprimirVetor(int* A, int n) {
    for (int i = 0; i < n; i++)
        printf("%d ", A[i]);
}

void processar(int n, int i, int imprimirtudo, int numthreads) {
    time_t hora;
    dados_t dados;
    
    srand((unsigned) time(&hora));
    dados.vetor = (int*)malloc(n * sizeof(int));
    dados.numthreads = numthreads;
    dados.tamanho = n;
    dados.posicao = i;
    dados.resultado = 0;
    dados.tempogasto = 0.0;
    for (int i = 0; i < n; i++)
        dados.vetor[i] = rand() % 50;
        
    selecionar(&dados);

    if (imprimirtudo) {
        imprimirVetor(dados.vetor, n);
        printf("\n%d\n", dados.resultado);
    }
    printf("%.6f\n", dados.tempogasto);
}

void receberEntrada(char* argv[], int* n, int* i, int* saidacompleta, int* t) {
    *n = atoi(argv[1]);
    *i = atoi(argv[2]);
    *saidacompleta = 0;
    if (argv[3][0] == 'a')
        *saidacompleta = 1;
    *t = atoi(argv[4]);
}

int main(int argc, char *argv[]) {
    int tamanho;
    int iesimo;
    int saida;
    int numthreads;
    
    if (argc > 4) {
        receberEntrada(argv, &tamanho, &iesimo, &saida, &numthreads);
        processar(tamanho, iesimo, saida, numthreads);
    }
    else
        exit(EXIT_FAILURE);
    exit(EXIT_SUCCESS);
}
