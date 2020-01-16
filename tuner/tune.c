#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tune_eval.h"
#include "tune.h"
#include "../position.h"
#include <time.h>
#include <inttypes.h>

float sigmoid(const float s, float k) {
    return 1.0 / (1.0 + pow(10, s * -k / 400.0));
}

float E(
        struct tunerEntry *entries, int numentries, float k) {

    float sum = 0.0;

    for (int i = 0;i < numentries;i++) {
		struct tunerEntry entry = entries[i];
		int evalscore = tune_eval(&entry.pos);
        if (entry.pos.tomove == BLACK) {
            evalscore = -evalscore;
        }
        float n = entry.score - sigmoid(evalscore, k);
        sum += n * n;

    }

    return sum;
}
void findK(struct tunerEntry *entries, int numentries) {
	float minK = INT_MAX;
	float bestE = INT_MAX;
	for (float i = 0.1;i < 2.4;i+=0.05) {
		float newE = E(entries, numentries, i);
		printf("%f %f\n",i, minK);
		if (newE < bestE) {
			minK = i;
			bestE = newE;
		}
	}
}
void tune() {
	
	printf("Loading positions.\n");
	// Load .epd
	
	char path[256] = "tuner\\quiet-labeled_orig.epd";
	//char path[256] = "tuner\\big3.epd";
	//char path[256] = "tuner\\Stockfish.epd";
	struct tunerEntry entries[10000000];
	
	char *token;
	char splitstr[12][200];
	
    FILE *fp;
    char str[1024];
	
    fp = fopen(path, "r");
    if (fp == NULL){
        printf("Could not open file %s",path);
    }
	
	int numentries = 0;
	// read entries
	 
    while (fgets(str, 1024, fp) != NULL) {
		//if (numentries >= 4000000) break; // limit number of entries
        //printf("%s", str);
		char fen[1024];
		str[strcspn(str, "\n")] = 0;
		//split str into tokens into splitstr by space
		token = strtok(str," ");

		int splitstrend = 0;
		while (token != NULL) {
			strcpy(splitstr[splitstrend],token);
			splitstrend++;
			token = strtok(NULL, " ");
		}
		
		strcpy(fen,splitstr[0]);
		strcat(fen, " ");
		strcat(fen, splitstr[1]);
		strcat(fen, " ");
		strcat(fen, splitstr[2]);
		strcat(fen, " ");
		strcat(fen, splitstr[3]);
		
		float score = -2.0;
		char result[1024];
		strcpy(result, splitstr[4]);
		
		// for stockfish, ethereal, laser, alona
		/*
		if (strcmp(result,"1/2-1/2") == 0) {
			score = 0.5;
		} else if (strcmp(result,"1-0") == 0) {
			score = 1.0;
		} else if (strcmp(result,"0-1") == 0) {
			score = 0.0;		
		}
		*/
		// for big3.epd
		/*
		for (int i = 3;i < 100;i++) {
			if (strcmp(splitstr[i],"c2") == 0) {
				strcpy(result,splitstr[i+1]);
				if (strcmp(result,"\"0.000\";") == 0) {
					score = 0.0;
				}
				else if (strcmp(result,"\"0.500\";") == 0) {
					score = 0.5;
				}
				else if (strcmp(result,"\"1.000\";") == 0) {
					score = 1.0;
				}
				break;
			}
		}
		
		*/
		
		// for quiet-labelled.epd
			
		if (strcmp(result,"\"1/2-1/2\";") == 0) {
			score = 0.5;
		} else if (strcmp(result,"\"1-0\";") == 0) {
			score = 1.0;
		} else if (strcmp(result,"\"0-1\";") == 0) {
			score = 0.0;		
		}
		
		struct position pos;
		parsefen(&pos, fen);
		entries[numentries].pos = pos;
		entries[numentries].score = score;
		if (numentries % 100000 == 0) printf("Loaded %d positions.\n", numentries);
		numentries++;
		
	}
	printf("Loaded %d positions\n", numentries);
	//printf("test");
    fclose(fp);
	//for (int i = 0;i < 20;i++) printf("%f\n",entries[i].score);
	//printf("%f.2", entries[10].score);
	//findK(entries, numentries);
	
	// Best K values:
	
	// big3 - 0.50
	
	float k = 0.50;
	
	setParams(); // set initial parameters
	
	int increments[5] = { 1, 2, 5, 10, 20 };
	int incidx = 4;
	int inc = increments[incidx];
	
	float bestE = E(entries, numentries, k);
	
	clock_t start = clock();
	
	int max = 5150;
	
	int improved = 1;
	
	int num_iterations = 0;
	
	while (improved) {
		
		improved = 0;
		num_iterations++;
		
		if (num_iterations % 1 == 0) {
			clock_t now = clock();
			printf("Values: ");
			printf("\n");
			for (int i = 0;i < 63;i++) {
				printf("%s: ", tuneParamsName[i]);
				printf("%d -> %d\n", tuneParamsInit[i], tuneParams[i]);
				//if (tuneParams[i] != tuneParamsInit[i]) printf("(changed)\n");
				//else printf("\n");
			}
			printf("\n");
			printf("Iteration: %d\n", num_iterations);
			printf("Error: %f\n", bestE);
			printf("Time: %d\n", (int)((now - start) / 1000));
			printf("Inc: %d\n", inc);
			printf("-----\n");
		}
		
		for (int i = 0;i < 63;i++) {
			if (i % 5 == 0) printf("tuning parameter: %d/72\n",i);
			//int oldTuneParam = tuneParams[i];
			if (tuneParams[i] > max) continue;
			tuneParams[i] += inc;
			float newE = E(entries, numentries, k);
			if (newE < bestE) {
				bestE = newE;
				improved = 1;
				printf("Changed %s to %d.\n",tuneParamsName[i], tuneParams[i]);
			}
			else {
				tuneParams[i] -= 2 * inc;
				newE = E(entries, numentries, k);
				if (newE < bestE) {
					bestE = newE;
					improved = 1;
					printf("Changed %s to %d.\n",tuneParamsName[i], tuneParams[i]);
				}
				else {
					tuneParams[i] += inc;
				}
			}
			//if (oldTuneParam != tuneParams[i]) {
			//	printf("Changed %s from %d to %d.\n",tuneParamsName[i], oldTuneParam, tuneParams[i]);
			//}
		}
		
		// Lower increment if we can
		
		if (!improved && incidx > 0) {
			printf("Reducing inc size from %d to %d\n",increments[incidx],increments[incidx - 1]);
			incidx -= 1;
			inc = increments[incidx];
			improved = 1;
		}
	}
	clock_t now = clock();
	printf("Finished tuning:\n");
	printf("-----\n");
	printf("Iteration: %d\n", num_iterations);
	printf("Error: %f\n", bestE);
	printf("\n");
	printf("Time: %d\n", (int)(now - start));
	printf("-----\n");
	
}