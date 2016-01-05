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

int nThreads;				//Número de Threads
int *freeThreads; 			//Threads disponíveis
Interval *threadIntervals; 	//Intervalo que cada thread tem que calcular

int *goodToGo;				//Verifica se as threads têm intervalos a ser calculados
char end;					//Verifica o término do programa

pthread_mutex_t freeThreads_mutex, sum_mutex;
pthread_cond_t freeThreads_cond;

double *finalSum;			//Vetor com resultado da aproximação. Cada posição representa a parcial encontrada por cada thread, para evitar sincronizações desnecessárias.

//Pede uma thread para calcular o intervalo dado
void askForThread(Interval interval);

//Avisa para o programa que uma thread está livre
void imFree(int id);

//Função que será realizada por cada thread
void* threadFunction(void* arg);

//Faz a aproximação para dado intervalo
void adaptativeQuadrature(Interval interval, int id);


void askForThread(Interval interval)
{
	int i, j;
	for (i = 0; i < nThreads; ++i)
	{
		pthread_mutex_lock(&freeThreads_mutex);
		while(freeThreads[0]==-1)
			pthread_cond_wait(&freeThreads_cond, &freeThreads_mutex);

		//jeito horrível plim plim (lerdo as fudge)
		threadIntervals[freeThreads[0]] = interval;
		goodToGo[freeThreads[0]] = 1;
		for (j = 0; j < nThreads-1; ++j)
			freeThreads[j] = freeThreads[j+1];
		freeThreads[nThreads-1] = -1;

		pthread_mutex_unlock(&freeThreads_mutex);
	}
}

void imFree(int id)
{
	int i;
	for (i = 0; freeThreads[i] != -1; ++i);
	freeThreads[i] = id;
	pthread_cond_signal(&freeThreads_cond);
}

void* threadFunction(void* arg)
{
	int myid = *(int*)(arg);
	while(!end)
	{
		printf("penes %d\n", myid);
		while(!goodToGo[myid])
			if(end)
				pthread_exit(NULL);
		adaptativeQuadrature(threadIntervals[myid], myid);
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
    // agora, se o módulo da diferença entre a area do retangulo maior e da soma dos retangulos menores for maior que o erro máximo, calcula a área dos dois intervalos [a,m], [b,m]
    // senão, apenas retorna o valor da area do retangulo maior
    if (fabs(areaB - (areaS1 + areaS2)) > interval.err)
    {
		// assim, a área nova sera definida pela soma das áreas dos retangulos menores
		// areaB = adaptativeQuadrature(interval.func, interval.a, m, interval.err) + adaptativeQuadrature(func, m, b, err);
		leftInterval = rightInterval = interval;
    	leftInterval.b = m;
    	rightInterval.a = m;
		askForThread(leftInterval);
		threadIntervals[id] = rightInterval;
    }
    else
    {    	
    	goodToGo[id] = 0;
    	imFree(id);
    	pthread_mutex_lock(&sum_mutex);
    	finalSum[id] += areaB;
    	pthread_mutex_unlock(&sum_mutex);

    	if(id == 0)
    		end = 1;
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
    pthread_mutex_init(&sum_mutex, NULL);
    pthread_cond_init(&freeThreads_cond, NULL);
	
	id = (int *) malloc(sizeof(int) * nThreads);
    tid = (pthread_t *) malloc(sizeof(pthread_t) * nThreads);
    freeThreads = (int *) malloc(sizeof(int) * nThreads);
    goodToGo = (int *) malloc(sizeof(int) * nThreads);
	threadIntervals = (Interval *) malloc(sizeof(Interval) * nThreads);
	finalSum = (double *) calloc(sizeof(double), nThreads);
    //if(goodToGo == NULL) {printf("--ERRO: malloc()\n"); exit(1);}
	//if(id == NULL) {printf("--ERRO: malloc()\n"); exit(1);}
    //if(tid == NULL) {printf("--ERRO: malloc()\n"); exit(1);}
    //if(freeThreads == NULL) {printf("--ERRO: malloc()\n"); exit(1);}

	for (int i = 0; i < nThreads; ++i)
	{		
		id[i] = i;
		goodToGo[i] = 1;
    	freeThreads[i] = i;
	}
	end = 0;
	for (int i = 0; i < nThreads; ++i)
	{		
		if (pthread_create(&tid[i], NULL, threadFunction, (void *) &id[i])) {
      		printf("--ERRO: pthread_create()\n"); exit(-1);
    	}
	}	
	askForThread((Interval){func, a, b, e});

	while(!end);

	pthread_mutex_destroy(&sum_mutex);
	pthread_mutex_destroy(&freeThreads_mutex);
 	pthread_cond_destroy(&freeThreads_cond);

	for (int i = 1; i < nThreads; ++i)
	{
		finalSum[0]+=finalSum[i];
	}

    printf("Approximate value for the integral of f from %lf to %lf: %lf\n", a, b, finalSum[0]);

  	pthread_exit (NULL);
}
