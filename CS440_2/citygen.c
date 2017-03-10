#include <stdio.h>

void generatesingle(int id, int cities) {
	FILE* newfile;
	char* filename = (char*)malloc(sizeof(char) * 100);
	sprintf(filename, "worlds\\world%d_%d", id, cities);
	newfile = fopen(filename, "w");
	fprintf(newfile,"%d\n", cities);
	for (int i = 0; i < cities; i++) {
		int a = rand() % 101;
		int b = rand() % 101;
		fprintf(newfile, "%d %d %d\n", i, a, b);
	}
	free(filename);
	fclose(newfile);
}

void genall() {
	for (int i = 10; i <= 100; i += 25) {
		for (int j = 0; j < 25; j++) {
			generatesingle(j, i);
		}
		if (i == 10) i = 0;
	}
}