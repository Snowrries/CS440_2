#include <stdio.h>
#include <stdarg.h>
#include <math.h>

int variables, clauses;
char* format;
int* allClauses;

int readInput(int vars, int i) {
	char* filename;
	FILE* problems;
	char* buffer;
	char check;
	format = (char*)malloc(sizeof(char) * 4); //cnf
	filename = (char*)malloc(sizeof(char) * 500);
	sprintf(filename, "uf%d-0%d.cnf",vars, i);
	problems = fopen(filename, "r");
	fgets(buffer, 500, problems);

	while (buffer[0] == 'c') {
		fgets(buffer, 500, problems);
	}

	fscanf(problems, "%c %s %d %d", check, format, variables, clauses);
	if (check != 'p' || format[0] != 'c' || format[1] != 'n' || format[2] != 'f') {
		printf("File format error.\n");
		return -1;
	}
	allClauses = (int*)malloc(sizeof(int)*clauses * 3);
	int shouldBeZero;
	for (int j = 0; j < clauses * 3; j += 3) {
		//repeat once for every clause
		//Last one is a zero. Tailored only to work with 3 CNF
		fscanf(problems, "%d %d %d %d", allClauses[j], allClauses[j + 1], allClauses[j + 2], shouldBeZero);
		if (shouldBeZero != 0) {
			printf("3-CNF format exception.");
			return -2;
		}
	}

	return 0;
}
int clausesSatisfied(int* test) {
	int a, b, c;
	int t1, t2, t3;
	int gather;
	int mask;
	int sat = 0;
	for (int i = 0; i < clauses; i++) {
		a = allClauses[clauses * 3];
		b = allClauses[clauses * 3 + 1];
		c = allClauses[clauses * 3 + 2];
		t1 = test[Abs(a)];
		t2 = test[Abs(b)];
		t3 = test[Abs(c)];
		//t1, t2, and t3 will be 1 or 0.

		gather = 0;
		if (a > 0)
			gather = gather | (1 << 1);
		if (b > 0)
			gather = gather | (1 << 2);
		if (c > 0)
			gather = gather | (1 << 3);

		mask = 0;
		if (t1) // nonzero
			mask = mask | (1 << 1);
		if (t2)
			mask = mask | (1 << 2);
		if (t3)
			mask = mask | (1 << 3);

		if ((gather ^ mask) == 0) //If gather xor mask is zero, the clause is satisfied
			sat++;

	}
	return sat;
}

void solve() {
	int** states = (int**)malloc(sizeof(int*) * 10);
	for (int i = 0; i < 10; i++) {
		states[i] = (int*)malloc(sizeof(int) * variables);
	}










}

int main() {
	//For uf20
	for (int i = 0; i < 1000; i++)
	{
		readInput(20, i);
		solve();


	}





	return 0;
}