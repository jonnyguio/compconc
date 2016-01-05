#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include "../timer.h"

//funções existentes para o cálculo de suas integrais
double func1(double x){return 1+x;}
double func2(double x){return pow ((1 - pow((x), 2)), 0.5);}
double func3(double x){return pow ((1 + pow((x), 4)), 0.5);}
//double func4(double x){return sin(x);}

//lista das funções
double (*funcList[4])(double) = {func1, func2, func3, sin};

//Numero maximo de threads do programa
#define MAX_THREADS 8

//macro apenas para calcular o ponto médio entre duas entradas a e b
#define getMiddle(a, b) ((a) + (b)) / 2

//Parametros passados para cada threads
typedef struct _params {
    int id;
    double err, a, b;
    double (*func)(double);
} params;

pthread_t *threads; // Vetor que contém as threads
pthread_mutex_t theMutex; // Mutex responsável por seções críticas
pthread_cond_t theCond; // Condição usada na espera de novos retangulos

int
    instantiated, finished, allinstantiated = 0, nThreads, // usado em instanciação e checagem de términos
    anyoneFree, anythingSent, // usado na espera de novos retangulos
    doneMath; // Usado apenas para saber se alguma thread já calculou o resultado final

double
    *results, // vetor responsável pelas contas finais
    begin, end, // para cálculo de tempo
    globala, globalb; // para passar novos retangulos à threads ociosas

void *calcIntegral(void *args); // função das threads
double adaptativeQuadrature(int id, double (*func)(double), double a, double b, double err); // função de calculo da quadratura adaptativa

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

    printf("Choose which function:\n");
    printf("(a) f(x) = 1 + x\n(b) f(x) = √(1 − xˆ2), −1 < x < 1\n(c) f(x) = √(1 + xˆ4)\n(d) f(x) = sin(x)\n");
    do {
        printf("a, b, c or d: ");
        scanf("%c", &choice);
        if (choice == 'b' && (a < -1 || b > 1)) {
            printf("cannot choose function 'b' because a is less than -1 or b is bigger than 1\n");            
        }
    } while (choice < 'a' || choice > 'd') ;

    // Inicialização do mutex e da condição
    pthread_mutex_init(&theMutex, NULL);
    pthread_cond_init(&theCond, NULL);

    // Escolha da função
    func = funcList[choice-'a'];

    // Inicialização de variáveis
    instantiated = 0;
    finished = 0;
    threads = (pthread_t *) malloc(sizeof(pthread_t) * nThreads);
    results = (double *) malloc(sizeof(double) * nThreads);
    doneMath = 0;

    //inicia-se então o cálculo do tempo da concorrencia (que inclui criações de threads e suas execuções)
    GET_TIME(begin);

    // Criação da primeira thread
    input = (params *) malloc(sizeof(params));
    input->id = 0;
    input->err = e;
    input->a = a;
    input->b = b;
    input->func = func;
    instantiated++;
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

    // pequena seção crítica para quem terminou os cálculos anotar que terminou
    pthread_mutex_lock(&theMutex);
    finished++;
    pthread_mutex_unlock(&theMutex);

    // Loop então para threads que já calcularam seus devidos retangulos e agora estão ociosas.
    while (finished != instantiated) {
        pthread_mutex_lock(&theMutex);
        anyoneFree = arguments->id; // avisa a todos que ela está livre
        if (finished != instantiated) { // garantindo na seção crítica a condição do loop

            // se alguém enviou um novo retangulo a calcular, faz-se assim os cálculos do novo retangulo
            // senão, apenas espera o envio de um novo sinal
            if (anythingSent) {
                finished--;
                anythingSent = 0;
                anyoneFree = 0;
                pthread_cond_broadcast(&theCond);
                pthread_mutex_unlock(&theMutex);
                results[arguments->id] += adaptativeQuadrature(arguments->id, arguments->func, globala, globalb, arguments->err);
                pthread_mutex_lock(&theMutex);
                finished++;
                pthread_mutex_unlock(&theMutex);
            }
            else {
                pthread_cond_wait(&theCond, &theMutex);
                pthread_mutex_unlock(&theMutex);
            }
        }
        else {
            // executou o loop mesmo sem a condição (pois não está na seção crítica) avisa a todas outras threads que podem estar presas para continuar.
            pthread_cond_broadcast(&theCond);
            pthread_mutex_unlock(&theMutex);
        }
    }
    // avisa a todas as outras threads que já passou direto do loop, para caso tenha alguma esperando novos dados (que não irão chegar nunca)
    pthread_cond_broadcast(&theCond);
    pthread_mutex_lock(&theMutex);
    if (!doneMath) {  // quando todas as threads instanciadas terminam, a ultima fica responsável por somar todos os retangulos finais.
        doneMath = 1;
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
        if (!allinstantiated) { //mesmo estando fora de um mutex, não há problema por ter uma confirmação dentro.
            pthread_mutex_lock(&theMutex);

            if (instantiated < nThreads) { // checa se é possível instanciar.
                input = (params *) malloc(sizeof(params));
                input->id = instantiated;
                input->err = err;
                input->a = m;
                input->b = b;
                input->func = func;
                pthread_create(&threads[instantiated], NULL, calcIntegral, (void *) input);
                instantiated++;

                pthread_mutex_unlock(&theMutex);
                areaB = adaptativeQuadrature(id, func, a, m, err);
            }
            else { //se não for, apenas avisa que não é mais possível instanciar e continua a trabalhar normalmente

                allinstantiated = 1;
                pthread_mutex_unlock(&theMutex);
                areaB = adaptativeQuadrature(id, func, a, m, err) + adaptativeQuadrature(id, func, m, b, err);
            }
        }
        else {
            if (anyoneFree && !anythingSent) { // checa se tem alguém ocioso e se ninguém enviou dado ainda
                pthread_mutex_lock(&theMutex);
                if (anythingSent) { // recheca a condição, agora numa seção crítica (entre locks)
                    globala = m;
                    globalb = b;
                    anythingSent = 1;
                    // avisa as threads que estão esperando que já enviou dados
                    pthread_cond_broadcast(&theCond);
                    pthread_mutex_unlock(&theMutex);
                    areaB = adaptativeQuadrature(id, func, a, m, err);
                }
                else {
                    // senão, simplesmente avisa que saiu do loop e continua execução
                    pthread_cond_broadcast(&theCond);
                    pthread_mutex_unlock(&theMutex);
                    areaB = adaptativeQuadrature(id, func, a, m, err) + adaptativeQuadrature(id, func, m, b, err);
                }
            }
            else { // senão, simplesmente calcula mais retangulos
                areaB = adaptativeQuadrature(id, func, a, m, err) + adaptativeQuadrature(id, func, m, b, err);
            }
        }
    }
    return areaB;
}
