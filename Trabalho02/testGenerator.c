// Codigo do Tiago Montalvão, com poucas adaptações
// Thanks

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

char* NAME_OF_FILE(char str[20], int x) {
	sprintf(str, "%d", x);
	strcat(str, ".in");
	return str;
}

int main(int argc, char *argv[]) {
	char str[20];
	int onScreen, fileNumber;
	if (argc < 2) {
		printf("Usage: <onScreen>\n");
		return 1;
	}
	else {
		onScreen = atoi(argv[1]);
		if (!onScreen && argc < 3) {
			printf("Usage: <0> <number of file>\n");
			return 2;
		}
		else
			fileNumber = atoi(argv[2]);
	}

	srand(time(0));
	int N = rand()%96 + 5;
	int M = rand()%16 + 5;
	int C = rand()%16 + 5;

	if (!onScreen) freopen(NAME_OF_FILE(str, fileNumber | 0), "w", stdout);

	fprintf(stdout, "%d %d %d\n", N, M, C);
	for (int i = 0, X; i < M; ++i) {
		X = rand()%N;
		fprintf(stdout, "%d%c", X, " \n"[i == (M-1)]);
	}
	for (int i = 0, X, quant; i < N; i++) {
		quant = rand()%(2*C);
		fprintf(stdout, "%d", quant);
		for (int j = 0; j < quant; ++j) {
			while ((X=rand()%N) == i);
			fprintf(stdout, " %d", X);
		}
		fprintf(stdout, "\n");
	}
	return 0;
}
