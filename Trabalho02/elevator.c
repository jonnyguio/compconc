#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "elevator.h"

int N = 0; int M = 0; int C = 0; req *floorsReqs = (req *) NULL;
pthread_mutex_t *floorsMutex = NULL;

void initQueue(queue *q) {
    q->size = 0;
}

void insertQ(queue *q, int val) {
    if (q->size + 1 < MAX_CAPACITY) {
        q->array[q->size] = val;
        q->size++;
    }
    else {
        printf("Queue is full\n");
    }
}

void printQueue(queue *q) {
    int i;
    for (i = 0; i < q->size; i++) {
        printf("%d ", q->array[i]);
    }
    printf("\n");
}

int getFloorFree(int testFloor) {
    int i = 0, found = 0, size1, size2;
    while ((testFloor + i < N || testFloor - i > 0) && !found) {
        if (!i) {
            pthread_mutex_lock(&floorsMutex[testFloor]);
            if (!floorsReqs[testFloor].inUse && floorsReqs[testFloor].size > 0) {
                printf("size: %d\n", floorsReqs[testFloor].size);
                floorsReqs[testFloor].inUse = 1;
                found = 1;
            }
            pthread_mutex_unlock(&floorsMutex[testFloor]);
        }
        else {
            pthread_mutex_lock(&floorsMutex[testFloor + i]);
            pthread_mutex_lock(&floorsMutex[testFloor - i]);
            size1 = 0;
            size2 = 0;
            if (!floorsReqs[testFloor + i].inUse) {
                size1 = floorsReqs[testFloor + i].size;
            }
            if (!floorsReqs[testFloor - i].inUse) {
                size2 = floorsReqs[testFloor - i].size;
            }
            if (size1 > size2) {
                found = 1;
                testFloor = testFloor + i;
            }
            else {
                if (size1 < size2) {
                    found = 1;
                    testFloor = testFloor - i;
                }
            }
            pthread_mutex_unlock(&floorsMutex[testFloor + i]);
            pthread_mutex_unlock(&floorsMutex[testFloor - i]);
        }
        i++;
    }
    return testFloor;
}

void insertionSort(int *array, int lenght) {
    int i, j, now;
    for (i = 0; i < lenght; i++) {
        now = array[i];
        j = i - 1;
        while ((j >= 0) && (now < array[j])) {
            array[j + 1] = array[j];
            j--;
        }
        array[j + 1] = now;
    }
}

void* elevator(void* args) {
    params *p;
    int targetFloor, path[MAX_CAPACITY], i;

    p = (params *) args;

    printf("Thread: %d\nFloor: %d\n", p->id, p->floor);
    targetFloor = getFloorFree(p->floor);

    if (targetFloor != -1) {
        printf("(%d) Vou para: %d\n", p->id, targetFloor);
/*        printQueue(floorsReqs[targetFloor].people);
        while (p->capacity < C) {
            floorsReqs[targetFloor].size--;
            path[p->capacity] = removeQ(floorsReqs[targetFloor].people);
            printf("%d ", path[p->capacity]);
            p->capacity++;
        }
        printf("\n");
        printQueue(floorsReqs[targetFloor].people);
        pthread_mutex_lock(&floorsMutex[targetFloor]);
        floorsReqs[targetFloor].inUse = 0;
        pthread_mutex_unlock(&floorsMutex[targetFloor]);

        insertionSort(path, floorsReqs[targetFloor].size);

        for (i = 0; i < floorsReqs[targetFloor].size; i++) {
            printf("%d ", path[i]);
        }
        printf("\n");;*/
    }
    free(p);
    pthread_exit(NULL);
}
