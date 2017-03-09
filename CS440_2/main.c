#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <time.h>
#include <direct.h>
#include <windows.h>

HANDLE ghMutex;




struct package {
	int i;
	int j;
	HANDLE mutex;
};

int readInput(int vars, int i, int** allClauses, int* variables, int* clauses) {
	FILE* problems;
	char* buffer = (char*) malloc( sizeof(char) * 2000);
	char* format = (char*)malloc(sizeof(char) * 4); //cnf
	char* filename = (char*)malloc(sizeof(char) * 50);
	char check;
	sprintf(filename, "uf%d\\uf%d-0%d.cnf",vars, vars, i);
	problems = fopen(filename, "r");
	fgets(buffer, 1000, problems);

	while (buffer[0] == 'c') {
		fgets(buffer, 1000, problems);
	}

	if (sscanf(buffer, "%c %s %d %d", &check, format, variables, clauses) < 0) {
		printf("error in scanf for problem statement");
		return -1;
	}
	if (check != 'p' || format[0] != 'c' || format[1] != 'n' || format[2] != 'f') {
		printf("File format error.\n");
		return -1;
	}
	*allClauses = (int*)malloc(sizeof(int)*(*clauses) * 3);
	int* ac = *allClauses;
	int shouldBeZero;
	for (int j = 0; j < (*clauses); j++) {
		//repeat once for every clause
		//Last one is a zero. Tailored only to work with 3 CNF
		if (fscanf(problems, "%d %d %d %d", &ac[j * 3], &ac[j * 3 + 1], &ac[j * 3 + 2], &shouldBeZero) < 0) {
			printf("Error in fscanf of clause %d", j);
			return -1;
		}
		if (shouldBeZero != 0) {
			printf("3-CNF format exception.");
			return -2;
		}
	}
	fclose(problems);
	free(format);
	free(buffer);
	free(filename);
	return 0;
}

//Checks how many clauses are satisfied by the input. O(n)
int clausesSatisfied(int* test, int*allClauses, int clauses) {
	int a, b, c;
	int t1, t2, t3;
	int d, e, f;
	int gather;
	int mask;
	int sat = 0;
	for (int i = 0; i < clauses; i++) {
		
		a = allClauses[i * 3];
		b = allClauses[i * 3 + 1];
		c = allClauses[i * 3 + 2];

		t1 = test[abs(a)-1];
		t2 = test[abs(b)-1];
		t3 = test[abs(c)-1];
		
		d = 0; e = 0; f = 0;
		if (a > 0) d = 1;
		if (b > 0) e = 1;
		if (c > 0) f = 1;
		if ((t1 == d) || (t2 == e) || (t3 == f))
			sat++;
		
		/*/
		a = allClauses[i * 3];
		b = allClauses[i * 3 + 1];
		c = allClauses[i * 3 + 2];
		t1 = test[abs(a)-1];
		t2 = test[abs(b)-1];
		t3 = test[abs(c)-1];
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
		/**/

	}
	return sat;
}

void selection(int *csat, int* ret) {
	int sum = 0;
	int randomint;
	int* normalizedcsat = (int*)malloc(sizeof(int) * 10);
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
		ret[i] = -1;
		for (int j = 0; j < 10; j++) {
			if (normalizedcsat[j] >= randomint) {
				ret[i] = j;
				break;
			}
		}
		if (ret[i] == -1) ret[i] = 9;
	}
	free(normalizedcsat);
}

