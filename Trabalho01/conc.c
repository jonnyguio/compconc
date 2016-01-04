#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include "../timer.h"

double func1(double x){return 1+x;}
double func2(double x){return pow ((1 - pow((x), 2)), 0.5);}
double func3(double x){return pow ((1 + pow((x), 4)), 0.5);}

double (*funcList[3])(double) = {func1, func2, func3};

//#define func(op, x) ((op) == 'a') ? 1 + (x) : ((op) == 'c') ? pow ((1 + pow((x), 4)), 0.5) : (x > -1 || x < 1) ? pow ((1 - pow((x), 2)), 0.5) : 0

#define MAX_THREADS 8
#define TAM 1000000
#define outpp (out + 1 < 1000) ? out + 1 : 0

#define getMiddle(a, b) ((a) + (b)) / 2
/*
typedef struct _queue {
    float val;
    struct _queue *next;
} queue;
typedef struct _stack {
    float val;
    struct _stack *next;
} stack;
queue *coordinates;
stack *results;
*/

typedef struct _params {
    int id;
    float err;
    double (*func)(double);
} params;

double inputs[TAM], output, begin, end;
int in, out, size;

pthread_mutex_t theMutex, stackMutex;
pthread_cond_t theCond;

int usingQueue = 0, doneMath = 0;
/*
double sumAllStack(stack *s) {
    double res = 0;
    for (; s != NULL; s = s->next)
        res += s->val;
    return res;
}
double popStack(stack **s) {
    double ret;
    ret = (*s)->val;
    (*s) = (*s)->next;
    return ret;
}
void pushStack(stack **s, float val) {
    stack *aux;
    aux = (stack *) malloc(sizeof(stack));
    aux->val = val;
    aux->next = (*s);
    (*s) = aux;
}
double popQueue(queue **q) {
    double ret;
    queue *aux;
    aux = *q;
    ret = (*q)->val;
    *q = (*q)->next;
    free(aux);
    return ret;
}
void pushQueue(queue **q, float val) {
    queue *aux;
    if (*q == NULL) {
        (*q) = (queue *) malloc(sizeof(queue));
        (*q)->val = val;
        (*q)->next = NULL;
    }
    else {
        for (aux = *q; aux->next != NULL; aux = aux->next);
        aux->next = (queue *) malloc(sizeof(queue));
        aux->next->val = val;
        aux->next->next = NULL;
    }
}
void print(queue *q, stack *s) {
    queue *qaux;
    stack *saux;
    printf("Queue:\n\t");
    for (qaux = q; qaux != NULL; qaux = qaux->next) {
        printf("%lf ", qaux->val);
    }
    printf("\nStack:\n\t");
    for (saux = s; saux != NULL; saux = saux->next)
        printf("%lf ", saux->val);
    printf("\n");
}
*/

void adaptativeQuadrature(int id, double (*func)(double), double a, double b, double err) {
    double m, funcB, funcSleft, funcSright, areaS1, areaS2, areaB;
    int counter = 0;
    m = getMiddle(a, b);
    funcB = func(m);
    funcSleft = func(getMiddle(a, m));
    funcSright = func(getMiddle(m, b));
    areaB = (b - a) * funcB;
    //printf("(%.20lf - %.20lf) * %.20lf = %.20lf\n", b, a, funcB, areaB);
    areaS1 = (m - a) * funcSleft;
    //printf("(%.20lf - %.20lf) * %.20lf = %.20lf\n", m, a, funcSleft, areaS1);
    areaS2 = (b - m) * funcSright;
    //printf("(%.20lf - %.20lf) * %.20lf = %.20lf\n", b, m, funcSright, areaS2);
    //printf("%.20lf, %.20lf\n", fabs(areaB - (areaS1 + areaS2)), err);
    if (fabs(areaB - (areaS1 + areaS2)) <= err) {
        pthread_mutex_lock(&stackMutex);
        output += areaB;
        pthread_mutex_unlock(&stackMutex);
        pthread_mutex_lock(&theMutex);
        usingQueue--;
        pthread_mutex_unlock(&theMutex);
    }
    else {
        //printf("(%d) parei aqui?\n", id);
        pthread_mutex_lock(&theMutex);
        while (size + 4 > TAM) {
            printf("(%d) estorou - %d\n", id, ++counter);
            pthread_cond_wait(&theCond, &theMutex);
        }
        inputs[in] = a;
        in = (in + 1 < TAM) ? in + 1 : 0;
        inputs[in] = m;
        in = (in + 1 < TAM) ? in + 1 : 0;
        inputs[in] = m;
        in = (in + 1 < TAM) ? in + 1 : 0;
        inputs[in] = b;
        in = (in + 1 < TAM) ? in + 1 : 0;
        size += 4;
        //printf("%.10lf, %.10lf (2x), %.10lf\n", a,m,b);
        usingQueue--;
        pthread_mutex_unlock(&theMutex);
    }
}

