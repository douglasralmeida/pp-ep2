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

int global_achouresultado = 0; //Guarda a informação de resultado encontrada

int global_escolheresq = 1; //Avisa as threads para escolherem o lado esq ou dir

long global_numthreads; //Guarda o numero de threads criadas

int global_pivo; //Guarda o valor do pivô escolhido aleatoriamente

long global_i; // Guarda a posição desejada

long* global_klocais; //Guarda a k-ésima posição que cada thread calculou

long global_k; //Guarda a posição real do pivo

vetor_t global_dados; //Guarda os dados que serão tratados pelo algoritmo

// Barreiras de sincronização

pthread_barrier_t barreira_pivo;

pthread_barrier_t barreira_kglobal;

pthread_barrier_t barreira_escolherpartic;

pthread_barrier_t barreira_processarescolha;

//Separa a partição em duas partes...
//..a esquerda com os números menores ou iguais ao pivô
//..a direita com os números maios que o pivô
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

//Procura pelas partições de cada thread onde está
//o pivô escolhido
int obter_pivo(threadinfo_t* info) {
    long i, x;
    
    x = rand() % global_dados.tamanho;
    for (i = 0; i < global_numthreads; i++) {
        if (x < info[i].dir - info[i].esq + 1)
            return global_dados.dados[info[i].esq + x];
        x -= info[i].dir - info[i].esq + 1;
    }
    
    printf("Erro: Pivô não pôde não encontrado.\n");
    abort();
}

// Realiza Seleção sequencial quado houverem poucos itens
int SelecaoAleatoriaSequencial(threadinfo_t* info) {
    int A[6 * global_numthreads];
    long i, j, k;

    for (i = 0, j = 0; i < global_numthreads; i++) {
        for (k = info[i].esq; k <= info[i].dir; k++) {  
            A[j] = info[i].vetor[k];
            j++;
        }
    }
    return SelecaoAleatoria(A, 0, global_dados.tamanho - 1, global_i);
}

//SelecaoAleatoriaDistribuida é executada em cada thread paralelamente
void* SelecaoAleatoriaDistribuida(void* info) {
    long local_k;
    threadinfo_t* local_info = (threadinfo_t*)info;
    
    while (1) {

        //A thread 0 calcula o pivô e o distribui para as demais
        //desde que a quantidade de dados seja satisfatória
        //do contrário, ela mesmo faz a pesquisa sequencialmente
        if (local_info->indice == 0) {
            if (6 * global_numthreads >= global_dados.tamanho) {
                resultado = SelecaoAleatoriaSequencial(local_info);
                global_achouresultado = 1;
            } else
                global_pivo = obter_pivo(local_info);
        }
        
        //Barreira de sincronização
        //As demais threads estão aguardando pelo pivô da
        //thread 0
        pthread_barrier_wait(&barreira_pivo);
        
        //A thread 0 foi egoísta e decidiu procurar pelo
        //resultado sozinha sequenciamente e já o encontrou
        //Hora de ir embora, então
        if (global_achouresultado)
            pthread_exit(NULL);
    
        //Cada thread calculará a sua nova partição e a posição local
        //hipotética que o pivô teria se estivesse em uma destas partições
        if (local_info->esq > local_info->dir)
            local_k = 0;
        else
            local_k = particaoDistribuida(local_info->vetor, local_info->esq, local_info->dir, global_pivo);
        global_klocais[local_info->indice] = local_k;
        
        //Barreira de sincronização
        //Cada thread deve espearar até que todas as outras threads tenham
        //calculado seu k local
        pthread_barrier_wait(&barreira_kglobal);

        //A thread 0 consolida as informações e encontra a posição k real do pivô
        if (local_info->indice == 0) {
            global_k = 0;
            for (long i = 0; i < global_numthreads; i++)
                global_k += global_klocais[i];
            if (global_i == global_k) {
                
                //Achou o resultado...avisa as demais threads para encerrarem
                resultado = global_pivo;
                global_achouresultado = 1;
            }
            else if (global_i < global_k) {
        
                //Não achou o resultado ainda...
                //Avisa as outras threads para escolhear a partição da esquerda
                global_escolheresq = 1;
                global_dados.tamanho = global_k;
            }
            else {
    
                //Avisa as outras threads para escolhear a partição da direita
                global_escolheresq = 0;
                global_i -= global_k;
                global_dados.tamanho -= global_k;
            }
        }
        
        //Barreira de sincronização
        //As demais threads devem espearar até que a thread 0 tenha
        //determinado qual partição seguir ou o resultado final
        pthread_barrier_wait(&barreira_escolherpartic);
        if (global_achouresultado)
            pthread_exit(NULL);
        local_k += local_info->esq;
        if (global_escolheresq)
            local_info->dir = local_k - 1;
        else
            local_info->esq = local_k;
            
        //Barreira de sincronização
        //Cada thread deve espearar até que todas as outras threads
        //tenha atualizado os dados da escolha de partição
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
    pthread_barrier_destroy(&barreira_pivo);
    pthread_barrier_destroy(&barreira_kglobal);
    pthread_barrier_destroy(&barreira_processarescolha);
    pthread_barrier_destroy(&barreira_escolherpartic);
    free(threads);
    free(info);
}
