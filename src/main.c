#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "utils.h"
#include "rselect.h"


void imprimirVetor(int* A, unsigned long n) {
    for (unsigned long i = 0; i < n; i++)
        printf("%d ", A[i]);
}

void processar(unsigned long n, unsigned long i, int imprimirtudo, unsigned long numthreads) {
    time_t hora;
    dados_t dados;
    
    srand((unsigned) time(&hora));
    dados.vetor = (int*)malloc(n * sizeof(int));
    dados.numthreads = numthreads;
    dados.tamanho = n;
    dados.posicao = i;
    dados.resultado = 0;
    dados.tempogasto = 0.0;
    for (unsigned long i = 0; i < n; i++)
        dados.vetor[i] = rand();
    
    //printf("TESTE\n");
    //imprimirVetor(dados.vetor, n);
    //printf("\n");
    
    selecionar(&dados);

    if (imprimirtudo) {
        imprimirVetor(dados.vetor, n);
        printf("\n%lu\n", dados.resultado);
    }
    printf("%.6f\n", dados.tempogasto);
}

void receberEntrada(char* argv[], unsigned long* n, unsigned long* i, int* saidacompleta, unsigned long* t) {
    *n = atol(argv[1]);
    *i = atol(argv[2]);
    *saidacompleta = 0;
    if (argv[3][0] == 'a')
        *saidacompleta = 1;
    *t = atol(argv[4]);
}

int main(int argc, char *argv[]) {
    unsigned long tamanho;
    unsigned long iesimo;
    int saida;
    unsigned long numthreads;
    
    if (argc > 4) {
        receberEntrada(argv, &tamanho, &iesimo, &saida, &numthreads);
        processar(tamanho, iesimo, saida, numthreads);
    }
    else
        exit(EXIT_FAILURE);
    exit(EXIT_SUCCESS);
}
