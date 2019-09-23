#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include "piscina.h"
#include "utils.h"

typedef struct mudanca_s {
    int *bitsL;
    int *L;
    int *prefixSumL;
    int *bitsR;
    int *R;
    int *prefixSumR;
    int *A;  
    int idx;
} mudanca_t;

typedef struct particao_s {
    int *R;
    int sizeR;
    int *L;
    int sizeL;
} particao_t;

void mudar(mudanca_t* dados) {
  if (dados->bitsL[dados->idx] == 1)
    dados->L[dados->prefixSumL[dados->idx] - 1] = dados->A[dados->idx];
  if (dados->bitsR[dados->idx] == 1)
    dados->R[dados->prefixSumR[dados->idx] - 1] = dados->A[dados->idx];
}

void somarPrefixos(int* prefixSumL, int* prefixSumR, int* bitsL, int* bitsR, int n) {
    prefixSumL[0] = bitsL[0];
    prefixSumR[0] = bitsR[0];
    for(int i = 1; i < n; i++) {
        prefixSumL[i] = prefixSumL[i-1] + bitsL[i];
        prefixSumR[i] = prefixSumR[i-1] + bitsR[i];
    }
}

particao_t* particao(int* A, int n, int pivo, piscina_t* piscina) {
    int j;
    int* bitsL = (int*)calloc(n, sizeof(int));
    int* bitsR = (int*)calloc(n, sizeof(int));
    int* prefixSumL = (int*)calloc(n, sizeof(int));
    int* prefixSumR = (int*)calloc(n, sizeof(int));
  
    for (j = 0; j < n; j++) {
        if (A[j] < pivo)
            bitsL[j] = 1;
        else if (A[j] > pivo)
            bitsR[j] = 1;
    }
    somarPrefixos(prefixSumL, prefixSumR, bitsL, bitsR, n);

    int sizeL = prefixSumL[n - 1];
    int sizeR = prefixSumR[n - 1];
    int* L = (int*)calloc(sizeL, sizeof(int));
    int* R = (int*)calloc(sizeR, sizeof(int));
    for(j = 0; j < n; j++) {
        int memalocar = 1;
        mudanca_t* dados = (mudanca_t*)malloc(sizeof(mudanca_t)); 
        dados->bitsL = bitsL;
        dados->L = L;
        dados->prefixSumL = prefixSumL;
        dados->bitsR = bitsR;
        dados->R = R;
        dados->prefixSumR = prefixSumR;
        dados->A = A;
        dados->idx = j;

        piscina_enfileirar(piscina, (void*)dados, memalocar);
    }

  piscina_esperar(piscina);

  particao_t* resultado = (particao_t*)malloc(sizeof(particao_t));
  resultado->L = L;
  resultado->sizeL = sizeL;
  resultado->R = R;
  resultado->sizeR = sizeR;

  return resultado;
}

int SelecaoAleatoria(int* A, int n, int i, piscina_t* piscina) {
    int x = rand() % n;
    int pivo = A[x];
    particao_t* resultado = particao(A, n, pivo, piscina);

    if (resultado->sizeL == i - 1)
        return pivo;
    if (resultado->sizeL == 0 && resultado->sizeR == 0)
        return pivo;
    if (resultado->sizeL >= i)
        return SelecaoAleatoria(resultado->L, resultado->sizeL, i, piscina);
    else
        return SelecaoAleatoria(resultado->R, resultado->sizeR, i - resultado->sizeL - 1, piscina);
}

void selecionar(dados_t* dados) {
    double tempoinicial, tempofinal;
    
    piscina_t* piscina = piscina_criar((void*)(*mudar), dados->numthreads);
    
    tempoinicial = omp_get_wtime();
    dados->resultado = SelecaoAleatoria(dados->vetor, dados->tamanho, dados->posicao, piscina);
    tempofinal = omp_get_wtime();
    dados->tempogasto = tempofinal - tempoinicial;
}
