#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include "elevator.h"

int N = 0; int M = 0; int C = 0; int finishedInputs = 0; req *floorsReqs = (req *) NULL;

int isInRange(int n, int i, int f) {
    return n >= i && n <= f;
}

void initQueue(queue *q) {
    q->size = 0;
}

void insertQ(queue *q, int val) {
    if (q->size + 1 < MAX_PEOPLE) {
        q->array[q->size] = val;
        q->size++;
    }
    else {
        if (TAG_DEBUG) printf("Queue is full\n");
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
    if (TAG_DEBUG) printf("(%d) iniciando nova busca\n", id);
    found = 0;
    while ((isInRange(testFloor + i, 0, N) || isInRange(testFloor - i, 0, N)) && !found) {
        if (!i && isInRange(testFloor + i, 0, N)) {
            if (TAG_DEBUG) printf("(%d) Gettin lock of: %d\n", id, testFloor);
            pthread_mutex_lock(&floorsMutex[testFloor]);
            if (!floorsReqs[testFloor].inUse && floorsReqs[testFloor].size > 0) {
                if (TAG_DEBUG) printf("size%d: %d\n",testFloor, floorsReqs[testFloor].size);
                floorsReqs[testFloor].inUse = 1;
                found = 1;
            }
            pthread_mutex_unlock(&floorsMutex[testFloor]);
            if (TAG_DEBUG) printf("(%d) Dropped lock of: %d\n", id, testFloor);
        }
        else {
            size1 = 0;
            size2 = 0;
            if (isInRange(testFloor + i, 0, N)) {
                if (TAG_DEBUG) printf("(%d) :Gettin lock of: %d\n", id, testFloor + i);
                pthread_mutex_lock(&floorsMutex[testFloor + i]);
                if (!floorsReqs[testFloor + i].inUse) {
                    size1 = floorsReqs[testFloor + i].size;
                }
            }
            if (isInRange(testFloor - i, 0, N)) {
                if (TAG_DEBUG) printf("(%d) /Gettin lock of: %d\n", id, testFloor - i);
                pthread_mutex_lock(&floorsMutex[testFloor - i]);
                if (!floorsReqs[testFloor - i].inUse) {
                    size2 = floorsReqs[testFloor - i].size;
                }
            }
            if (TAG_DEBUG) printf("(%d) %d > %d?\n", id, size1, size2);
            if (size1 > size2) {
                found = 1;
                result = testFloor + i;
                floorsReqs[result].inUse = 1;
            }
            else {
                if (size1 < size2) {
                    found = 1;
                    result = testFloor - i;
                    floorsReqs[result].inUse = 1;
                }
            }
            if (isInRange(testFloor - i, 0, N)) {
                pthread_mutex_unlock(&floorsMutex[testFloor - i]);
                if (TAG_DEBUG) printf("(%d) /Dropped lock of: %d\n", id, testFloor - i);
            }
            if (isInRange(testFloor + i, 0, N)) {
                pthread_mutex_unlock(&floorsMutex[testFloor + i]);
                if (TAG_DEBUG) printf("(%d) :Dropped lock of: %d\n", id, testFloor + i);
            }
            if (found)
                testFloor = result;
        }
        i++;
    }
    if (!found) {
        if (TAG_DEBUG) printf("(%d) Not found.\n", id);
        testFloor = -1;
    }
    return testFloor;
}

void closerSort(int teste[MAX_PEOPLE], int lenght, int start) {
    int i, j, aux, distance[MAX_PEOPLE];
    for (i = 0; i < lenght; i++)
        distance[i] = abs(teste[i] - start);
    for (i = lenght - 1; i >= 0; i--) {
        for (j = 0; j < i; j++) {
            if (distance[j] > distance[j + 1]) {
                aux = distance[j];
                distance[j] = distance[j + 1];
                distance[j + 1] = aux;
                aux = teste[j];
                teste[j] = teste[j + 1];
                teste[j + 1] = aux;
            }
        }
    }
}

void* elevator(void* args) {
    FILE *fElevator;
    params *p;
    int targetFloor, path[MAX_PEOPLE], i, j;
    char buffer[40];

    p = (params *) args;

    printf("Thread: %d\tFloor: %d\n", p->id, p->floor);

    snprintf(buffer, sizeof(char) * 40, "outputs/elevator%d.txt", p->id);
    fElevator = fopen(buffer, "w+");
    fprintf(fElevator, "%s\n", buffer);
    printf("(%d) começar sa porra\n", p->id);
    targetFloor = getFloorFree(p->id, p->floor);
    while (targetFloor != -1 || !finishedInputs) {
        if (targetFloor != -1) {
            fprintf(fElevator, "Vou para: %d\n", targetFloor);
            while (p->capacity < C && floorsReqs[targetFloor].size > 0) {
                floorsReqs[targetFloor].size--;
                path[p->capacity] = removeQ(&floorsReqs[targetFloor].people);
                p->capacity++;
            }
            if (TAG_DEBUG) printf("(%d) -Gettin lock of: %d\n", p->id, targetFloor);
            pthread_mutex_lock(&floorsMutex[targetFloor]);
            floorsReqs[targetFloor].inUse = 0;
            pthread_mutex_unlock(&floorsMutex[targetFloor]);
            if (TAG_DEBUG) printf("(%d) -Dropped lock of: %d\n", p->id, targetFloor);

            closerSort(path, p->capacity, targetFloor);

            if (TAG_DEBUG) { for (i = 0; i < p->capacity; i++) printf("%d ", path[i]); printf("\n"); }

            while (p->capacity > 0) {
                i = 0;
                p->floor = path[0];
                while (p->floor == path[0] && p->capacity - i > 0) {
                    for (j = 0; j < p->capacity; j++) {
                        path[j] = path[j + 1];
                    }
                    i++;
                }
                fprintf(fElevator, "Deixei %d pessoas no andar %d.\n", i, p->floor);
                p->capacity -= i;
            }
        }
        targetFloor = getFloorFree(p->id, p->floor);
    }
    printf("(%d) acabou né\n", p->id);

    fclose(fElevator);
    free(p);

    pthread_exit(NULL);
}
