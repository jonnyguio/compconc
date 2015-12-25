#include <stdio.h>
#include <stdlib.h>
#include <math.h>

double func1(double x){return 1+x;}
double func2(double x){return pow ((1 - pow((x), 2)), 0.5);}
double func3(double x){return pow ((1 + pow((x), 4)), 0.5);}

double (*funcList[3])(int) = {func1, func2, func3};

//#define func(op, x) ((op) == 'a') ? 1 + (x) : ((op) == 'c') ? pow ((1 + pow((x), 4)), 0.5) : (x > -1 || x < 1) ? pow ((1 - pow((x), 2)), 0.5) : 0

#define getMiddle(a, b) ((a) + (b)) / 2

double adaptativeQuadrature(double (*func)(int), float a, float b, float err) {
    double m, funcB, funcSleft, funcSright, areaS1, areaS2, areaB;
    m = getMiddle(a, b);
    funcB = func(m);
    funcSleft = func(getMiddle(a, m));
    funcSright = func(getMiddle(m, b));
    areaB = (b - a) * funcB;
    areaS1 = (m - a) * funcSleft;
    areaS2 = (b - m) * funcSright;
    if (fabs(areaB - (areaS1 + areaS2)) > err) {
        areaB = adaptativeQuadrature(func, a, m, err) + adaptativeQuadrature(func, m, b, err);
    }
    return areaB;
}

int main(int argc, char const *argv[]) {
    float a, b, e;
    char choice;
    double (*func)(double);

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
        scanf("%c", &choice);
        if (choice == 'b' && (a < -1 || b > 1)) {
            printf("cannot choose function 'b' because a is less than -1 or b is bigger than 1\n");
            choice = 'd';
        }
    } while (choice != 'a' && choice != 'b' && choice != 'c') ;

    func = funcList[choice-'a'];

    printf("Valor aproximado de f: %f\n", adaptativeQuadrature(func, a, b, e));

    return 0;
}
