#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>

int nThread, n;
double *sumRes;

void *sumF(void *args) {
    int i, idThread;
    idThread = *(int*)args;

    for (i = idThread; i <= n; i += nThread) {
        sumRes[idThread-1] += (1 / ((double)i * 2 - 1)) * ((i & 1) ? 1 : -1);
    }

    free(args);
    pthread_exit(NULL);
}

int main (int argc, char *argv[]) {
    int i, *idThread;
    double pi = 4, sum = 0;
    pthread_t *threads;

    if (argc < 3) {
        printf("ERROR. Use <number of elements> <number of threads>.\n");
        exit(1);
    }
    n = atoi(argv[1]);
    nThread = atoi(argv[2]);

    sumRes = (double *) malloc(sizeof(double) * nThread);
    threads = (pthread_t *) malloc(sizeof(pthread_t) * nThread);

    for (i = 0; i < nThread; i++) {
        idThread = (int *) malloc(sizeof(int));
        *idThread = i + 1;
        *(sumRes+i) = 0;
        pthread_create(threads+i, NULL, sumF, (void *)idThread);
    }

    for (i = 0; i < nThread; i++) {
        pthread_join(threads[i], NULL);
        sum += sumRes[i];
    }

    pi *= sum;

    printf("PI: %.20lf\tM_PI: %.20lf\n", pi, M_PI);

    return 0;
}
