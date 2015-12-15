#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define TIMES 50
#define NTHREADS_SIZE 4

pthread_mutex_t theMutex;
pthread_cond_t theCond;

typedef struct _shared {
    int counter, id;
} sharedContent;

sharedContent shared;
int nThread, lock = 0;

void* writer(void *args) {
    int i, *id;
    id = (int *) args;

    printf("Starting (writer), id: %d\n", *id);

    pthread_cond_wait(&theCond, &theMutex);

    for (i = *id; i < TIMES; i += nThread) {
        pthread_mutex_lock(&theMutex);
        lock = 0;
        shared.counter++;
        shared.id = *id;
        pthread_mutex_unlock(&theMutex);
        lock = 1;
        printf("%d - %d - %d\n", lock, shared.id, shared.counter);
    }

    free(args);

    pthread_mutex_lock(&theMutex);
    lock = -1;
    pthread_mutex_unlock(&theMutex);
    pthread_exit(NULL);
}

void* reader(void *args) {
    int *id;
    id = (int *) args;

    printf("Starting (reader), id: %d\n", *id);

    while (lock != -1)
    {
        printf("tried to read (%d)... ", *id);
        if (lock <= 0)
            continue;
        printf("Im Reading (%d)!: Id: %d - Counter: %d\n", *id, shared.id, shared.counter);
    }
    pthread_exit(NULL);
}

int main(int argc, char const *argv[]) {
    int i, *id;
    pthread_t *threads;

    if (argc < 2) {
        printf("Usage: <threads>\n");
        exit(0);
    }
    shared.counter = 0;
    shared.id = 0;

    nThread = atoi(argv[1]);
    threads = (pthread_t*) malloc(sizeof(pthread_t) * nThread);

    pthread_mutex_init(&theMutex, NULL);
    pthread_cond_init (&theCond, NULL);

    for (i = 0; i < nThread; i++) {
        id = (int *) malloc(sizeof(int));
        *id = i + 1;
        if (i % 2 == 0) {
            pthread_create(threads + i, NULL, writer, (void *) id);
        }
        else {
            pthread_create(threads + i, NULL, reader, (void *) id);
        }
    }
    pthread_exit(NULL);
    return 0;
}
