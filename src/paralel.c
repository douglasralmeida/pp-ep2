#include <omp.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rselect.h"
#include "utils.h"

typedef struct threadinfo_s {
    int* vetor;
    long esq;
    long dir;
    long indice;
} threadinfo_t;

typedef struct vetor_s {
    int* dados;
    long tamanho;
} vetor_t;

int resultado = 0;

int global_achouresultado = 0;

int global_escolheresq = 1;

long global_numthreads; //Guarda o numero de threads criadas

int global_pivo; //Guarda o valor do pivô escolhido aleatoriamente

long global_i; // Guarda a posição desejada

long* global_klocais; //Guarda a k-ésima posição que cada thread calculou

long global_k; //Guarda a posição real do pivo

vetor_t global_dados; //Guarda os dados que serão tratados pelo algoritmo

pthread_barrier_t barreira_pivo;

pthread_barrier_t barreira_kglobal;

pthread_barrier_t barreira_escolherpartic;

pthread_barrier_t barreira_processarescolha;


long particaoDistribuida(int a[], long p, long r, int x) {
    long i = p - 1;
    
    for (long j = p; j <= r; j++) {
        if (a[j] <= x) {
            i++;
            trocar(a + i, a + j);
        }
    }
    return i + 1 - p;
}

void imprimir(threadinfo_t* minhainfo) {
    char teste[1000000];
    long i;
    

    sprintf(teste, "Thread %ld: ", minhainfo->indice);
    for (i = minhainfo->esq; i <= minhainfo->dir; i++)
        sprintf(teste + strlen(teste), "%d ", minhainfo->vetor[i]);
    sprintf(teste + strlen(teste), "\n\n");
    
    printf("%s\n", teste);
    
//    printf("esq(%ld): %ld\n", minhainfo->indice, minhainfo->esq);
//    printf("dir(%ld): %ld\n", minhainfo->indice, minhainfo->dir);
}

int obter_pivo(threadinfo_t* info, long tam) {
    long i, x;
    
    x = rand() % tam;
    
    //printf("pos aleat: %lu\n", x);
    for (i = 0; i < global_numthreads; i++) {
        //printf("%lu < %lu - %lu + 1\n", x, info[i].dir, info[i].esq);
        if (x < info[i].dir - info[i].esq + 1) {
            //printf("sim\n");
            return global_dados.dados[info[i].esq + x];
        }
        x -= info[i].dir - info[i].esq + 1;
    }
    
    printf("Erro: Pivô não pôde não encontrado.\n");
    abort();
}

int SelecaoAleatoriaSequencial(threadinfo_t* info, long tam) {
    int A[6 * global_numthreads];
    long i, j, k;
    threadinfo_t abc = info[0];

    for (i = 0, j = 0; i < global_numthreads; i++) {
        for (k = info[i].esq; k <= info[i].dir; k++) {  
				    A[j] = info[i].vetor[k];
            j++;
        }
    }
    return SelecaoAleatoria(A, 0, tam - 1, global_i);
}

