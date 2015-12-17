#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>

#define NTHREADS 4

sem_t fsem;
int in, out;

typedef struct queue {
    int val;
    struct queue *next
} _queue;

int buffer[5];

void printfBuffer() {
    for (int i = 0; i < 5; i++) {
        printf("%d, ", buffer[i]);
    }
    printf("\n");
}

void* producer(void *args){
    sem_wait(fsem);

}

void* consumer(void *args){

}

int main(int argc, char *argv[]) {
    pthread_t threads[NTHREADS];
    int *id, t;

    in = 0;
    out = 0;

    sem_init(&fcond, 0, 1);
    sem_init(&scond, 0, 0);

    for (t = 0; t < NTHREADS; t++) {
        if ((id = (int *) malloc(sizeof(int))) == NULL) {
            pthread_exit(NULL);
            return 1;
        }
        if (t == 0)
            pthread_create(&threads[t], NULL, t1, (void *) id);
        if (t == 1)
            pthread_create(&threads[t], NULL, t2, (void *) id);
        if (t == 2)
            pthread_create(&threads[t], NULL, t3, (void *) id);
        if (t == 3)
            pthread_create(&threads[t], NULL, t4, (void *) id);
    }

    pthread_exit(NULL);
    return 0;
}
