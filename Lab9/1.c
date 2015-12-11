#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#define TIMES 10

typedef struct _sharedContent {
    int id, counter;
} sharedContent;

int nThread, readCount, writeCount, _flagFinished = 0, _writing, _reading;
sharedContent shared;

pthread_mutex_t readersMutex, writersMutex, globalMutex;
pthread_cond_t theCond;

/*sharedContent read(sharedContent last) {
    if (last.counter == shared.counter)
        pthread_cond_wait(&theCond);
}*/

void* reader(void *args) {
    int i, *id;
    sharedContent local;
    id = (int *) args;
    printf("Initializing reader\n");

    while (_flagFinished < nThread / 2) {
        pthread_mutex_lock(&globalMutex);

        while (_writing)
            pthread_cond_wait(&theCond,&globalMutex);
        while (writeCount)
            pthread_cond_wait(&theCond, &globalMutex);
        readCount++;

        pthread_mutex_unlock(&globalMutex);

        local.counter = shared.counter;
        local.id = shared.id;

        pthread_mutex_lock(&globalMutex);
        readCount--;
        pthread_cond_broadcast(&theCond);
        pthread_mutex_unlock(&globalMutex);

        printf("(%d) - Id:%d, Counter%d\n", *id, local.id, local.counter);
    }

    free(id);

    printf(" (reader)bye\n");
    pthread_exit(NULL);
}

void* writer(void *args) {
    int i, *id, _flag = 0;
    id = (int *) args;
    printf("Initializing writer\n");

    for (i = 0; i < TIMES; i++) {
        pthread_mutex_lock(&globalMutex);
        _writing++;
        while (readCount || writeCount)
            pthread_cond_wait(&theCond, &globalMutex);
        writeCount++;

        shared.counter++;
        shared.id = *id;

        writeCount--;
        _writing--;
        pthread_cond_broadcast(&theCond);
        pthread_mutex_unlock(&globalMutex);
    }

    _flagFinished++;

    free(id);
    printf(" (writer)bye\n");
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    int i, *id;
    pthread_t *threads;

    if (argc < 2) {
        printf("Usage: <threads>\n");
        exit (1);
    }

    nThread = atoi(argv[1]);
    if (nThread < 4) {
        printf("Error. Threads must be equal or greater than 4 (>= 4)\n");
        exit (2);
    }

    pthread_mutex_init(&readersMutex, NULL);
    pthread_mutex_init(&writersMutex, NULL);
    pthread_mutex_init(&globalMutex, NULL);

    printf("Threads: %d\n", nThread);

    threads = (pthread_t *) malloc(sizeof(pthread_t) * nThread);
    shared.counter = 0;
    readCount = 0;
    writeCount = 0;
    _writing = 0;
    _reading = 0;

    for (i = 0; i < nThread; i++) {
        id = (int *) malloc(sizeof(int));
        *id = i + 1;
        if (i < nThread / 2) {
            pthread_create(threads + i, NULL, reader, (void *) id);
        }
        else {
            pthread_create(threads + i, NULL, writer, (void *) id);
        }
    }

    pthread_exit(NULL);
}
