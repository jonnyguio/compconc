/* Disciplina: Computacao Concorrente */
/* Prof.: Silvana Rossetto */
/* Laboratório: 1 */
/* Codigo: "Hello World" usando threads em C */

#include <stdio.h>
#include <stdlib.h> 
#include <pthread.h>

#define NTHREADS  10

//cria a estrutura de dados para armazenar os argumentos da thread
typedef struct {
   int idThread, nThreads;
} t_Args;

//funcao executada pelas threads
void *PrintHello (void *arg) {
  t_Args *args = (t_Args *) arg;

  printf("Sou a thread %d de %d threads\n", args->idThread, args->nThreads);
  free(arg);

  pthread_exit(NULL);
}

//funcao principal do programa
int main() {
  pthread_t tid_sistema[NTHREADS];
  int t;
  t_Args *arg; //receberá os argumentos para a thread

  for(t=0; t<NTHREADS; t++) {
    printf("--Aloca e preenche argumentos para thread %d\n", t);
    arg = malloc(sizeof(t_Args));
    if (arg == NULL) {
      printf("--ERRO: malloc()\n"); exit(-1);
    }
    arg->idThread = t; 
    arg->nThreads = NTHREADS; 
    
    printf("--Cria a thread %d\n", t+1);
    if (pthread_create(&tid_sistema[t], NULL, PrintHello, (void*) arg)) {
      printf("--ERRO: pthread_create()\n"); exit(-1);
    }
  }

  printf("--Thread principal terminou\n");
  pthread_exit(NULL);
}
