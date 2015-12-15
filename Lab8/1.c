#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define SIZE 5
#define NTHREADS_SIZE 4

pthread_mutex_t theMutex;
pthread_cond_t theCond;

typedef struct _buffer {
    int val;
    struct _buffer *next;
} buffer;

buffer *beginBuf;

void* producer(void *args);
void* consumer(void *args);
void insert(int n);
int get();
int getSize(buffer *buf);
int isPrime(long unsigned int n);

void printBuf(buffer *buf) {
    buffer *aux;
    int i;
    for (aux = buf, i =0; aux != NULL; aux = aux->next, i++)
        printf("[%d] - Valor: %d\n", i, aux->val);
    printf("\n");
}

int main(int argc, char const *argv[]) {
    int i, *id;

    pthread_t threads[NTHREADS_SIZE];
    beginBuf = NULL;
    pthread_mutex_init(&theMutex, NULL);
    pthread_cond_init (&theCond, NULL);

    for (i = 0; i < NTHREADS_SIZE; i++) {
        id = (int *) malloc(sizeof(int));
        if (!i) {
            *id = i;
            pthread_create(&threads[i], NULL, producer, (void *) id);
        }
        else {
            *id = i - 1;
            pthread_create(&threads[i], NULL, consumer, (void *) id);
        }
    }
    pthread_exit(NULL);
    return 0;
}

int getSize(buffer *buf) { // Receives begin of the buffer
    int counter = 0;
    buffer *aux;
    aux = buf;
    while (aux != NULL)
    {
        counter++;
        aux = aux->next;
    }
    return counter;
}

void insert(n) {
    buffer *aux;
    pthread_mutex_lock(&theMutex);

    while (getSize(beginBuf) > SIZE) {
        pthread_cond_wait(&theCond, &theMutex);
    }

    if (beginBuf != NULL) {
        for (aux = beginBuf; aux->next != NULL; aux = aux->next);
        aux->next = (buffer *) malloc(sizeof(buffer));
        aux->next->val = n;
        aux->next->next = NULL;
    }
    else {
        beginBuf = (buffer *) malloc(sizeof(buffer));
        beginBuf->next = NULL;
        beginBuf->val = n;
    }

    //printBuf(beginBuf);

    pthread_cond_signal(&theCond);
    pthread_mutex_unlock(&theMutex);
}

int get() {
    int val;
    buffer *aux;

    pthread_mutex_lock(&theMutex);
    while (getSize(beginBuf) == 0)
        pthread_cond_wait(&theCond, &theMutex);
    val = beginBuf->val;
    aux = beginBuf->next;
    free(beginBuf);
    beginBuf = aux;

    pthread_cond_signal(&theCond);
    pthread_mutex_unlock(&theMutex);

    return val;
}

void* producer(void *args) {
    int n[3] = {0, 1, 1}, i, *myId;
    insert(n[1]);
    insert(n[2]);
    myId = (int *) args;
    for (i = 3; i <= 25; i++)
    {
        n[0] = n[1] + n[2];
        insert(n[0]);

        n[1] = n[2];
        n[2] = n[0];
    }
    printf("%d (Producer) - exiting...\n", *myId);

    /*pthread_mutex_lock(&theMutex);
    pthread_cond_broadcast(&theCond);
    pthread_mutex_unlock(&theMutex);*/

    pthread_exit(NULL);
}

void* consumer(void *args) {
    int n, i, *myId;
    myId = (int *) args;
    //printf("myId: %d, jump:%d\n", *myId, NTHREADS_SIZE - 1);
    for (i = *myId; i < 25; i += NTHREADS_SIZE - 1) {
        n = get();
        printf("(%d) - %d: %s\n", i, n, (isPrime(n))?"is prime":"is NOT prime");
    }
    free(myId);
    printf("%d (Consumer) - exiting...\n", *myId);
    pthread_exit(NULL);
}

int isPrime(long unsigned int n) {
    int i;
    if (n <= 1) return 0;
    if (n == 2) return 1;
    if (n%2 == 0) return 0;
    for(i = 3; i < sqrt(n) + 1; i += 2) {
        if (n % i == 0)
            return 0;
    }
    return 1;
}
