#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>

#define MAX_THREADS 8
#define TIMES 10
#define TAM 5

sem_t *fsem;
int in, out, wThreads, rThreads;

static const char *semname1 = "Semaphore1";
int buffer[TAM] = {-1, -1, -1, -1, -1}, end = 0, count = 0;

void printfBuffer(char *string) {
    printf("%s\n", string);
    for (int i = 0; i < TAM; i++) {
        printf("%d, ", buffer[i]);
    }
    printf("\n");
}

void* writer(void *args) {
    int *id, i;
    id = (int *) args;
    printf("comecei produtor\n");
    for (i = 0; i < TIMES; i++) {
        sem_wait(fsem);
        buffer[in] = *id;
        in = (in + 1) % TAM;
        printfBuffer("Inseri...");
        if (i == TIMES - 1)
            end++;
        sem_post(fsem);
    }
    free(id);
    pthread_exit(NULL);
}

void* reader(void *args){
    int *id, i;
    id = (int *) args;
    printf("comecei consumidor\n");
    while (end < MAX_THREADS / 2) {
        sem_wait(fsem);
        if (buffer[out] != - 1) {
            printf("%d\n", buffer[out]);
            buffer[out] = -1;
            out = (out + 1) % TAM;
            printfBuffer("Retirei...");
        }
        sem_post(fsem);
    }
    free(args);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    pthread_t threads[MAX_THREADS];
    int *id, t;

    if (argc < 3) {
        printf("Usage <writer threads> <readers threads>\n");
        exit(1);
    }
    wThreads = atoi(argv[1]);
    rThreads = atoi(argv[2]);

    in = 0;
    out = 0;

    fsem = sem_open(semname1, O_CREAT, 0777, 1);
    if (fsem == SEM_FAILED)
    {
        fprintf(stderr, "%s\n", "ERROR creating semaphore semname1");
        exit(EXIT_FAILURE);
    }

    for (t = 0; t < wThreads + rThreads; t++) {
        if ((id = (int *) malloc(sizeof(int))) == NULL) {
            pthread_exit(NULL);
            return 1;
        }
        *id = t;
        if (i < wThreads) {
            pthread_create(&threads[t], NULL, writer, (void *) id);
        }
        else {
            pthread_create(&threads[t], NULL, reader, (void *) id);
        }
    }
    sem_unlink(semname1);

    pthread_exit(NULL);
    return 0;
}
