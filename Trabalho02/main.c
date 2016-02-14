#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "elevator.h"

pthread_t threads[MAX_ELEVATORS];
pthread_mutex_t floorsMutex[MAX_FLOORS];
req floorsReqs[MAX_FLOORS];

int main(int argc, char const *argv[]) {

    FILE *arq;
    params *p;
    int elevatorsFloors[MAX_ELEVATORS], i, j, peopleOnFloor, valorLouco, inputFileNumber;
    char buffer[21];

    if (argc < 2) {
        printf("Usage: <input file number>\n");
        return 4;
    }
    inputFileNumber = atoi(argv[1]);

    snprintf(buffer, sizeof(char) * 20, "inputs/%d.in", inputFileNumber);
    arq = fopen(buffer, "r+");
    printf("%s\n", buffer);

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

    printf("Floors: %d\tElevators: %d\tCapacity: %d\n", N, M, C);

    for (i = 0; i < M; i++) {
        fscanf(arq, "%d", &elevatorsFloors[i]);
    }

    for (i = 0; i < N; i++) {
        floorsReqs[i].inUse = 0;
        floorsReqs[i].size = 0;
    }

    for (i = 0; i < N; i++) {
        pthread_mutex_init(&floorsMutex[i], NULL);
    }

    pthread_mutex_init(&teste, NULL);

    for (i = 0; i < M; i++) {
        p = (params *) malloc(sizeof(params));
        p->id = i;
        p->f = elevatorsFloors[i];
        p->capacity = 0;
        pthread_create(&threads[i], NULL, elevator, (void *) p);
    }

    for (i = 0; i < N; i++) {
        pthread_mutex_lock(&floorsMutex[i]);
        fscanf(arq, "%d", &peopleOnFloor);
        printf("PeopleOnFloor(%d): %d\n", i, peopleOnFloor);
        for (j = 0; j < peopleOnFloor; j++) {
            fscanf(arq, "%d", &valorLouco);
            insertQ(&floorsReqs[i], valorLouco);
        }
        if (TAG_DEBUG) printReq(&floorsReqs[i]);
        pthread_mutex_unlock(&floorsMutex[i]);
    }

//    free(floorsReqs);
//    free(floorsMutex);

    finishedInputs = 1;
    fclose(arq);

    pthread_exit(NULL);

}
