#include <stdio.h>
#include <stdlib.h>
#include <utils.h>
#include <rselect.h>

// Randomized Selection

int particao(int a[], int p, int r) {
    int pivo = a[r];
    int i = p - 1;

    for (int x = p; x < r; x++) {
        if(a[x] <= pivo){
            i++;
            trocar(a + i, a + x);
        }
    }
    trocar(a + i + 1, a + r);

    return i + 1;
}

int ParticaoAleatoria(int a[], int p, int r) {
    int i = p + rand() % (r - p + 1);

    trocar(a + i, a + p);

    return particao(a, p, r);
}

int SelecaoAleatoria(int a[], int p, int r, int i) {
    int q;
    int k;
    
    if (p == r)
        return a[p];
    q = ParticaoAleatoria(a, p, r);
    k = q - p + 1;
    if (i == k)
        return a[q];
    else if (i < k)
        return SelecaoAleatoria(a, p, q - 1, i);
    else return SelecaoAleatoria(a, q + 1, r, i - k);
}