//SelecaoAleatoriaDistribuida é executada em cada thread paralelamente
void* SelecaoAleatoriaDistribuida(void* info) {
    long tam;
    long q;
    int local_pivo = -1;
    threadinfo_t* minhainfo = (threadinfo_t*)info;
    
    while (1) {
        //printf("===\n");
        //printf("iesimo: %lu\n", global_i);
        //imprimir(minhainfo); 
        
        //A thread 0 calcula o pivô e o distribui para as demais
        //desde que a quantidade de dados sejam satisfatória
        //do contrário a pesquisa será sequencial
        if (minhainfo->indice == 0) {
            tam = 0;
            for (long i = 0; i < global_numthreads; i++)
                tam += minhainfo[i].dir - minhainfo[i].esq + 1;
            //printf("tam: %lu\n", tam);
            if (6 * global_numthreads >= tam) {
                resultado = SelecaoAleatoriaSequencial(minhainfo, tam);
                global_achouresultado = 1;
            } else {
                global_pivo = obter_pivo(minhainfo, tam);
                local_pivo = global_pivo;
                //printf("pivo: %d\n", local_pivo);
                //printf("i: %lu\n", global_i);
            }
            
            //printf("Thread %lu transmitindo pivo.\n", minhainfo->indice);
            pthread_barrier_wait(&barreira_pivo);
            
            if (global_achouresultado)
                pthread_exit(NULL);
        } else {
            //printf("Thread %lu esperando transmissão pivo.\n", minhainfo->indice);
            pthread_barrier_wait(&barreira_pivo);
            
            if (global_achouresultado) {
                pthread_exit(NULL);
            } else {
                local_pivo = global_pivo;
            }
        }
    
        //Cada thread calculará a sua nova partição e a posição local
        //hipotética que o pivô teria se estivesse em uma destas partições
        if (minhainfo->esq > minhainfo->dir) {
            q = 0;
            //printf("k_local(%lu): (vazio)\n", minhainfo->indice);
        }
        else {
            q = particaoDistribuida(minhainfo->vetor, minhainfo->esq, minhainfo->dir, local_pivo);
            //printf("k_local(%lu): %lu\n", minhainfo->indice, q);
        }
        global_klocais[minhainfo->indice] = q;
        
        //imprimir(minhainfo);
        
        //Cada thread deve espearar até que todas as outras threads tenham
        //calculado seu k local
        //printf("Thread %lu na barreira do k global.\n", minhainfo->indice);
        pthread_barrier_wait(&barreira_kglobal);
    
        //A thread 0 junta as informações e encontra a posição k real do pivô
        if (minhainfo->indice == 0) {
            global_k = 0;
            for (long i = 0; i < global_numthreads; i++)
                global_k += global_klocais[i];
                
            //printf("global_k: %lu\n", global_k);
            if (global_i == global_k) {
                
                //printf("res: %d\n", local_pivo);
                
                //Achou o resultado...avisa as demais threads para encerrarem
                resultado = local_pivo;
                global_achouresultado = 1;
            }
            else if (global_i < global_k) {
        
                //Avisa as outras threads para escolhear a partição da esquerda
                global_escolheresq = 1;
            }
            else {
    
                //Avisa as outras threads para escolhear a partição da direita
                global_escolheresq = 0;
                global_i -= global_k;
            }
        }
        
        //Cada thread deve espearar até que todas a thread 0 tenha
        //determinado qual partição seguir ou encontrado o resultado
        //printf("Thread %lu na barreira da escolha do lado da partição.\n", minhainfo->indice);
        pthread_barrier_wait(&barreira_escolherpartic);
        if (global_achouresultado)
            pthread_exit(NULL);
        q += minhainfo->esq;
        if (global_escolheresq)
            minhainfo->dir = q - 1;
        else
            minhainfo->esq = q;
            
        //Cada thread deve espearar até que todas as outras threads
        //tenha escolhido sua partição
        //printf("Thread %lu na barreira para processamento da escolha.\n", minhainfo->indice);
        pthread_barrier_wait(&barreira_processarescolha);
    }
    
    return NULL;
}

void selecionar(dados_t* dados) {
    long i;
    threadinfo_t* info;
    pthread_t* threads;
    double tempoinicial, tempofinal;
    
    //Inicia variáveis globais p/ threads
    global_i = dados->posicao;
    global_k = 0;
    global_numthreads = dados->numthreads;
    global_dados.dados = dados->vetor;
    global_dados.tamanho = dados->tamanho;
    global_klocais = (long*)malloc(dados->numthreads * sizeof(long));
    
    //Inicia barreiras
    pthread_barrier_init(&barreira_kglobal, NULL, dados->numthreads);
    pthread_barrier_init(&barreira_escolherpartic, NULL, dados->numthreads);
    pthread_barrier_init(&barreira_processarescolha, NULL, dados->numthreads);
    pthread_barrier_init(&barreira_pivo, NULL, dados->numthreads);
    
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
       
    //Grava resultados
    dados->resultado = resultado;
    dados->tempogasto = tempofinal - tempoinicial;
    
    //Limpa a memória
    //pthread_barrier_destroy(&barreira_pivo);
    pthread_barrier_destroy(&barreira_kglobal);
    pthread_barrier_destroy(&barreira_processarescolha);
    pthread_barrier_destroy(&barreira_escolherpartic);
    free(threads);
    free(info);
}
