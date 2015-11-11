#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>

int main (int argc, char *argv[]) {
    int i, n;
    double pi = 4, sum = 0;

    if (argc < 2) {
        printf("ERROR. Use <N>.\n");
        exit(1);
    }
    n = atoi(argv[1]);

    for (i = 1; i <= n; i++) {
        sum += (1 / ((double)i * 2 - 1)) * ((i & 1) ? 1 : -1);
    }
    pi *= sum;
    printf("PI: %.20lf\tM_PI: %.20lf\n", pi, M_PI);

    return 0;
}
