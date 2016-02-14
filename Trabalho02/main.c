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
    int floors[MAX_ELEVATORS], i, j, peopleOnFloor, valorLouco, inputFileNumber;
    char buffer[10];

    if (argc < 2) {
        printf("Usage: <input file number>\n");
        return 4;
    }
    inputFileNumber = atoi(argv[1]) | 1;

    snprintf(buffer, sizeof(char) * 10, "%d.in", inputFileNumber);
    arq = fopen(buffer, "r+");

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
    }

    for (i = 0; i < N; i++) {
        printf("%p\n", &floorsMutex[i]);
        pthread_mutex_init(&floorsMutex[i], NULL);
    }

    init(N, M, C, floors);

    for (i = 0; i < N; i++) {
        pthread_mutex_lock(&floorsMutex[i]);
        fscanf(arq, "%d", &peopleOnFloor);
        printf("PeopleOnFloor(%d): %d\n", i, peopleOnFloor);
        floorsReqs[i].size = peopleOnFloor;
        initQueue(&floorsReqs[i].people);
        for (j = 0; j < peopleOnFloor; j++) {
            fscanf(arq, "%d", &valorLouco);
            insertQ(&floorsReqs[i].people, valorLouco);
        }
        pthread_mutex_unlock(&floorsMutex[i]);
    }

//    free(floorsReqs);
//    free(floorsMutex);

    finishedInputs = 1;
    fclose(arq);

    pthread_exit(NULL);

}
