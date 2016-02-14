#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "elevator.h"

pthread_t threads[MAX_ELEVATORS];

void init(int N, int M, int C, int floors[MAX_ELEVATORS]) {
    int i;
    params *a;
    for (i = 0; i < M; i++) {
        a = malloc(sizeof(params));
        a->id = i;
        a->floor = floors[i];
        a->capacity = 0;
        pthread_create(&threads[i], NULL, elevator, (void *) a);
        printf("%d\n", a->floor);
    }
}

int main(int argc, char const *argv[]) {

    FILE *arq;
    int floors[MAX_ELEVATORS], i, j, peopleOnFloor, valorLouco;

    arq = fopen("1.in", "r+");

    if (!arq) {
        printf("Error. File not found.\n");
        return 2;
    }

    fscanf(arq, "%d %d %d", &N, &M, &C);
    if (N < 5 || N > 100) {
        printf("Error. Floors only go from 5 to 100.\n");
        return 1;
    }
    if (M < 1 || M > 100) {
        printf("Error. Elevators only go from 1 to 100.\n");
        return 1;
    }
    if (C < 5 || C > 20) {
        printf("Error. Capacity only go from 5 to 20.\n");
        return 1;
    }

    for (i = 0; i < M; i++) {
        fscanf(arq, "%d", &floors[i]);
    }

    floorsReqs = (req *) malloc(sizeof(int *) * N);
    for (i = 0; i < N; i++) {
        floorsReqs[i].inUse = 0;
        floorsReqs[i].size = 0;
        printf("%d ", floorsReqs[i].size);
    }
    printf("\n");

    floorsMutex = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t) * N);
    for (i = 0; i < N; i++) {
        pthread_mutex_init(&floorsMutex[i], NULL);
    }

    //init(N, M, C, floors);

    for (i = 0; i < N; i++) {
        pthread_mutex_lock(&floorsMutex[i]);
        fscanf(arq, "%d", &peopleOnFloor);
        printf("PeopleOnFloor(%d): %d\n", i, peopleOnFloor);
        floorsReqs[i].size = peopleOnFloor;
        initQueue(&floorsReqs[i].people);
        for (j = 0; j < peopleOnFloor; j++) {
            fscanf(arq, "%d", &valorLouco);
            insertQ(&floorsReqs[i].people, valorLouco);
            printf("%d ", valorLouco);
        }
        printf("\n");
        printQueue(&floorsReqs[0].people);
        pthread_mutex_unlock(&floorsMutex[i]);
    }

    printf("\n\n");

    for (i = 0; i < N; i++) {
        printf("%d\t", i, &floorsReqs[i].people);
        printQueue(&floorsReqs[i].people);
    }

    free(floorsReqs);
    free(floorsMutex);

    fclose(arq);

    //pthread_exit(NULL);

    return 0;
}
