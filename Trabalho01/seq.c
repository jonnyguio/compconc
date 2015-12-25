#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define func(op, x) ((op) == 'a') ? 1 + (x) : ((op) == 'c') ? pow ((1 + pow((x), 4)), 0.5) : (x > -1 || x < 1) ? pow ((1 - pow((x), 2)), 0.5) : 0 // Macro maluco que retonra o valor de cada função para a, b e c

#define getMedium(a, b) ((a) + (b)) / 2 //macro apenas para calcular o ponto médio entre duas entradas a e b

//função recursiva para quadratura adaptativa
float adaptativeQuadrature(char choose, float a, float b, float err) {
    float m, funcB, funcSleft, funcSright, areaS1, areaS2, areaB;
    /*
    funcB, funcSleft, funcSright: alturas dos retangulos, calculado a partir do ponto médio entre cada um dos retangulos.
    areaB: area do retangulo maior (b - a) * funcB
    areaS1, areaS2: area dos retangulos menores (m - a) * funcSleft e (b - m) * funcSright

    (left e right indicam apenas se está a esquerda ou direita do ponto médio)
    */
    m = getMedium(a, b);
    funcB = func(choose, m);
    funcSleft = func(choose, getMedium(a, m));
    funcSright = func(choose, getMedium(m, b));
    areaB = (b - a) * funcB;
    areaS1 = (m - a) * funcSleft;
    areaS2 = (b - m) * funcSright;
    // agora, se o módulo da diferença entre a area do retangulo maior e da soma dos retangulos menores for maior que o erro máximo, calcula a área dos dois intervalos [a,m], [b,m]
    // senão, apenas retorna o valor da area do retangulo maior
    if (fabsf(areaB - (areaS1 + areaS2)) > err) {
        // assim, a área nova sera definida pela soma das áreas dos retangulos menores
        areaB = adaptativeQuadrature(choose, a, m, err) + adaptativeQuadrature(choose, m, b, err);
    }

    return areaB;
}

int main(int argc, char const *argv[]) {
    float a, b, e;
    char choose;

    if (argc < 4) {
        printf("Usage: <a (interval min)> <b (interval max)> <e (max error)>\n");
        exit(1);
    }
    a = atof(argv[1]);
    b = atof(argv[2]);
    e = atof(argv[3]);

    printf("Choose which function:\n");
    printf("(a) f(x) = 1 + x\n(b) f(x) = √(1 − xˆ2), −1 < x < 1\n(c) f(x) = √(1 + xˆ4)\n");
    // Apenas um loop que força o usuario a escolher uma função que pode ser usada.
    do {
        printf("a b or c: ");
        scanf("%c", &choose);
        if (choose == 'b' && (a < -1 || b > 1)) {
            printf("cannot choose function 'b' because a is less than -1 or b is bigger than 1\n");
            choose = 'd';
        }
    } while (choose != 'a' && choose != 'b' && choose != 'c') ;

    printf("Valor aproximado de f: %.15f\n", adaptativeQuadrature(choose, a, b, e));

    return 0;
}
