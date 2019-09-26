#include <omp.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

typedef struct threadinfo_s {
    int* vetor;
    unsigned long esq;
    unsigned long dir;
    unsigned long indice;
} threadinfo_t;

void particao() {
}


//SelecaoAleatoriaDistribuida é executada em cada thread paralelamente
void* SelecaoAleatoriaDistribuida(void* info) {
    unsigned long i;
    threadinfo_t* minhainfo = (threadinfo_t*)info;

    char teste[256];
    
    sprintf(teste, "Thread %lu: ", minhainfo->indice);
    for (i = minhainfo->esq; i <= minhainfo->dir; i++)
        sprintf(teste + strlen(teste), "%d ", minhainfo->vetor[i]);
    sprintf(teste + strlen(teste), "\n\n");
    printf("%s", teste);
    
    return NULL;
}

void selecionar(dados_t* dados) {
    unsigned long i;
    threadinfo_t* info;
    pthread_t* threads;
    double tempoinicial, tempofinal;

    //Cria as threads e divide o vetor inicial para cada uma
    info = (threadinfo_t*)malloc(dados->numthreads * sizeof(threadinfo_t));
    threads = (pthread_t*)malloc(dados->numthreads * sizeof(pthread_t));
    for (i = 0; i < dados->numthreads; i++) {
        info[i].indice = i;
        info[i].esq = i * dados->tamanho / dados->numthreads;
        info[i].dir = (i+1) * dados->tamanho / dados->numthreads - 1;   
        info[i].vetor = dados->vetor;
    }
    info[i-1].dir = dados->tamanho - 1;
    
    //Executa todas as threads
    tempoinicial = omp_get_wtime();
    for (i = 0; i < dados->numthreads; i++)
        pthread_create(&(threads[i]), NULL, SelecaoAleatoriaDistribuida, (void*)(info + i));

    //Aguarda a finalização de todas as threads
    for (i = 0; i < dados->numthreads; i++)
        pthread_join(threads[i], NULL);
    tempofinal = omp_get_wtime();
       
    //        
    dados->resultado = 0;
    dados->tempogasto = tempofinal - tempoinicial;
    
    //Limpa a memória
    free(threads);
    free(info);
}
