#ifndef UTILS_H
#define UTILS_H

typedef struct {
    int* vetor;
    long numthreads;
    long posicao;
    int resultado;
    long tamanho;
    double tempogasto;
} dados_t;

void selecionar(dados_t* dados);

void trocar(int* a, int* b);

#endif