void* calcIntegral(void *args) {
    params *arguments;
    arguments = (params *) args;
    double a, b;
    //printf("(%d) comecei...\n", arguments->id);
    do {
        //printf("(%d) %d\n", arguments->id, usingQueue);
        pthread_mutex_lock(&theMutex);
        //printf("%d\n", size);
        if (inputs[out] != -1) {

            a = inputs[out];
            inputs[out] = -1;
            out = (out + 1 < TAM) ? out + 1 : 0;

            b = inputs[out];
            inputs[out] = -1;
            out = (out + 1 < TAM) ? out + 1 : 0;

            size -= 2;
            //printf("%.10lf, %.10lf\n", a, b);
            usingQueue++;
            pthread_cond_broadcast(&theCond);
            pthread_mutex_unlock(&theMutex);
            adaptativeQuadrature(arguments->id, arguments->func, a, b, arguments->err);
        }
        else {
            pthread_mutex_unlock(&theMutex);
        }
    } while (usingQueue || inputs[out] != -1);

    //printf("(%d) sai do meio, indo pro fim\n", arguments->id);
    pthread_mutex_lock(&stackMutex);
    if (!doneMath) {
        GET_TIME(end);
        printf("Valor aproximado de f: %.20lf\n", output);
        printf("Tempo gasto: %lfs\n", end - begin);
        doneMath = 1;
    }
    pthread_mutex_unlock(&stackMutex);

    //printf("(%d) chega\n", arguments->id);
    pthread_exit(NULL);
}

int main(int argc, char const *argv[]) {
    double a, b, e;
    char choice;
    double (*func)(double);
    int nThreads;

    params *input;
    pthread_t threads[MAX_THREADS];

    if (argc < 5) {
        printf("Usage: <a (interval min)> <b (interval max)> <e (max error)> <threads>\n");
        exit(1);
    }
    a = atof(argv[1]);
    b = atof(argv[2]);
    e = atof(argv[3]);
    nThreads = atoi(argv[4]);

    pthread_mutex_init(&theMutex, NULL);
    pthread_mutex_init(&stackMutex, NULL);
    pthread_cond_init(&theCond, NULL);

    printf("Choose which function:\n");
    printf("(a) f(x) = 1 + x\n(b) f(x) = √(1 − xˆ2), −1 < x < 1\n(c) f(x) = √(1 + xˆ4)\n");
    do {
        printf("a b or c: ");
        scanf("%c", &choice);
        if (choice == 'b' && (a < -1 || b > 1)) {
            printf("cannot choose function 'b' because a is less than -1 or b is bigger than 1\n");
            choice = 'd';
        }
    } while (choice != 'a' && choice != 'b' && choice != 'c') ;

    func = funcList[choice-'a'];

    in = 2;
    out = 0;
    output = 0;
    size = 0;

    inputs[0] = a;
    inputs[1] = b;

    for (int i = 2; i < TAM; i++)
        inputs[i] = -1;

    usingQueue = 0;
    doneMath = 0;

    GET_TIME(begin);

    for (int i = 0; i < nThreads; i++) {
        input = (params *) malloc(sizeof(input));
        input->id = i;
        input->err = e;
        input->func = func;
        pthread_create(&threads[i], NULL, calcIntegral, (void *) input);
    }

    pthread_exit(NULL);
    return 0;
}
