#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <string.h>
#include "../timer.h"

int nThread;
long long int n;
double *sumRes;

void *sumF(void *args) {
    int idThread;
    idThread = *(int*)args;
    long long int i;

    for (i = idThread; i <= n; i += nThread) {
        sumRes[idThread-1] += (1 / ((double)i * 2 - 1)) * ((i & 1) ? 1 : -1);
    }

    free(args);
    pthread_exit(NULL);
}

double sumSeq() {
    int j;
    long long int i;
    double *localSum, ret = 0;
    localSum = (double *) malloc(sizeof(double) * nThread);

    for (i = 0; i < nThread; i++)
        *(localSum + i) = 0;

    for (i = 1; i <= n; i += nThread) {
        for (j = 0; j < nThread && i + j <= n; j++) {
            *(localSum + j) += (1 / ((double)(i + j) * 2 - 1)) * (((i + j) & 1) ? 1 : -1);
        }
    }

    for (i = 0; i < nThread; i++)
        ret += *(localSum + i);

    return ret;
}

int main (int argc, char *argv[]) {
    int i, *idThread;
    double pi = 4, pi2 = 4, sum = 0, sum2, startTime, endTime;
    pthread_t *threads;

    if (argc < 3) {
        printf("ERROR. Use <number of elements> <number of threads>.\n");
        exit(1);
    }

    n = atoll(argv[1]);
    nThread = atoi(argv[2]);

    sumRes = (double *) malloc(sizeof(double) * nThread);
    threads = (pthread_t *) malloc(sizeof(pthread_t) * nThread);

    GET_TIME(startTime);
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
    GET_TIME(endTime);

    printf("Tempo concorrente: %.10lfs\n", endTime - startTime);

    GET_TIME(startTime);
    sum2 = sumSeq();
    pi2 *= sum2;
    GET_TIME(endTime);

    printf("Tempo sequencial: %.10lfs\n", endTime - startTime);

    printf("PI: %.20lf\tPI2: %.20lf\nM_PI: %.20lf\n", pi, pi2, M_PI);

    return 0;
}
