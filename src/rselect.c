#include <stdio.h>
#include <stdlib.h>
#include <utils.h>
#include <rselect.h>

// Randomized Selection

long particao(int a[], long p, long r) {
    int x = a[r];
    long i = p - 1;

    for (long j = p; j < r; j++) {
        if (a[j] <= x) {
            i++;
            trocar(a + i, a + j);
        }
    }
    trocar(a + i + 1, a + r);

    return i + 1;
}

long SelecaoAleatoria(int a[], long p, long r, long i) {
    long pivo;
    long q;
    long k;
        
    if (p == r)
        return a[p];
    pivo = p + rand() % (r - p + 1);
    trocar(a + pivo, a + r);
    q = particao(a, p, r);
    k = q - p + 1;
    if (i == k)
        return a[q];
    else if (i < k)
        return SelecaoAleatoria(a, p, q - 1, i);
    else return SelecaoAleatoria(a, q + 1, r, i - k);
}
