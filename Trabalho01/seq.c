#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define func(op, x) ((op) == 'a') ? 1 + (x) : ((op) == 'c') ? pow ((1 + pow((x), 4)), 0.5) : (x > -1 || x < 1) ? pow ((1 - pow((x), 2)), 0.5) : 0

#define getMedium(a, b) ((a) + (b)) / 2

float adaptativeQuadrature(char choose, float a, float b, float err) {
    float m, funcB, funcSleft, funcSright, areaS1, areaS2, areaB;
    m = getMedium(a, b);
    funcB = func(choose, m);
    funcSleft = func(choose, getMedium(a, m));
    funcSright = func(choose, getMedium(m, b));
    areaB = (b - a) * funcB;
    areaS1 = (m - a) * funcSleft;
    areaS2 = (b - m) * funcSright;
    if (fabsf(areaB - (areaS1 + areaS2)) > err) {
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
    do {
        printf("a b or c: ");
        scanf("%c", &choose);
        if (choose == 'b' && (a < -1 || b > 1)) {
            printf("cannot choose function 'b' because a is less than -1 or b is bigger than 1\n");
            choose = 'd';
        }
    } while (choose != 'a' && choose != 'b' && choose != 'c') ;

    printf("Valor aproximado de f: %f\n", adaptativeQuadrature(choose, a, b, e));

    return 0;
}
