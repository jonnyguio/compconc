#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void* sumVector(void *args);
float sumVectorSeq();
float sumVectorSeq2();
float compare(float a, float b);
float *vec, *vecAns;
int nThread, tamVec;

int main (int argc, char *argv[]) {
    int i, *idThread;
    float sum = 0, sumSeq, sumSeq2;
    pthread_t *threads;

    if (argc < 3) {
        printf("ERROR. Use <number of threads> <file>.\n");
        exit(1);
    }

    freopen(argv[2], "r", stdin);
    scanf("%d", &tamVec);
    vec = malloc(sizeof(float) * tamVec);
    for (i = 0; i < tamVec; i++) {
        scanf("%f", vec + i);
    }

    nThread = atoi(argv[1]);
    vecAns = (float *) malloc(sizeof(float) * nThread);
    threads = (pthread_t *) malloc(sizeof(pthread_t) * nThread);

    for (i = 0; i < nThread; i++) {
        *(vecAns + i) = 0;
        idThread = malloc(sizeof(int));
        *idThread = i;
        pthread_create(threads+i, NULL, sumVector, (void *) idThread);
    }

    for (i = 0; i < nThread; i++) {
        pthread_join(*(threads+i), NULL);
    }

    for (i = 0; i < nThread; i++) {
        sum += *(vecAns + i);
    }
    sumSeq = sumVectorSeq();
    sumSeq2 = sumVectorSeq2();

    printf("Soma concorrente: \t%f\n", sum);
    printf("Soma sequencial: \t%f\n", sumSeq);
    printf("Soma sequencial2: \t%f\n", sumSeq2);
    return 0;
}

void* sumVector(void *args) {
    int i, *idThread;
    float localSum = 0;

    idThread = (int *) args;

    for (i = *idThread; i < tamVec; i += nThread) {
        localSum += *(vec+i);
    }
    *(vecAns + *idThread) = localSum;

    free(args);
    pthread_exit(NULL);
}

float sumVectorSeq() {
    int i;
    float sum = 0;
    for (i = 0; i < tamVec; i++) {
        sum += *(vec+i);
    }

    return sum;
}

float sumVectorSeq2() {
    int i, j;
    float localSum[2];
    localSum[0] = 0;
    localSum[1] = 0;
    for (j = 0; j < tamVec; j+= nThread) {
        localSum[0] += *(vec + j);
        localSum[1] += *(vec + j + 1);
    }
    return localSum[0] + localSum[1];
}

float compare(float a, float b) {
    return a - b;
}
