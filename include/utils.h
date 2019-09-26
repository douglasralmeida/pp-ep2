#ifndef UTILS_H
#define UTILS_H

typedef struct {
    int* vetor;
    unsigned long numthreads;
    unsigned long posicao;
    unsigned long resultado;
    unsigned long tamanho;
    double tempogasto;
} dados_t;

void selecionar(dados_t* dados);

void trocar(int* a, int* b);

#endif
