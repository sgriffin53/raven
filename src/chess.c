#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <inttypes.h>
#include <time.h>
#include "position.h"
#include "globals.h"
#include "move.h"
#include "makemove.h"
#include "bitboards.h"
#include "perft.h"
#include "tests.h"

int main() {
	setbuf(stdout, NULL);
	setbuf(stdin, NULL);
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stdin, NULL, _IONBF, 0);
	char instr[8192];
	char splitstr[1000][200];
	char * token;
	struct position pos;
	parsefen(&pos,"startpos");
	int keeprunning = 1;
	origwtime = -1;
	origbtime = -1;
	movestogo = 25;
	genLookups();
	while (keeprunning) {
		// read input from stdin
		fgets(instr, 8192, stdin);
		// removes newline character from input
		instr[strcspn(instr, "\n")] = 0;
		//split instr into tokens into splitstr by space
		token = strtok(instr," ");
		int splitstrend = 0;
		while (token != NULL) {
			strcpy(splitstr[splitstrend],token);
			splitstrend++;
			token = strtok(NULL, " ");
		}
		if (strcmp(splitstr[0],"uci") == 0) {
			printf("id name Raven 0.90\nid author Steve Griffin\n");
			//printf("option name Hash type spin default 32 min 32 max 256\n");
			printf("uciok\n");
		}
		else if (strcmp(splitstr[0],"board") == 0) {
			dspBoard(&pos);
		}
		else if (strcmp(splitstr[0],"move") == 0) {
			makeMovestr(splitstr[1],&pos);
		}
		else if (strcmp(splitstr[0],"unmove") == 0) {
			//unMakeMovestr(splitstr[1],&pos);
		}
		else if (strcmp(splitstr[0],"moves") == 0) {
			for (int i = 1;i < splitstrend;i++) {
				// make move
				makeMovestr(splitstr[i],&pos);
			}
		}
		else if (strcmp(splitstr[0],"test") == 0) {
			runTestsMakeMove();
		}
		else if (strcmp(splitstr[0],"perft") == 0) {
			int depth;
			U64 pnodes;
			U64 nps;
			depth = atoi(splitstr[1]);
			for (int i = 1;i <= depth;i++) {
				clock_t begin = clock();
				pnodes = perft(&pos,i);
				clock_t end = clock();
				double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
				nps = pnodes / time_spent;
				printf("info depth %d nodes %" PRIu64 " time %d nps %" PRIu64 "\n",i,pnodes,(int)(1000 * time_spent),nps);
			}

			printf("nodes %" PRIu64 "\n", pnodes);
		}
		else if (strcmp(splitstr[0],"sperft") == 0) {
			int depth = atoi(splitstr[1]);
			splitperft(&pos,depth);
		}
	}
}