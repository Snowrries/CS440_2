#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define min(x,y) ((x < y)?(x:y))
//Declare global result structs
/*a) number of problems solved within the time
threshold, b) average solution time, c) number of nodes generated, and d) average solution
quality, as the size of challenge increases(i.e., average for the 10 - city challenges, then for
	the 25 - city ones, etc.)*/
int* problemsSolved;
float* averageTime;
int* nodesGenerated;
int* averageQuality;
int furthest;

citypair* citypairs;
citypair** iclosed;

double cost(citypair* current, citypair* goal) {
	return sqrt((pow(current->x - goal->x, 2) + pow(current->y - goal->y, 2)));
}
double heuristic(citypair* current, citypair* furthest) {
	return sqrt((pow(current->x - furthest->x, 2) + pow(current->y - furthest->y, 2))) + furthest->distanceFromStart;
}
typedef struct citypair_ {
	int x;
	int y;
	int g;
	double distanceFromStart;
} citypair;

typedef struct pqn_ {
	int id;
	citypair *self;
	pqn *child;
	double f;
} pqn;

int* find_closest(citypair *dqd, int cities) {
	double temp;

	int n1, n2, n3, n4;
	double c1, c2, c3, c4;
	n1 = -1; n2 = -1; n3 = -1; n4 = -1;
	int count = 0;
	for (int i = 0; i < cities; i++) {
		if (iclosed[i] == NULL)
			continue;

		temp = cost(iclosed[i], dqd);
		if (count == 0) {
			count++;
			c1 = temp;
			n1 = i;
		}

		else {
			if (temp < c1) {
				c4 = c3;
				n4 = n3;

				c3 = c2;
				n3 = n2;

				c2 = c1;
				n2 = n1;

				n1 = i;
				c1 = temp;
			}
			else if (temp < c2) {

				c4 = c3;
				n4 = n3;

				c3 = c2;
				n3 = n2;

				c2 = temp;
				n2 = i;
			}
			else if (temp < c3) {
				c4 = c3;
				n4 = n3;

				c3 = temp;
				n3 = i;
			}
			else if (temp < c4) {
				c4 = temp;
				n4 = i;
			}
		}
	}
	int* retval = (int*)malloc(sizeof(int) * 4);
	retval[0] = n1;
	retval[1] = n2;
	retval[2] = n3;
	retval[3] = n4;
	return retval;

}

int readOneProblem(int world, int cities) {
	FILE* newfile;
	char* filename = (char*)malloc(sizeof(char) * 100);
	int temp;
	sprintf(filename, "world%2d%3d", world, cities);
	newfile = fopen(filename, "r");
	pqn* fringehead;
	pqn* tempfree;
	fscanf(newfile, "%*d\n");
	//Ignore the first line, it's just the number of cities in the file

	citypairs = (citypair*)malloc(sizeof(citypair) * cities);
	iclosed = (citypair**)malloc(sizeof(citypair*) * cities); //Stands for implicit closed list.

	furthest = 1;
	for (int i = 0; i < cities; i++) {
		fscanf(newfile, "%*d %d %d", citypairs[i].x, citypairs[i].y);
		citypairs[i].g = 30000; // Essentially infinity
		temp = cost(&citypairs[0], &citypairs[i]);
		citypairs[i].distanceFromStart = temp;
		if (temp > citypairs[furthest].distanceFromStart)
			furthest = i;
		iclosed[i] = &citypairs[i];
	}
	citypairs[0].g = 0;
	fringehead = (pqn*)malloc(sizeof(pqn));
	fringehead->id = 0;
	fringehead->child = NULL;
	fringehead->self = &citypairs[0];

	while (fringehead != NULL) {
		citypair* dqd = fringehead->self;
		iclosed[fringehead->id] = NULL;
		tempfree = fringehead;
		fringehead = fringehead->child;
		free(tempfree);
		//dequeue
		//Do an O(n) check against every node to find the min(4, remaining nodes) closest neighbors, 
		//then enqueue to fringe based on g + heuristic
		//Update g values of the enqueued nodes
		int* neighbors = find_closest(dqd, cities);
		double currentg;
		double cost2next;
		for (int j = 0; j < 4; j++) {
			//The four closest neighbors; enqueue them if the current calculated g value is less than 
			//Make sure the enqueue operation keeps the list in sorted order of f
			if (neighbors[j] != -1) {
				currentg = iclosed[neighbors[j]]->g;
				cost2next = dqd->g + cost(dqd, iclosed[neighbors[j]]);
				if (cost2next < currentg) {
					iclosed[neighbors[j]]->g = cost2next;
					//Enqueue the neighbor into the fringe!
					//Node creation
					pqn* fringenode = (pqn*) malloc(sizeof(pqn));
					fringenode->self = iclosed[neighbors[j]];
					fringenode->f = cost2next + heuristic(iclosed[neighbors[j]], iclosed[furthest]);
					fringehead->child = NULL;
					pqn* anothernode = fringehead;
					//Search for insert index
					if (fringehead == NULL) {
						fringehead = fringenode;
					}
					else {
						// If the current node is less costly than the first node in the list,
						// replace the head reference
						if (fringenode->f < fringehead->f) { 
							fringenode->child = fringehead;
							fringehead = fringenode;
						}
						citypair savedRef;
						while (anothernode != NULL) {
							if (fringenode->f < anothernode->f) {
								fringenode->child = anothernode;
								break;
							}
							if (anothernode->child == NULL) {
								anothernode->child = fringenode;
							}
							anothernode = anothernode->child;
						}
					}
				}
			}
		}
	}

	free(citypairs);
}

