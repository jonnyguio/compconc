#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include "elevator.h"

int N = 0; int M = 0; int C = 0; int finishedInputs = 0;

pthread_mutex_t teste;

// FUNÇÃO SIMPLES PARA SE NUMERO ESTA DENTRO DE UM RANGE
int isInRange(int n, int i, int f) {
    return n >= i && n <= f;
}

// FUNÇÃO DE INSERÇÃO NA FILA
void insertQ(req *q, int val) {
    if (q->size + 1 < MAX_PEOPLE) {
        q->people[q->size] = val;
        q->size++;
    }
    else {
        if (TAG_DEBUG) printf("Queue is full\n");
    }
}

// FUNÇÃO DE IMPRESSÃO DA FILA
void printQueue(req *q) {
    int i;
    for (i = 0; i < q->size; i++) {
        printf("%d ", q->people[i]);
    }
    printf("\n");
}

// REARRUMA A FILA APÓS UMA RETIRADA
void reorder(req *q) {
    int i = 0;
    for (i = 0; i < q->size; i++) {
        q->people[i] = q->people[i + 1];
    }
}

// REMOVE O TOPO DA FILA (SENÃO RETORNA -1)
int removeQ(req *q) {
    int val;
    if (q->size > 0) {
        q->size--;
        val = q->people[0];
        reorder(q);
    }
    else {
        printf("uer\n");
        val = -1;
    }
    return val;
}

// FUNÇÃO FEITA PARA PEGAR QUAL O PRÓXIMO ANDAR QUE O ELEVADOR DEVE IR
int getFloorFree(int id, int testFloor) {
    int i = 0, found = 0, size1, size2, result;
    if (TAG_DEBUG) printf("(%d) iniciando nova busca\n", id);
    found = 0;

    // Enquanto não encontrar, ou ainda tiver andares a serem procurados
    while ((isInRange(testFloor + i, 0, N - 1) || isInRange(testFloor - i, 0, N - 1)) && !found) {
        // Checa o próprio andar
        if (!i && isInRange(testFloor + i, 0, N - 1)) {
            if (TAG_DEBUG) printf("(%d) Gettin lock of: %d\n", id, testFloor);

            pthread_mutex_lock(&floorsMutex[testFloor]); // pega o mutex para acessar varíavel (inUse) por exclusão mutua
            if (!floorsReqs[testFloor].inUse && floorsReqs[testFloor].size > 0) {
                if (TAG_DEBUG) printf("size%d: %d\n",testFloor, floorsReqs[testFloor].size);
                floorsReqs[testFloor].inUse = 1;
                found = 1;
            }
            pthread_mutex_unlock(&floorsMutex[testFloor]);
            if (TAG_DEBUG) printf("(%d) Dropped lock of: %d\n", id, testFloor);
        }
        else {
            // Nessa parte, a função checa os andares acima e abaixo do elevador (se elas existem)
            // Assim, ele seleciona a que não possue elevador lá e que possui + pessoas esperando
            size1 = 0;
            size2 = 0;
            if (isInRange(testFloor + i, 0, N - 1)) {
                if (TAG_DEBUG) printf("(%d) :Gettin lock of: %d\n", id, testFloor + i);
                pthread_mutex_lock(&floorsMutex[testFloor + i]);
                if (!floorsReqs[testFloor + i].inUse) {
                    size1 = floorsReqs[testFloor + i].size;
                }
            }
            if (isInRange(testFloor - i, 0, N - 1)) {
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
            if (isInRange(testFloor - i, 0, N - 1)) {
                pthread_mutex_unlock(&floorsMutex[testFloor - i]);
                if (TAG_DEBUG) printf("(%d) /Dropped lock of: %d\n", id, testFloor - i);
            }
            if (isInRange(testFloor + i, 0, N - 1)) {
                pthread_mutex_unlock(&floorsMutex[testFloor + i]);
                if (TAG_DEBUG) printf("(%d) :Dropped lock of: %d\n", id, testFloor + i);
            }
            if (found)
                testFloor = result;
        }
        i++;
    }
    if (!found) {
        // Se não achou, força resultado para "-1"
        if (TAG_DEBUG) printf("(%d) Not found.\n", id);
        testFloor = -1;
    }
    return testFloor;
}

// Bubble sort, porém baseado na distância de um numero (no caso, do numer 'start')
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

// Função da thread
void* elevator(void* args) {

    FILE *fElevator;
    params *p;

    int path[MAX_PEOPLE], targetFloor, i, j, sn;
    char buffer[41];

    p = (params *) args;

    if (TAG_DEBUG) printf("Thread: %d\tFloor: %d\n", p->id, p->f);

    // Inicialização de arquivos
    sn = snprintf(buffer, sizeof(char) * 41, "outputs/elevator%d.txt", p->id);
    fElevator = fopen(buffer, "w");
    fprintf(fElevator, "%s\n", buffer);

    if (TAG_DEBUG) printf("(%d) começar sa bobba\n", p->id);

    targetFloor = getFloorFree(p->id, p->f); // pega qual andar deve ir
    while (targetFloor != -1 || !finishedInputs) { // Enquanto não há requisições e a thread principal não acabou

        if (isInRange(targetFloor, 0, N - 1)) { // Se o andar fornecido for possível
            fprintf(fElevator, "Vou para: %d\n", targetFloor);

            while (p->capacity < C && floorsReqs[targetFloor].size > 0) { // Pega gente até capacidade ou até acabar
                path[p->capacity] = removeQ(&floorsReqs[targetFloor]);
                p->capacity++;
            }

            if (TAG_DEBUG) printf("(%d) -Gettin lock of: %d\n", p->id, targetFloor);
            pthread_mutex_lock(&floorsMutex[targetFloor]);
            floorsReqs[targetFloor].inUse = 0; // avisa que deixou de usar aquele andar
            pthread_mutex_unlock(&floorsMutex[targetFloor]);
            if (TAG_DEBUG) printf("g(%d) -Dropped lock of: %d\n", p->id, targetFloor);

            //closerSort(path, p->capacity, targetFloor);
            if (TAG_DEBUG) { for (i = 0; i < p->capacity; i++) printf("%d ", path[i]); printf("\n"); }

            while (p->capacity > 0) { // Caminha atualizando a cada passo qual o próximo andar que ele deve ir.
                closerSort(path, p->capacity, p->f);
                i = 0;
                p->f = path[0];
                while (p->f == path[0] && p->capacity - i > 0) {
                    for (j = 0; j < p->capacity; j++) {
                        path[j] = path[j + 1];
                    }
                    i++;
                }
                fprintf(fElevator, "Deixei %d pessoas no andar %d.\n", i, p->f);
                p->capacity -= i;
            }
        }
        targetFloor = getFloorFree(p->id, p->f);
    }
    if (TAG_DEBUG) printf("(%d) Thread ended\n", p->id);

    fclose(fElevator);
    free(p);

    pthread_exit(NULL);
}
