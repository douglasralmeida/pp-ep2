#ifndef PISCINA_H
#define PISCINA_H

/* Implementa uma piscina de threads */

#include <pthread.h>

typedef struct piscina_fila_s piscina_fila_t;

struct piscina_fila_s {
	void* arg;
	int memalocada;
	piscina_fila_t* prox;
};

typedef struct piscina_s {
  	char cancelada;
  	void* (*funcao)(void*);
  	int faltam;
  	int numthreads;
  	piscina_fila_t* fila;
  	piscina_fila_t* proximos;
  	pthread_mutex_t mutex;
  	pthread_cond_t cond;
  	pthread_t* threads;
} piscina_t;


// cria a piscina de threads na memória
piscina_t* piscina_criar(void* (*thread_funcao)(void*), int numthreads);

// enfileira uma tarefa para ser executada na piscina de threads 
void piscina_enfileirar(piscina_t* piscina, void* arg, int memalocada);

// espera todas as tarefas serem executadas
void piscina_esperar(piscina_t* piscina);

// para as threads da piscina
void piscina_destruir(piscina_t* piscina);

#endif
