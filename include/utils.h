#ifndef UTILS_H
#define UTILS_H

typedef struct {
    int* vetor;
    int numthreads;
    int posicao;
    int resultado;
    int tamanho;
    double tempogasto;
} dados_t;

void selecionar(dados_t* dados);

void trocar(int* a, int* b);

#endif
