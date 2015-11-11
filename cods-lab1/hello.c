/* Disciplina: Computacao Concorrente */
/* Prof.: Silvana Rossetto */
/* Laboratório: 1 */
/* Codigo: "Hello World" usando threads em C */

#include <stdio.h>
#include <stdlib.h> 
#include <pthread.h>

#define NTHREADS  10

//--funcao executada pelas threads
void *PrintHello (void *arg) {

  printf("Hello World\n");

  pthread_exit(NULL);
}

//--funcao principal do programa
int main(void) {
  pthread_t tid_sistema[NTHREADS]; //identificadoes das threads no sistema
  int t; //variavel auxiliar

  for(t=0; t<NTHREADS; t++) {
    printf("--Cria a thread %d\n", t);
    if (pthread_create(&tid_sistema[t], NULL, PrintHello, NULL)) {
      printf("--ERRO: pthread_create()\n"); exit(-1);
    }
  }

  printf("--Thread principal terminou\n");

  pthread_exit(NULL);
}
