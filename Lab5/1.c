#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int main () {
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
}
