#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "rselect.h"

void selecionar(dados_t* dados) {
    double tempoinicial, tempofinal;
    
    tempoinicial = omp_get_wtime();
    dados->resultado = SelecaoAleatoria(dados->vetor, 0, dados->tamanho - 1, dados->posicao);
    tempofinal = omp_get_wtime();
    dados->tempogasto = tempofinal - tempoinicial;
}
