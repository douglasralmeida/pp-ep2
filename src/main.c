#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "utils.h"
#include "rselect.h"


void imprimirVetor(int* A, long n) {
    for (long i = 0; i < n; i++)
        printf("%d ", A[i]);
}

void processar(long n, long i, int imprimirtudo, long numthreads) {
    time_t hora;
    dados_t dados;
    
    srand((unsigned) time(&hora));
    dados.vetor = (int*)malloc(n * sizeof(int));
    if (dados.vetor == NULL) {
        printf("Erro ao alocar vetor de dados.\n");
        abort();
    }
    dados.numthreads = numthreads;
    dados.tamanho = n;
    dados.posicao = i;
    dados.resultado = 0;
    dados.tempogasto = 0.0;
    for (long j = 0; j < n; j++)
        dados.vetor[j] = rand();

    selecionar(&dados);

    if (imprimirtudo) {
        imprimirVetor(dados.vetor, n);
        printf("\n%d\n", dados.resultado);
    }
    printf("%.6f\n", dados.tempogasto);
}

void receberEntrada(char* argv[], long* n, long* i, int* saidacompleta, long* t) {
    *n = atol(argv[1]);
    *i = atol(argv[2]);
    *saidacompleta = 0;
    if (argv[3][0] == 'a')
        *saidacompleta = 1;
    *t = atol(argv[4]);
}

int main(int argc, char *argv[]) {
    long tamanho;
    long iesimo;
    int saida;
    long numthreads;
    
    if (argc > 4) {
        receberEntrada(argv, &tamanho, &iesimo, &saida, &numthreads);
        processar(tamanho, iesimo, saida, numthreads);
    }
    else {
        printf("Erro ao alocar vetor de dados.\n");
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}
