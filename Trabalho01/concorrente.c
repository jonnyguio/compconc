#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include "../timer.h"

double func1(double x){return 1+x;}
double func2(double x){return pow ((1 - pow((x), 2)), 0.5);}
double func3(double x){return pow ((1 + pow((x), 4)), 0.5);}

double (*funcList[3])(double) = {func1, func2, func3};

#define MAX_THREADS 8

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
    double a, b, e;
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

    if (nThreads > 8) {
        printf("WARNING: Trying to use more than 8 threads. Please select a number [1,8].\n");
        exit(2);
    }

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

    //inicia-se então o cálculo do tempo da concorrencia (que inclui criações de threads e suas execuções)
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

    // inicia-se aqui a recursão do retangulo que foi designado
    // qualquer thread pode criar outras threads se o numero máximo ainda não foi alcançado.
    results[arguments->id] = adaptativeQuadrature(arguments->id, arguments->func, arguments->a, arguments->b, arguments->err);

    pthread_mutex_lock(&theMutex);
    finished++; // avisa que terminou
    if (finished == instantiate) {  // quando todas as threads instanciadas terminam, a ultima fica responsável por somar todos os retangulos finais.
        res = 0;
        for (int i = 0; i < nThreads; i++)
            res += results[i];
        GET_TIME(end); // após todos os cálculos, podemos pegar o tempo total de execução.
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
    /*
    funcB, funcSleft, funcSright: alturas dos retangulos, calculado a partir do ponto médio entre cada um dos retangulos.
    areaB: area do retangulo maior (b - a) * funcB
    areaS1, areaS2: area dos retangulos menores (m - a) * funcSleft e (b - m) * funcSright
    (left e right indicam apenas se está a esquerda ou direita do ponto médio)
    */
    m = getMiddle(a, b);
    funcB = func(m);
    funcSleft = func(getMiddle(a, m));
    funcSright = func(getMiddle(m, b));
    areaB = (b - a) * funcB;
    areaS1 = (m - a) * funcSleft;
    areaS2 = (b - m) * funcSright;
    // agora, se o módulo da diferença entre a area do retangulo maior e da soma dos retangulos menores for maior que o erro máximo, calcula a área dos dois intervalos [a,m], [b,m]
    // porém, agora na versão concorrente, checamos se existe a possibilidade de delegar essa tarefa a uma nova threads
    // assim, se nem todas as threads foram iniciadas, então iremos iniciar uma nova thread que ficará responsável por um dos retangulos e a thread atual fica responsável apenas pelo outro retangulo.
    // senão, apenas retorna o valor da area do retangulo maior
    if (fabs(areaB - (areaS1 + areaS2)) > err) {
        if (!allinstantiate) { //mesmo estando fora de um mutex, não há problema por ter uma confirmação dentro.
            pthread_mutex_lock(&theMutex);

            if (instantiate < nThreads) { // checa se é possível instanciar.
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
            else { //se não for, apenas avisa que não é mais possível instanciar e continua a trabalhar normalmente

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
