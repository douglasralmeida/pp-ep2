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
    unsigned long tamanho;
} threadinfo_t;

void particao() {

}

unsigned long global_pivo;

pthread_cond_t cond_global_pivo = PTHREAD_COND_INITIALIZER;

pthread_mutex_t mutex_global_pivo = PTHREAD_MUTEX_INITIALIZER;


int particao(int a[], int p, int r, int x) {
    int i = p - 1;

    for (int j = p; j <= r; j++) {
        if(a[j] <= x){
            i++;
            trocar(a + i, a + j);
        }
    }

    return i + 1;
}

//SelecaoAleatoriaDistribuida é executada em cada thread paralelamente
void* SelecaoAleatoriaDistribuida(void* info) {
    unsigned long q;
    unsigned long pivo;
    threadinfo_t* minhainfo = (threadinfo_t*)info;

    //char teste[256];
    //unsigned long i;
    //sprintf(teste, "Thread %lu: ", minhainfo->indice);
    //for (i = minhainfo->esq; i <= minhainfo->dir; i++)
    //    sprintf(teste + strlen(teste), "%d ", minhainfo->vetor[i]);
    //sprintf(teste + strlen(teste), "\n\n");
    //printf("%s", teste);
    
    //A thread 0 calcula o pivô e o distribui para as demais
    if (minhainfo->indice == 0) {
        pthread_mutex_lock(&mutex_global_pivo);
        global_pivo = info->vetor[rand() % info->tamanho];
        local_pivo = global_pivo;
        pthread_cond_broadcast(&cond_global_pivo);
        pthread_mutex_unlock(&mutex_global_pivo);
    } else {
        pthread_mutex_lock(&mutex_global_pivo);
        pthread_cond_wait(&cond_global_pivo, &mutex_global_pivo);
        local_pivo = global_pivo;
        pthread_mutex_unlock(&mutex_global_pivo);
    }
    
    //Cada thread fara a sua partição e retornará a posição que o pivô 
    //tem ou teria se estivesse em uma das partições
    q = particao(info->vetor, info->esq, info->dir, local_pivo);
    
    //Todas as threads iram devem enviar a posição hipotética para a thread 0
    if (minhainfo->indice == 0) {
        
    } else 
        
    }
    
    
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
        info[i].tamanho = dados->tamanho;
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
