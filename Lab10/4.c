#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>

#define NTHREADS 4

sem_t fsem;

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

}

void* consumer(void *args){

}

int main(int argc, char *argv[]) {

}
