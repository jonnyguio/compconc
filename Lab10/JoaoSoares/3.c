#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>

#define NTHREADS 4

sem_t fcond, scond;
int g = 0;

void* t1(void *args) {
    printf("hello!\n");
    sem_wait(&fcond);
    if (++g > 1) {
        sem_post(&scond);
        sem_post(&scond);
    }
    else {
        sem_post(&fcond);
    }
    pthread_exit(NULL);
}

void* t2(void *args) {
    printf("ola, tudo bem?\n");
    sem_wait(&fcond);
    if (++g > 1) {
        sem_post(&scond);
        sem_post(&scond);
    }
    else {
        sem_post(&fcond);
    }
    pthread_exit(NULL);
}

void* t3(void *args) {
    /*int g;
    do {
        sem_getvalue(&scond, &g);
    } while (g < 2);*/
    sem_wait(&scond);
    printf("até mais tarde\n");
    pthread_exit(NULL);
}

void* t4(void *args) {
    /*int g;
    do {
        sem_getvalue(&scond, &g);
    } while (g < 2);*/
    sem_wait(&scond);
    printf("tchau!\n");
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    pthread_t threads[NTHREADS];
    int *id, t;

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
