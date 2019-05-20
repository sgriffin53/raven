#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <inttypes.h>
#include <limits.h>
#include "globals.h"
#include "hash.h"
#include "TT.h"
#include "makemove.h"
#include "movegen.h"
#include "PST.h"
#include "eval.h"
#include "search.h"
#include "perft.h"
#include "position.h"
#include "tests.h"

int main() {
	setbuf(stdout, NULL);
	setbuf(stdin, NULL);
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stdin, NULL, _IONBF, 0);

	char instr[8192];
	char splitstr[1000][200];
	char * token;
	int wtime, btime;
	int movetime;
	int keeprunning = 1;
	struct position pos;
	parsefen(&pos, "startpos"); // set start position
	posstack[0] = pos;
	posstackend = 1;

	initZobrist();

	//initPTT(&PTT);
	initTT(&TT);
	clearTT(&TT);
	//initETT(&ETT);

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
		if (strcmp(splitstr[0],"test") == 0) {
			if (strcmp(splitstr[1],"silent") == 0) {
				testRunAll(1);
			}
			else testRunAll();
			parsefen(&pos, "startpos");
			posstack[0] = pos;
		}
		if (strcmp(splitstr[0],"setoption") == 0) {
			if ((strcmp(splitstr[1],"name") == 0) && (strcmp(splitstr[3],"value") == 0))   {
				char name[128];
				char value[128];
				strcpy(name,splitstr[2]);
				strcpy(value,splitstr[4]);
				if (strcmp(strlwr(name),"hash") == 0) {
					hashsize = atoi(value);
				}
				
			}
		}
		if (strcmp(splitstr[0],"ucinewgame") == 0) {
			clearTT(&TT);
		}
		
		if (strcmp(splitstr[0],"legalmoves") == 0) {

			struct move moves[MAX_MOVES];
			int num_moves = genLegalMoves(&pos,moves);
			//sortMoves(&pos,moves,num_moves);
			for (int i = 0;i < num_moves;i++) {
				makeMove(&moves[i], &pos);
				pos.tomove = !pos.tomove;
				int incheck = isCheck(&pos);
				if (incheck) {
					unmakeMove(&pos);
					continue;
				}
				pos.tomove = !pos.tomove;
				unmakeMove(&pos);
				printf("%s ",movetostr(moves[i]));
				fflush(stdout);
			}
			printf("\n");
		}

		else if (strcmp(splitstr[0],"quit") == 0) keeprunning = 0;

		else if (strcmp(splitstr[0],"hash") == 0) {
			U64 hash = generateHash(&pos);
			printf("%" PRIu64 "\n",hash);
		}

		else if (strcmp(splitstr[0],"go") == 0) {
			int searchdepth = 100;
			//movetime = 2147483646;
			movetime = INT_MAX / 100;

			if (strcmp(splitstr[1],"depth") == 0) {
				searchdepth = atoi(splitstr[2]);
			}
			assert(searchdepth >= 1);
			wtime = -1;
			btime = -1;
			for (int i = 1;i < splitstrend;i++) {
				if (strcmp(splitstr[i],"wtime") == 0) {
					wtime = atoi(splitstr[i+1]);
				}
				if (strcmp(splitstr[i],"btime") == 0) {
					btime = atoi(splitstr[i+1]);
				}
			}

			if (pos.tomove == WHITE) {
				if (wtime != -1) movetime = wtime / 25;
			}
			else {
				if (btime != -1) movetime = btime / 25;
			}

			if (strcmp(splitstr[1],"movetime") == 0) {
				movetime = atoi(splitstr[2]);
			}

			nodesSearched = 0;

			search(pos,searchdepth,movetime);

			//printf("bestmove %s\n",movetostr(bestmove));
		}

		else if (strcmp(splitstr[0],"isready") == 0) {
			printf("readyok\n");
		}

		else if (strcmp(splitstr[0],"uci") == 0) {
			printf("id name Raven 0.30\nid author Steve Griffin\n");
			printf("option name Hash type spin default 32 min 32 max 256\n");
			printf("uciok\n");
		}

		else if (strcmp(splitstr[0],"board") == 0) dspboard(&pos);

		else if (strcmp(splitstr[0],"move") == 0) makeMovestr(splitstr[1], &pos);

		else if (strcmp(splitstr[0],"unmove") == 0) unmakeMove(&pos);

		else if (strcmp(splitstr[0],"eval") == 0) {
			//printf("score: %d",taperedEval(&pos));
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
				printf("info depth %d nodes %" PRIu64 " time %f nps %" PRIu64 "\n",i,pnodes,time_spent,nps);
				fflush(stdout);
			}

			printf("nodes %" PRIu64 "\n", pnodes);
			fflush(stdout);
		}

		else if (strcmp(splitstr[0],"PST") == 0) {
			char piece = splitstr[1][0];
			printf("%d\n",PSTval(piece, atoi(splitstr[2]),'O'));
		}

		else if (strcmp(splitstr[0],"rank") == 0) printf("%d\n",getrank(atoi(splitstr[1])));

		else if (strcmp(splitstr[0],"file") == 0) printf("%d\n",getfile(atoi(splitstr[1])));

		else if (strcmp(splitstr[0],"sperft") == 0) {
			int depth = atoi(splitstr[1]);
			splitperft(&pos,depth);
		}
		else if (strcmp(splitstr[0],"moves") == 0) {
			for (int i = 1;i < splitstrend;i++) {
				// make move
				makeMovestr(splitstr[i],&pos);
			}
		}
		else if ( (strcmp(splitstr[0],"position") == 0) && (strcmp(splitstr[1],"fen") == 0) ) {
			char fen[1024] = "";
			int readingfen = 1;
			for (int i = 2;i < splitstrend;i++) {
				if (!readingfen) {
					makeMovestr(splitstr[i],&pos);
				}
				if (strcmp(splitstr[i],"moves") == 0) {
					parsefen(&pos, fen);
					readingfen = 0;
				}
				if (readingfen) {
					strcat(fen,splitstr[i]);
					if (i != (splitstrend - 1)) strcat(fen," ");
				}
			}
			if (readingfen) {
				parsefen(&pos, fen);
			}
			posstack[0] = pos;
		}
		else if ( (strcmp(splitstr[0],"position") == 0) && (strcmp(splitstr[1],"startpos") == 0) ) {
			parsefen(&pos, "startpos"); // set start position
			posstack[0] = pos;
			posstackend = 1;
			if (strcmp(splitstr[2],"moves") == 0) {
				// make all moves given by position command
				for (int i = 3;i < splitstrend;i++) {
					// make move
					makeMovestr(splitstr[i],&pos);
				}
			}
		}

	}
	//free(TT.entries);
	//free(PTT.entries);
	//free(ETT.entries);
	return 0;
}
