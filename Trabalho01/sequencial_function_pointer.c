#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../timer.h"

//funções existentes para o cálculo de suas integrais
double func1(double x){return 1+x;}
double func2(double x){return pow ((1 - pow((x), 2)), 0.5);}
double func3(double x){return pow ((1 + pow((x), 4)), 0.5);}

//lista das funções
double (*funcList[3])(double) = {func1, func2, func3};

//macro apenas para calcular o ponto médio entre duas entradas a e b
#define getMiddle(a, b) ((a) + (b)) / 2

double adaptativeQuadrature(double (*func)(double), double a, double b, double err) {
    double m, funcB, funcSleft, funcSright, areaS1, areaS2, areaB;
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
    // senão, apenas retorna o valor da area do retangulo maior
    if (fabs(areaB - (areaS1 + areaS2)) > err) {
        // assim, a área nova sera definida pela soma das áreas dos retangulos menores
        areaB = adaptativeQuadrature(func, a, m, err) + adaptativeQuadrature(func, m, b, err);
    }
    return areaB;
}

int main(int argc, char const *argv[]) {
    double a, b, e, begin, end;

    char choice;
    double (*func)(double);

    if (argc < 4) {
        printf("Usage: <a (interval min)> <b (interval max)> <e (max error)>\n");
        exit(1);
    }
    //converte os valores recebidos para double
    a = strtod(argv[1], NULL);
    b = strtod(argv[2], NULL);
    e = strtod(argv[3], NULL);

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

    GET_TIME(begin);
    printf("Approximate value for the integral of f from %lf to %lf: %.20lf\n", a, b, adaptativeQuadrature(func, a, b, e));
    GET_TIME(end);
    printf("Time: %lfs\n", end - begin);

    return 0;
}
