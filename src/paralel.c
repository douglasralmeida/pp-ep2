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

typedef struct vetor_s {
    int* dados;
    unsigned long tamanho
} vetor_t;


unsigned long global_numthreads; //Guarda o numero de threads criadas
s
unsigned long global_pivo; //Guarda o valor do pivô escolhido aleatoriamente

unsigned long global_tamanhofalta; //Guarda o número de itens que ainda serão 
                                   //tratados pelo algoritmo

unsigned long global_i; // Guarda a posição desejada

unsigned long global_klocais*; //Guarda a k-ésima posição que cada thread calculou

unsigned long global_k; //Guarda a posição real do pivo

vetor_t global_dados; //Guarda os dados que serão tratados pelo algoritmo

pthread_cond_t cond_global_pivo = PTHREAD_COND_INITIALIZER;

pthread_mutex_t mutex_global_pivo = PTHREAD_MUTEX_INITIALIZER;

pthread_barrier_t barreira_global_k;


int particao(int a[], int p, int r, int x) {
    int i = p - 1;

    for (int j = p; j <= r; j++) {
        if(a[j] <= x){
            i++;
            trocar(a + i, a + j);
        }
    }

    return i - p + 1;
}

int obter_pivo(threadinfo_t* info) {
    int i, x, local_x, tam;
    
    tam = 0;
    x = rand() % global_tamanhofalta;
    local_x = x;
    for (i = 0; i < global_numthreads; i++) {
        tam += info[i].dir - info[i].esq + 1;
        if (local_x < tam)
            return global_dados.dados[info[i].esq + local_x];
        local_x -= info[i].dir - info[i].esq + 1;
    }
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
        global_pivo = obter_pivo(minhainfo);
        local_pivo = global_pivo;
        pthread_cond_broadcast(&cond_global_pivo);
        pthread_mutex_unlock(&mutex_global_pivo);
    } else {
        pthread_mutex_lock(&mutex_global_pivo);
        pthread_cond_wait(&cond_global_pivo, &mutex_global_pivo);
        local_pivo = global_pivo;
        pthread_mutex_unlock(&mutex_global_pivo);
    }
    
    //Cada thread calculará a sua nova partição e a posição local
    //hipotética que o pivô teria se estivesse em uma destas partições
    if (minhainfo->esq > minhainfo->dir)
        q = 0;
    else
        q = particao(info->vetor, minhainfo->esq, minhainfo->dir, local_pivo);
    global_klocais[minhainfo->indice] = q - minhainfo->esq + 1;
    
    //Cada thread deve espearar até que todas as outras threads tenham
    //calculado seu k local
    pthread_barrier_wait(&barreira_global_k);
    
    //A thread 0 junta as informações e encontra a posição k real do pivô
    if (minhainfo->indice == 0) {
        for (int i = 0; i < global_numthreads; i++)
            global_k += global_klocais[i];
        if (globak_i == global_k)
            resultado = local_pivo;
        else if (global_i < global_k)
            minhainfo->dir = 
        else {
            minhainfo->esq = 
        }
    }
    
    
    return NULL;
}

void selecionar(dados_t* dados) {
    unsigned long i;
    threadinfo_t* info;
    pthread_t* threads;
    double tempoinicial, tempofinal;

    global_i = dados->posicao;
    global_k = 0;
    global_numthreads = dados->numthreads;
    global_dados.dados = dados->vetor;
    global_dados.tamanho = dados->tamanho;
    global_tamanhofalta = dados->tamanho;
    global_khipotetico = (unsigned long*)malloc(dados->numthreads * sizeof(unsigned long));
    pthread_barrier_init(&barreira_global_k, NULL, dados->numthreads);
    
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
    pthread_barrier_destroy(&barreira_global_k);
    free(threads);
    free(info);
}
