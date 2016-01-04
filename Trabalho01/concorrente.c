#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include "../timer.h"

double func1(double x){return 1+x;}
double func2(double x){return pow ((1 - pow((x), 2)), 0.5);}
double func3(double x){return pow ((1 + pow((x), 4)), 0.5);}

double (*funcList[3])(double) = {func1, func2, func3};

#define getMiddle(a, b) ((a) + (b)) / 2

typedef struct _params {
    int id;
    double err, a, b;
    double (*func)(double);
} params;

pthread_t *threads;
pthread_mutex_t theMutex;

int instantiate, finished, allinstantiate = 0, nThreads;
double *results, begin, end;

void *calcIntegral(void *args);
double adaptativeQuadrature(int id, double (*func)(double), double a, double b, double err);

pthread_mutex_t theMutex;
pthread_cond_t theCond;

int main(int argc, char const *argv[]) {
    float a, b, e;
    char choice;
    double (*func)(double);

    params *input;

    if (argc < 5) {
        printf("Usage: <a (interval min)> <b (interval max)> <e (max error)> <threads>\n");
        exit(1);
    }

    a = atof(argv[1]);
    b = atof(argv[2]);
    e = atof(argv[3]);
    nThreads = atoi(argv[4]);

    pthread_mutex_init(&theMutex, NULL);

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
    instantiate = 0;
    finished = 0;
    threads = (pthread_t *) malloc(sizeof(pthread_t) * nThreads);
    results = (double *) malloc(sizeof(double) * nThreads);

    GET_TIME(begin);

    input = (params *) malloc(sizeof(params));
    input->id = 0;
    input->err = e;
    input->a = a;
    input->b = b;
    input->func = func;
    instantiate++;
    pthread_create(&threads[0], NULL, calcIntegral, (void *) input);

    pthread_exit(NULL);
    return 0;
}


void *calcIntegral(void *args) {
    params *arguments;
    arguments = (params *) args;
    double res;
    results[arguments->id] = adaptativeQuadrature(arguments->id, arguments->func, arguments->a, arguments->b, arguments->err);
    pthread_mutex_lock(&theMutex);
    finished++;
    if (finished == nThreads) {
        res = 0;
        for (int i = 0; i < nThreads; i++)
            res += results[i];
        GET_TIME(end);
        printf("Approximate value for the integral of f: %.20lf\n", res);
        printf("Time: %lfs\n", end - begin);
    }
    pthread_mutex_unlock(&theMutex);

    free(args);
    pthread_exit(NULL);
}

double adaptativeQuadrature(int id, double (*func)(double), double a, double b, double err) {
    double m, funcB, funcSleft, funcSright, areaS1, areaS2, areaB;
    params *input;
    m = getMiddle(a, b);
    funcB = func(m);
    funcSleft = func(getMiddle(a, m));
    funcSright = func(getMiddle(m, b));
    areaB = (b - a) * funcB;
    areaS1 = (m - a) * funcSleft;
    areaS2 = (b - m) * funcSright;

    if (fabs(areaB - (areaS1 + areaS2)) > err) {
        if (!allinstantiate) {
            pthread_mutex_lock(&theMutex);

            if (instantiate < nThreads) {
                input = (params *) malloc(sizeof(params));
                input->id = instantiate;
                input->err = err;
                input->a = m;
                input->b = b;
                input->func = func;
                pthread_create(&threads[instantiate], NULL, calcIntegral, (void *) input);
                instantiate++;

                pthread_mutex_unlock(&theMutex);
                areaB = adaptativeQuadrature(id, func, a, m, err);
            }
            else {
                
                allinstantiate = 1;
                pthread_mutex_unlock(&theMutex);
                areaB = adaptativeQuadrature(id, func, a, m, err) + adaptativeQuadrature(id, func, m, b, err);
            }
        }
        else
            areaB = adaptativeQuadrature(id, func, a, m, err) + adaptativeQuadrature(id, func, m, b, err);
    }
    return areaB;
}
