#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//funções existentes para o cálculo de suas integrais
double func1(double x){return 1+x;}
double func2(double x){return pow ((1 - pow((x), 2)), 0.5);}
double func3(double x){return pow ((1 + pow((x), 4)), 0.5);}

//lista das funções
double (*funcList[3])(double) = {func1, func2, func3};

//macro apenas para calcular o ponto médio entre duas entradas a e b
#define getMiddle(a, b) (a + b) / 2


typedef struct Interval
{
    double (*func)(double);
    double a;
    double b;
    double err;
}Interval;

typedef struct IntervalList
{
    Interval *intervals;
    int size;
    int maxSize;
}IntervalList;

int nThreads;                   //Número de Threads
int *freeThreads;               //Threads disponíveis
int nFreeThreads;               //Número de threads disponíveis
int overloadMagicNumber = 10;   //Número mágico que define se uma thread está ou não sobrecarregada.
IntervalList *threadIntervals;  //Vetor de Listas de Intervalos que cada thread tem para calcular
IntervalList overloadIntervals; //Lista que contém Intervalos a ser calculados. Intervalos vão parar aqui quando threads ficam sobrecarregadas.

int *goodToGo;              //Verifica se as threads têm intervalos a ser calculados
char end;                   //Verifica o término do programa

pthread_mutex_t freeThreads_mutex, overload_mutex;
pthread_cond_t freeThreads_cond;

double *finalSum;           //Vetor com resultado da aproximação. Cada posição representa a parcial encontrada por cada thread, para evitar sincronizações desnecessárias.

//Pede uma thread para calcular o intervalo dado
int askForThread(Interval interval);

//Avisa para o programa que uma thread está livre
void imFree(int id);

//Função que será realizada por cada thread
void* threadFunction(void* arg);

//Faz a aproximação para dado intervalo
void adaptativeQuadrature(Interval interval, int id);


int askForThread(Interval interval)
{
    int j;
    pthread_mutex_lock(&freeThreads_mutex);
    if(nFreeThreads != 0)
    {
        int id = freeThreads[0];
        nFreeThreads--;
        printf("Asking for Thread\n");
        //jeito horrível plim plim (lerdo as fudge)
        threadIntervals[id].intervals[0] = interval;
        threadIntervals[id].size++;
        printf("Thread %d recebendo trabalho...\n", id);
        goodToGo[id] = 1;
        for (j = 0; j < nThreads-1; ++j)
            freeThreads[j] = freeThreads[j+1];            
        freeThreads[nThreads-1] = -1;
        pthread_mutex_unlock(&freeThreads_mutex);
        return 1;
    }
    else
    {
        pthread_mutex_unlock(&freeThreads_mutex);
        return 0;
    }
}

void storeOverloadedIntervals(IntervalList list)
{
    pthread_mutex_lock(&freeThreads_mutex);
    for (int i = list.size/2; i < list.size; ++i)
    {
        if(overloadIntervals.size + 1 >= overloadIntervals.maxSize)
            overloadIntervals.intervals = (Interval *)realloc(overloadIntervals.intervals, (sizeof(Interval) * 2 * overloadIntervals.maxSize));
        overloadIntervals.intervals[overloadIntervals.size] = list.intervals[i];
        overloadIntervals.size++;
        list.size--;
    }

    pthread_mutex_unlock(&freeThreads_mutex);
}

void imFree(int id)
{
    int i;
    pthread_mutex_lock(&freeThreads_mutex);
    nFreeThreads++;
    for (i = 0; freeThreads[i] != -1; ++i);
    freeThreads[i] = id;    
    printf("freeThreads[%d] = %d\n", i, id);
    pthread_cond_signal(&freeThreads_cond);
    pthread_mutex_unlock(&freeThreads_mutex);    
}

void* threadFunction(void* arg)
{
    int myid = *(int*)(arg);
    while(!end)
    {
        //printf("penes %d\n", myid);
        while(!goodToGo[myid])
            if(end)
                pthread_exit(NULL);
        while(threadIntervals[myid].size)
        {
            adaptativeQuadrature(threadIntervals[myid].intervals[0], myid);
            for (int i = 1; i < threadIntervals[myid].size-1; ++i)
            {
                threadIntervals[myid].intervals[i-1] = threadIntervals[myid].intervals[i];
            }
            threadIntervals[myid].size--;
        }
    }
    pthread_exit(NULL);
}

