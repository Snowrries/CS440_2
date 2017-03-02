#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <time.h>

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

//Checks how many clauses are satisfied by the input. O(n)
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
int* selection(int *csat) {
	int sum = 0;
	int randomint;
	int* normalizedcsat = (int*)malloc(sizeof(int) * 10);
	int* ret = (int*)malloc(sizeof(int) * 8);
	for (int i = 0; i < 10; i++) {
		sum += csat[i];
	}
	for (int i = 0; i < 10; i++) {
		normalizedcsat[i] = (100 * csat[i]) / sum;
	}
	for (int i = 1; i < 10; i++) {
		normalizedcsat[i] += normalizedcsat[i - 1];
	}
	for (int i = 0; i < 8; i++) {
		randomint = rand() % 100;
		for (int j = 0; j < 10; j++) {
			if (normalizedcsat[j] > randomint) {
				ret[i] = j;
				break;
			}
		}
	}
	free(normalizedcsat);
	return ret;
}
double* solve() {
	clock_t begin = clock();
	int** states = (int**)malloc(sizeof(int*) * 10);
	int** statestemp = (int**)malloc(sizeof(int*) * 10);
	int* csat = (int*)malloc(sizeof(int) * 10);
	int* selected;
	int* toRemove = (int*)malloc(sizeof(int) * 10);
	int* temp1, temp2;
	double* retval = (double*)malloc(sizeof(double) * 2);
	double bitflips = 0;
	int e1, e2, temp, tempa, improved;
	e1 = 0; e2 = 0;
	int i, j;

	for (i = 0; i < 10; i++) {
		states[i] = (int*)malloc(sizeof(int) * variables);
		for (j = 0; j < variables; j++) {
			states[i][j] = rand() % 2;
		}
		csat[i] = clausesSatisfied(states[i]);
		if (csat[i] > e1) {
			e1 = csat[i];
		}
	}
	while (e1 < clauses) {
		//Seperate elites
		for (i = 0; i < 10; i++) {
			if (csat[i] > e1) {
				e2 = e1;
				e1 = csat[i];
			}
		}

		//Perform selection
		selected = selection(csat);

		//Save elite states and do some copying so we can free the unused states
		statestemp[8] = states[e1];
		statestemp[9] = states[e2];

		for (i = 0; i < 8; i++) {
			statestemp[i] = states[selected[i]];
		}
		for (i = 0; i < 8; i++) {
			states[selected[i]] = NULL;
		}
		states[e1] = NULL;
		states[e2] = NULL;
		//Memory management is important.
		for (i = 0; i < 10; i++) {
			if (states[i] != NULL) {
				free(states[i]);
			}
			states[i] = statestemp[i];
		}

		//Perform uniform crossover
		//For each of the 4 selected pairs, swap a variable with a 50% chance
		//Operations performed: 4 * variables
		//Elite states do not participate in crossover
		for (i = 0; i < 7; i += 2) {
			for (j = 0; j < variables; j++) {
				if (rand() % 2) {
					temp = states[i][j];
					states[i][j] = states[i + 1][j];
					states[i + 1][j] = temp;
					bitflips++;
				}
			}
		}

		//Perform disruptive mutation

		for (i = 0; i < 7; i++) {
			if ((rand() % 10) < 8) {
				for (j = 0; j < variables; j++) {
					if (rand() % 2) {
						states[i][j] = !states[i][j];
						bitflips++;
					}
				}
			}
		}

		//Flip Heuristic
		// 8*variable calls to clausesSatisfied, which iterates over all clauses
		for (i = 0; i < 8; i++) {
			improved = 1;
			temp = csat[i];
			while (improved) {
				improved = 0;
				for (j = 0; j < variables; j++) {
					states[i][j] = !states[i][j];
					bitflips++;
					tempa = clausesSatisfied(states[i]);
					if (temp > tempa) {
						improved = 1;
						states[i][j] = !states[i][j];
						bitflips++;
					}
					else {
						temp = tempa;
					}
				}
			}
			csat[i] = temp;
		}
	}
	for (i = 0; i < 10; i++) {
		free(states[i]);
	}
	free(states);
	free(statestemp);
	free(csat);
	free(selected);
	free(toRemove);

	retval[0] = (double)(clock() - begin) / CLOCKS_PER_SEC;
	retval[1] = bitflips;
	return retval;
}

int main() {
	double* answer;
	for (int j = 20; j <= 100; j += 25) {
		//Solve 100 instances for each of 20, 50, 75, 100 variable instances
		for (int i = 1; i < 101; i++)
		{
			readInput(j, i);
			//We would like runtime and bit flips. 
			//answer[0] is runtime, answer[1] is bitflips
			answer = solve();
			//Write output to file

		}
		if (j == 20) j += 5;
	}
	return 0;
}