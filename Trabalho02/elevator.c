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

void reorder(queue *q) {
    int i = 0;
    for (i = 0; i < q->size; i++) {
        q->array[i] = q->array[i + 1];
    }
}

int removeQ(queue *q) {
    int val;
    if (q->size > 0) {
        q->size--;
        val = q->array[0];
        reorder(q);
    }
    else {
        val = -1;
    }
    return val;
}

int getFloorFree(int id, int testFloor) {
    int i = 0, found = 0, size1, size2, result;
    printf("(%d) iniciando nova busca\n", id);
    while ((testFloor + i < N || testFloor - i > 0) && !found) {
        if (!i) {
            if (TAG_DEBUG) printf("(%d) Gettin lock of: %d\n", id, testFloor);
            pthread_mutex_lock(&floorsMutex[testFloor]);
            if (!floorsReqs[testFloor].inUse && floorsReqs[testFloor].size > 0) {
                printf("size%d: %d\n",testFloor, floorsReqs[testFloor].size);
                floorsReqs[testFloor].inUse = 1;
                found = 1;
            }
            pthread_mutex_unlock(&floorsMutex[testFloor]);
            if (TAG_DEBUG) printf("(%d) Dropped lock of: %d\n", id, testFloor);
        }
        else {
            size1 = 0;
            size2 = 0;
            if (testFloor + i < N) {
                if (TAG_DEBUG) printf("(%d) :Gettin lock of: %d\n", id, testFloor + i);
                pthread_mutex_lock(&floorsMutex[testFloor + i]);
                if (!floorsReqs[testFloor + i].inUse) {
                    size1 = floorsReqs[testFloor + i].size;
                }
            }
            if (testFloor - i > 0) {
                if (TAG_DEBUG) printf("(%d) /Gettin lock of: %d\n", id, testFloor - i);
                pthread_mutex_lock(&floorsMutex[testFloor - i]);
                if (!floorsReqs[testFloor - i].inUse) {
                    size2 = floorsReqs[testFloor - i].size;
                }
            }
            if (size1 > size2) {
                found = 1;
                result = testFloor + i;
                floorsReqs[testFloor].inUse = 1;
            }
            else {
                if (size1 < size2) {
                    found = 1;
                    result = testFloor - i;
                    floorsReqs[testFloor].inUse = 1;
                }
            }
            if (testFloor - i > 0) {
                pthread_mutex_unlock(&floorsMutex[testFloor - i]);
                if (TAG_DEBUG) printf("(%d) /Dropped lock of: %d\n", id, testFloor - i);
            }
            if (testFloor + i < N) {
                pthread_mutex_unlock(&floorsMutex[testFloor + i]);
                if (TAG_DEBUG) printf("(%d) :Dropped lock of: %d\n", id, testFloor + i);
            }
            if (found)
                testFloor = result;
        }
        i++;
    }
    if (!found)
        testFloor = -1;
    return testFloor;
}

void closerSort(int array[MAX_CAPACITY], int lenght, int start) {
    int i, j, c2 = 0, c = 0, aux;
    for (i = 0; i < lenght; i++) {
        c = 0;
        for (j = 0; j < lenght; j++) {
            if (array[i] == start + c || array[i] == start - c) {
                aux = array[c2];
                array[c2] = array[i];
                array[i] = aux;
                c2++;
            }
            c++;
        }
    }
}

void* elevator(void* args) {
    params *p;
    int targetFloor, path[MAX_CAPACITY], i, j;

    p = (params *) args;

    printf("Thread: %d\tFloor: %d\n", p->id, p->floor);
    targetFloor = getFloorFree(p->id, p->floor);

    while (targetFloor != -1) {
        printf("(%d) Vou para: %d (%d/%d)\n", p->id, targetFloor, p->capacity, C);
        while (p->capacity < C && floorsReqs[targetFloor].size > 0) {
            floorsReqs[targetFloor].size--;
            path[p->capacity] = removeQ(&floorsReqs[targetFloor].people);
            //if (TAG_DEBUG) printf("(%d): %d\n", p->id, path[p->capacity]);
            p->capacity++;
        }
        if (TAG_DEBUG) printf("(%d) -Gettin lock of: %d\n", p->id, targetFloor);
        pthread_mutex_lock(&floorsMutex[targetFloor]);
        floorsReqs[targetFloor].inUse = 0;
        pthread_mutex_unlock(&floorsMutex[targetFloor]);
        if (TAG_DEBUG) printf("(%d) -Dropped lock of: %d\n", p->id, targetFloor);

        closerSort((int *) path, floorsReqs[targetFloor].size, targetFloor);

        if (TAG_DEBUG) for (i = 0; i < p->capacity; i++) printf("%d ", path[i]);
        printf("\n");

        while (p->capacity > 0) {
            i = 0;
            p->floor = path[0];
            while (p->floor == path[0]) {
                for (j = 0; j < p->capacity; j++) {
                    path[j] = path[j + 1];
                }
                i++;
            }
            printf("(%d) Deixei %d pessoas no andar %d.\n", p->id, i, p->floor);
            p->capacity--;
        }
        printf("\n");
        targetFloor = getFloorFree(p->id, p->floor);
    }
    free(p);
    pthread_exit(NULL);
}
