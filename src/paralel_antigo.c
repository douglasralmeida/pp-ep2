#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include "piscina.h"
#include "utils.h"

typedef struct mudanca_s {
    int* bitsE;
    int* esq;
    int* prefixoSomaE;
    int* bitsD;
    int* dir;
    int* prefixoSomaD;
    int* A;  
    int id;
} mudanca_t;

typedef struct particao_s {
    int* dir;
    int tamanhoD;
    int* esq;
    int tamanhoE;
} particao_t;

void mudar(mudanca_t* dados) {
  if (dados->bitsE[dados->id] == 1)
    dados->esq[dados->prefixoSomaE[dados->id] - 1] = dados->A[dados->id];
  if (dados->bitsD[dados->id] == 1)
    dados->dir[dados->prefixoSomaD[dados->id] - 1] = dados->A[dados->id];
}

void somarPrefixos(int* prefixoSomaE, int* prefixoSomaD, int* bitsE, int* bitsD, int n) {
    prefixoSomaE[0] = bitsE[0];
    prefixoSomaD[0] = bitsD[0];
    for(int i = 1; i < n; i++) {
        prefixoSomaE[i] = prefixoSomaE[i-1] + bitsE[i];
        prefixoSomaD[i] = prefixoSomaD[i-1] + bitsD[i];
    }
}

particao_t* particao(int* A, int n, int pivo, piscina_t* piscina) {
    int j;
    int* bitsE = (int*)calloc(n, sizeof(int));
    int* bitsD = (int*)calloc(n, sizeof(int));
    int* prefixoSomaE = (int*)calloc(n, sizeof(int));
    int* prefixoSomaD = (int*)calloc(n, sizeof(int));
  
    for (j = 0; j < n; j++) {
        if (A[j] < pivo)
            bitsE[j] = 1;
        else if (A[j] > pivo)
            bitsD[j] = 1;
    }
    somarPrefixos(prefixoSomaE, prefixoSomaD, bitsE, bitsD, n);

    int tamanhoE = prefixoSomaE[n - 1];
    int tamanhoD = prefixoSomaD[n - 1];
    int* esq = (int*)calloc(tamanhoE, sizeof(int));
    int* dir = (int*)calloc(tamanhoD, sizeof(int));
    for(j = 0; j < n; j++) {
        int memalocar = 1;
        mudanca_t* dados = (mudanca_t*)malloc(sizeof(mudanca_t)); 
        dados->bitsE = bitsE;
        dados->esq = esq;
        dados->prefixoSomaE = prefixoSomaE;
        dados->bitsD = bitsD;
        dados->dir = dir;
        dados->prefixoSomaD = prefixoSomaD;
        dados->A = A;
        dados->id = j;

        piscina_enfileirar(piscina, (void*)dados, memalocar);
    }

  piscina_esperar(piscina);

  particao_t* resultado = (particao_t*)malloc(sizeof(particao_t));
  resultado->esq = esq;
  resultado->tamanhoE = tamanhoE;
  resultado->dir = dir;
  resultado->tamanhoD = tamanhoD;

  return resultado;
}

int SelecaoAleatoria(int* A, int n, int i, piscina_t* piscina) {
    int x = rand() % n;
    int pivo = A[x];
    particao_t* resultado = particao(A, n, pivo, piscina);

    if (resultado->tamanhoE == i - 1)
        return pivo;
    if (resultado->tamanhoE == 0 && resultado->tamanhoD == 0)
        return pivo;
    if (resultado->tamanhoE >= i)
        return SelecaoAleatoria(resultado->esq, resultado->tamanhoE, i, piscina);
    else
        return SelecaoAleatoria(resultado->dir, resultado->tamanhoD, i - resultado->tamanhoE - 1, piscina);
        
    return 0;
}

void selecionar(dados_t* dados) {
    double tempoinicial, tempofinal;
    
    piscina_t* piscina = piscina_criar((void*)(*mudar), dados->numthreads);
    
    tempoinicial = omp_get_wtime();
    dados->resultado = SelecaoAleatoria(dados->vetor, dados->tamanho, dados->posicao, piscina);
    tempofinal = omp_get_wtime();
    dados->tempogasto = tempofinal - tempoinicial;
}