void adaptativeQuadrature(Interval interval, int id)
{
    double m, funcB, funcSleft, funcSright, areaS1, areaS2, areaB;
    Interval leftInterval, rightInterval;
    //int leftThread, rightThread;
    /*
    funcB, funcSleft, funcSright: alturas dos retangulos, calculado a partir do ponto médio entre cada um dos retangulos.
    areaB: area do retangulo maior (b - a) * funcB
    areaS1, areaS2: area dos retangulos menores (m - a) * funcSleft e (b - m) * funcSright
    (left e right indicam apenas se está a esquerda ou direita do ponto médio)
    */
    printf("piru %d\n", id);

    m = getMiddle(interval.a, interval.b);
    
    funcB = interval.func(m);

    funcSleft = interval.func(getMiddle(interval.a, m));
    funcSright = interval.func(getMiddle(m, interval.b));
    areaB = (interval.b - interval.a) * funcB;
    areaS1 = (m - interval.a) * funcSleft;
    areaS2 = (interval.b - m) * funcSright;

    printf("calculated\n");
    // agora, se o módulo da diferença entre a area do retangulo maior e da soma dos retangulos menores for maior que o erro máximo, calcula a área dos dois intervalos [a,m], [b,m]
    // senão, apenas retorna o valor da area do retangulo maior
    if (fabs(areaB - (areaS1 + areaS2)) > interval.err)
    {        
        leftInterval = rightInterval = interval;
        leftInterval.b = m;
        rightInterval.a = m;

        threadIntervals[id].intervals[threadIntervals[id].size] = rightInterval;
        if(!askForThread(leftInterval))
        {
            if(threadIntervals[id].size + 1 >= overloadMagicNumber)
            {
                threadIntervals[id].intervals[threadIntervals[id].size] = leftInterval;
                storeOverloadedIntervals(threadIntervals[id]);
            }
        }
    }
    else
    {       
        goodToGo[id] = 0;
        imFree(id);        
        finalSum[id] += areaB;
        if (nFreeThreads == nThreads)
        {
            end = 1;
        }
    }
}

int main(int argc, char const *argv[]) 
{
    double a, b, e;
    char choice;
    double (*func)(double);
    pthread_t *tid;
    int *id;

    if (argc < 5) {
        printf("Usage: <a (interval min)> <b (interval max)> <e (max error)> <#threads>\n");
        exit(1);
    }
    //converte os valores recebidos para double
    a = strtod(argv[1], NULL);
    b = strtod(argv[2], NULL);
    e = strtod(argv[3], NULL);
    nThreads = atoi(argv[4]);

    printf("nThreads = %d\n", nThreads);

    printf("Choose which function:\n");
    printf("(a) f(x) = 1 + x\n(b) f(x) = √(1 − xˆ2), −1 < x < 1\n(c) f(x) = √(1 + xˆ4)\n");

    // Apenas um loop que força o usuario a escolher uma função que pode ser usada.
    do {
        printf("a b or c: ");
        scanf("%c", &choice);
        if (choice == 'b' && (a < -1 || b > 1)) {
            printf("cannot choose function 'b' because a is less than -1 or b is bigger than 1\n");
            choice = 'd';
        }
    } while (choice != 'a' && choice != 'b' && choice != 'c') ;
    func = funcList[choice-'a'];

    pthread_mutex_init(&freeThreads_mutex, NULL);
    pthread_cond_init(&freeThreads_cond, NULL);
    
    id = (int *) malloc(sizeof(int) * nThreads);
    tid = (pthread_t *) malloc(sizeof(pthread_t) * nThreads);
    freeThreads = (int *) malloc(sizeof(int) * nThreads);
    goodToGo = (int *) malloc(sizeof(int) * nThreads);
    threadIntervals = (IntervalList *) malloc(sizeof(Interval) * nThreads);
    finalSum = (double *) calloc(sizeof(double), nThreads);

    for (int i = 0; i < nThreads; ++i)
    {
        threadIntervals[i].intervals = (Interval *) malloc(sizeof(Interval) * nThreads);
        threadIntervals[i].size = 0;
        threadIntervals[i].maxSize = overloadMagicNumber;
    }

    overloadIntervals.intervals = (Interval *) malloc(sizeof(Interval) * 10 * nThreads);
    overloadIntervals.size = 0;
    overloadIntervals.maxSize = 10*nThreads;

    //if(goodToGo == NULL) {printf("--ERRO: malloc()\n"); exit(1);}
    //if(id == NULL) {printf("--ERRO: malloc()\n"); exit(1);}
    //if(tid == NULL) {printf("--ERRO: malloc()\n"); exit(1);}
    //if(freeThreads == NULL) {printf("--ERRO: malloc()\n"); exit(1);}

    for (int i = 0; i < nThreads; ++i)
    {       
        id[i] = i;
        goodToGo[i] = 0;
        freeThreads[i] = i;
        nFreeThreads = nThreads;
    }
    end = 0;
    for (int i = 0; i < nThreads; ++i)
    {       
        if (pthread_create(&tid[i], NULL, threadFunction, (void *) &id[i])) {
            printf("--ERRO: pthread_create()\n"); exit(-1);
        }
    }   
    askForThread((Interval){func, a, b, e});

    for (int i = 0; i < nThreads; ++i)
    {
        pthread_join(tid[i], NULL);
    }   

    pthread_mutex_destroy(&freeThreads_mutex);
    pthread_cond_destroy(&freeThreads_cond);

    for (int i = 1; i < nThreads; ++i)
    {
        finalSum[0]+=finalSum[i];
    }

    printf("Approximate value for the integral of f from %lf to %lf: %lf\n", a, b, finalSum[0]);

    pthread_exit (NULL);
}