int readOneProblem_SA(int world, int cities) {
	int T = 1000;
	citypair current = citypairs[0];
	double E;
	double Enext;
	int swap1, swap2;
	citypair* temp;
	FILE* newfile;
	char* filename = (char*)malloc(sizeof(char) * 100);
	sprintf(filename, "world%2d%3d", world, cities);
	newfile = fopen(filename, "r");
	fscanf(newfile, "%*d\n");
	//Ignore the first line, it's just the number of cities in the file

	*citypairs = (citypair*)malloc(sizeof(citypair) * cities);
	citypair *tempforswaps = citypairs;

	for (int i = 0; i < cities; i++) {
		fscanf(newfile, "%*d %d %d", citypairs[i].x, citypairs[i].y);
	}

	// Initialize path by going down the list in order of the cities that were generated
	// Then return to start city, and record total path length. 
	for (int i = 0; i < cities-1; i++) {
		E += cost(&tempforswaps[i], &tempforswaps[i + 1]);
	}
	E += cost(&tempforswaps[cities - 1], &tempforswaps[0]);



	while (T > 0) {
		// Decrement T in some way
		T--; 
		//Swap two random cities and test if the evaluation is improved
		swap1 = rand() % cities;
		swap2 = rand() % cities;
		temp = citypairs[swap1];
		citypairs[swap1] = citypairs[swap2];
		citypairs[swap2] = citypairs[swap1];

		for (int i = 0; i < cities - 1; i++) {
			Enext += cost(&tempforswaps[i], &tempforswaps[i + 1]);
		}
		Enext += cost(&tempforswaps[cities - 1], &tempforswaps[0]);

		if (Enext < E) {
			citypairs = tempforswaps; // Potentially can't do this, maybe need to make new copy of wrapper and copy refrences and then use this as reference buccket,
		}



	}


	free(filename);
	free(citypairs);
}

void work10() {

	//Init global result structs
	problemsSolved = (int*) malloc(sizeof(int)*4);
	averageTime = (float*)malloc(sizeof(float) * 4);
	nodesGenerated = (int*)malloc(sizeof(int) * 4);
	averageQuality = (int*)malloc(sizeof(int) * 4);
	clock_t countdown;
	int world, cities;

	for (int twice = 0; twice < 2; twice++) {
		for (int i = 0; i < 4; i++) {
			problemsSolved[i] = 0;
			averageTime[i] = 0;
			nodesGenerated[i] = 0;
			averageQuality[i] = 0;
		}

		countdown = clock();
		world = -1;
		cities = 10;
		while ((double)(clock() - countdown) / CLOCKS_PER_SEC < 600) {
			//Do one problem
			world += 1;
			if (world > 24)
			{
				if (cities == 10) {
					cities -= 10;
				}
				if (cities == 100) {
					continue; // Finished all problems
				}
				cities += 25;
			}
			if (twice == 0)
				readOneProblem(world, cities);
			else
				readOneProblem_SA(world, cities);
		}
		//write results to disc

	}

	//free all the things

}