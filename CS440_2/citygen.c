#include <stdio.h>

void generatesingle(int id, int cities) {
	FILE* newfile;
	char* filename = (char*)malloc(sizeof(char) * 100);
	sprintf(filename, "world%2d%3d", id, cities);
	newfile = fopen(filename, "w");
	fprintf(newfile,"%d\n", cities);
	for (int i = 0; i < cities; i++) {
		fprintf(newfile, "%d %d %d", i, rand() % 101, rand() % 101);
	}

}

void genall() {
	for (int i = 10; i <= 100; i += 25) {
		for (int j = 0; j < 25; j++) {
			generatesingle(j, i);
		}
		if (i == 10) i = 0;
	}
}