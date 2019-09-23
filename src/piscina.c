#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "piscina.h"

static void* thread(void* arg);

// cria a piscina de threads na memória
piscina_t* piscina_criar(void* (*thread_funcao)(void*), int numthreads) {
    piscina_t* p;
    int i;

    p = (piscina_t*)malloc(sizeof(piscina_t));
    pthread_mutex_init(&p->mutex, NULL);
    pthread_cond_init(&p->cond, NULL);
    p->numthreads = numthreads;
    p->funcao = thread_funcao;
    p->cancelada = 0;
    p->faltam = 0;
    p->proximos = NULL;
    p->fila = NULL;
    p->threads = (pthread_t*)malloc(numthreads * sizeof(pthread_t));
    for (i = 0; i < numthreads; i++)
        pthread_create(&p->threads[i], NULL, &thread, p);

    return p;
}

// enfileira uma tarefa para ser executada na piscina de threads 
void piscina_enfileirar(piscina_t* piscina, void *arg, int memalocada) {
    piscina_fila_t* fila = (piscina_fila_t*)malloc(sizeof(piscina_fila_t));

    fila->arg = arg;
    fila->prox = NULL;
    fila->memalocada = memalocada;
    pthread_mutex_lock(&piscina->mutex);
    if (piscina->proximos != NULL)
        piscina->proximos->prox = fila;
    if (piscina->fila == NULL)
        piscina->fila = fila;
    piscina->proximos = fila;
    piscina->faltam++;
    pthread_cond_signal(&piscina->cond);
    pthread_mutex_unlock(&piscina->mutex);
}

// espera todas as tarefas serem executadas
void piscina_esperar(piscina_t* piscina) {
    pthread_mutex_lock(&piscina->mutex);
    while (!piscina->cancelada && piscina->faltam)
        pthread_cond_wait(&piscina->cond, &piscina->mutex);
    pthread_mutex_unlock(&piscina->mutex);
}

// para todas as threads da piscina
void piscina_destruir(piscina_t* piscina) {
    piscina_fila_t* fila;
    int i;

    piscina->cancelada = 1;
    pthread_mutex_lock(&piscina->mutex);
    pthread_cond_broadcast(&piscina->cond);
    pthread_mutex_unlock(&piscina->mutex);
    for (i = 0; i < piscina->numthreads; i++)
    		pthread_join(piscina->threads[i], NULL);
    while (piscina->fila != NULL) {
        fila = piscina->fila;
        piscina->fila = fila->prox;
        if (fila->memalocada)
            free(fila->arg);
        free(fila);
    }
    free(piscina);
}

static void* thread(void *arg) {
    piscina_fila_t* fila;
    piscina_t* piscina = (piscina_t*)arg;

    while (!piscina->cancelada) {
        pthread_mutex_lock(&piscina->mutex);
        while (!piscina->cancelada && piscina->fila == NULL)
            pthread_cond_wait(&piscina->cond, &piscina->mutex);
        if (piscina->cancelada) {
            pthread_mutex_unlock(&piscina->mutex);
            return NULL;
        }
        fila = piscina->fila;
        piscina->fila = fila->prox;
        if (fila == piscina->proximos)
            piscina->proximos = NULL;
        pthread_mutex_unlock(&piscina->mutex);
        piscina->funcao(fila->arg);
        if (fila->memalocada)
            free(fila->arg);
        free(fila);
        fila = NULL;
        pthread_mutex_lock(&piscina->mutex);
        piscina->faltam--;
        pthread_cond_broadcast(&piscina->cond);
        pthread_mutex_unlock(&piscina->mutex);
    }
    return NULL;
}