double* solve(int* allClauses, int variables, int clauses) {
	clock_t begin = clock();
	//We keep 10 states, and there are 20, 50, 75, or 100 variables for each state.
	int** states = (int**)malloc(sizeof(int*) * 10);
	int** statestemp = (int**)malloc(sizeof(int*) * 10);
	int* csat = (int*)malloc(sizeof(int) * 10);
	double* retval = (double*)malloc(sizeof(double) * 2);
	int* randomScan = (int*)malloc(sizeof(int) * variables);
	int* selected = (int*)malloc(sizeof(int) * 8);

	double bitflips = 0;
	int forrs, e1, e2, ei1, ei2, temp, tempa, improved;
	
	e1 = 0; e2 = 0;
	ei1 = 0; ei2 = 0;
	int i, j;

	for (i = 0; i < 10; i++) {
		states[i] = (int*)malloc(sizeof(int) * variables); 
		statestemp[i] = (int*)malloc(sizeof(int) * variables);
		for (j = 0; j < variables; j++) {
			states[i][j] = rand() % 2;
		}
		csat[i] = clausesSatisfied(states[i], allClauses, clauses);
		if (csat[i] > e1) {
			e1 = csat[i];
			ei1 = i;
		} // In case we randomly generate a satisfying state
	}
	while (e1 < clauses) {

		//Seperate elites
		if (csat[0] > csat[1]) {
			e1 = csat[0]; e2 = csat[1];
			ei1 = 0; ei2 = 1;
		}
		else {
			e2 = csat[0]; e1 = csat[1];
			ei2 = 0; ei1 = 1;
		}
		for (i = 2; i < 10; i++) {
			if (csat[i] > e2) {
				if (csat[i] > e1) {
					e2 = e1;
					e1 = csat[i];
					ei2 = ei1;
					ei1 = i;
				}
				else {
					e2 = csat[i];
					ei2 = i;
				}
			}
		}

		//Perform selection
		selection(csat, selected);
		//Save elite states and do some copying so we can f r e e the unused states
		//statestemp[8] = states[e1];
		//statestemp[9] = states[e2];
		memcpy(statestemp[8], states[ei1], sizeof(int)*variables);
		memcpy(statestemp[9], states[ei2], sizeof(int)*variables);

		for (i = 0; i < 8; i++) {
			//statestemp[i] = states[selected[i]];
			memcpy(statestemp[i], states[selected[i]], sizeof(int)*variables);
		}
		// Temporary array now contains the values we desire.
		// Copy states back to the main

		for (i = 0; i < 10; i++) {
			memcpy(states[i], statestemp[i], sizeof(int)*variables);
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
		for (i = 0; i < 8; i++) {
			if ((rand() % 10) < 8) { //90% chance of mutation
				for (j = 0; j < variables; j++) {
					if (rand() % 2) {
						states[i][j] = !states[i][j];
						bitflips++;
					}
				}
			}
		}

		//Update csat
		for (i = 0; i < 10; i++) {
			csat[i] = clausesSatisfied(states[i], allClauses, clauses);
		}
		//Flip Heuristic
		// 8*variable calls to clausesSatisfied, which iterates over all clauses
		for (i = 0; i < 8; i++) {
			improved = 1;
			temp = csat[i];
			while (improved) {
				forrs = 1;
				improved = 0;
				for (int j = 0; j < variables; j++) {
					randomScan[j] = 0;
				}
				int selectedvar = -1;
				while (forrs < variables) {
					selectedvar = rand() % (variables - 1);
					while (randomScan[selectedvar]) {
						selectedvar = (selectedvar+1)%(variables-1);
					}
					randomScan[selectedvar] = 1;
					forrs++;
					states[i][selectedvar] = !states[i][selectedvar];
					bitflips++;
					tempa = clausesSatisfied(states[i], allClauses, clauses);
					if (temp >= tempa) {
						improved = 0;
						states[i][selectedvar] = !states[i][selectedvar];
						bitflips++;
					}
					else {
						temp = tempa;
						improved = 1;
					}

				}
			}
			csat[i] = temp;
		}


	}
	for (i = 0; i < 10; i++) {
		free(states[i]);
		free(statestemp[i]);
	}
	free(states);
	free(statestemp);
	free(csat);
	free(selected);
	free(randomScan);

	retval[0] = ((double)(1000*(clock() - begin)) / CLOCKS_PER_SEC);
	retval[1] = bitflips;
	return retval;
}
DWORD WINAPI singleResult(LPVOID package){
	int i = ((struct package*)package)->i;
	int j = ((struct package*)package)->j;
	HANDLE mutex = ((struct package*)package)->mutex;
	int variables;
	int clauses;
	int* allClauses;

	double* answer;
	FILE* resultfile;
	resultfile = fopen("results.txt", "a");
	if (readInput(j, i, &allClauses, &variables, &clauses) < 0) {
		printf("Error in readInput()");
		return -1;
	}
	//We would like runtime and bit flips. 
	//answer[0] is runtime, answer[1] is bitflips
	answer = solve(allClauses, variables, clauses);
	//Write output to file
	WaitForSingleObject( mutex,	INFINITE);  // no time-out interval
	fprintf(resultfile, "%d vars %d file %12.3f ms %12.3f flips\n", j, i, answer[0], answer[1]);
	ReleaseMutex(mutex);
	free(answer);
	free(allClauses);
	free(package);
	fclose(resultfile);
	return 0;
}
int main() {
	for (int j = 75; j <= 100; j += 25) {
		for (int i = 1; i < 101; i++) {
			int variables;
			int clauses;
			int* allClauses;

			double* answer;
			FILE* resultfile;
			resultfile = fopen("results.txt", "a");
			if (readInput(j, i, &allClauses, &variables, &clauses) < 0) {
				printf("Error in readInput()");
				return -1;
			}
			//We would like runtime and bit flips. 
			//answer[0] is runtime, answer[1] is bitflips
			answer = solve(allClauses, variables, clauses);
			//Write output to file
			fprintf(resultfile, "%d vars %d file %12.3f ms %12.3f flips\n", j, i, answer[0], answer[1]);
			free(answer);
			free(allClauses);
			fclose(resultfile);
		}
	}

}
//int main() {
//	HANDLE ghEvents[100];
//	HANDLE ghMutex = CreateMutex(
//		NULL,              // default security attributes
//		FALSE,             // initially not owned
//		NULL);             // unnamed mutex
//
//	if (ghMutex == NULL)
//	{
//		printf("CreateMutex error: %d\n", GetLastError());
//		return 1;
//	}
//	for (int j = 20; j <= 20; j += 25) {
//		HANDLE threadArr[100];
//		DWORD ThreadID;
//		//Solve 100 instances for each of 20, 50, 75, 100 variable instances
//		for (int i = 1; i < 101; i++)
//		{
//
//			struct package* a = (struct package*) malloc(sizeof(struct package));
//			a->i = i; 
//			a->j = j;
//			a->mutex = ghMutex;
//
//			threadArr[i-1] = CreateThread(
//				NULL,       // default security attributes
//				0,          // default stack size
//				(LPTHREAD_START_ROUTINE)singleResult,
//				a,          // thread function arguments
//				0,          // default creation flags
//				&ThreadID); // receive thread identifier
//
//			if (threadArr[i - 1] == NULL)
//			{
//				printf("CreateThread error: %d\n", GetLastError());
//				return 1;
//			}
//
//		}
//		for (int i = 0; i < 100; i++) {
//			DWORD dwEvent = WaitForSingleObject(threadArr[i], INFINITE);
//			switch (dwEvent)
//			{
//			case WAIT_OBJECT_0:
//				printf("Thread %d completed\n", i);
//				break;
//
//			case WAIT_ABANDONED_0:
//				// TODO: Perform tasks required by this event
//				printf("Wait abandoned for some reason.\n");
//				break;
//
//			case WAIT_TIMEOUT:
//				printf("Wait timed out.\n");
//				break;
//
//				// Return value is invalid.
//			default:
//				printf("Wait error: %d\n", GetLastError());
//				//ExitProcess(0);
//			}
//		}
//		scanf("q");
//
//		for (int i = 0; i < 100; i++) {
//			CloseHandle(threadArr[i]);
//		}
//		if (j == 20) j += 5;
//	}
//	CloseHandle(ghMutex);
//	return 0;
//}