#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>
#include <math.h>
#include <limits.h>
#include "globals.h"
#include "uci.h"
#include "search/perft.h"
#include "search/search.h"
#include "search/eval.h"
#include "tests.h"
#include "misc.h"
#include "chess/position.h"
#include "chess/move.h"
#include "chess/makemove.h"
#include "chess/movegen.h"
void uci_listen() {
	int keeprunning = 1;
	struct position pos;
	parsefen(&pos, "startpos");
	char instr[8192];
	char splitstr[1000][200];
	char* token;
	while (keeprunning) {
		// read input from stdin
		fgets(instr, 8192, stdin);
		// removes newline character from input
		instr[strcspn(instr, "\n")] = 0;
		//split instr into tokens into splitstr by space
		token = strtok(instr, " ");

		int splitstrend = 0;
		while (token != NULL) {
			strcpy(splitstr[splitstrend], token);
			splitstrend++;
			token = strtok(NULL, " ");
		}

		// uci commands

		if (strcmp(splitstr[0], "uci") == 0) {
			printf("id name Raven 1.20\nid author Steve Griffin\n");
			printf("option name Hash type spin default 32 min 32 max 256\n");
			printf("uciok\n");
		}
		else if (strcmp(splitstr[0], "setoption") == 0) {
			if ((strcmp(splitstr[1], "name") == 0) && (strcmp(splitstr[3], "value") == 0)) {
				char name[128];
				char value[128];
				strcpy(name, splitstr[2]);
				strcpy(value, splitstr[4]);
				if (strcmp(name, "hash") == 0 || strcmp(name, "Hash") == 0) {
					hashsize = atoi(value);
					free(TT.entries);
					initTT(&TT);
				}

			}
		}
		else if (strcmp(splitstr[0], "ucinewgame") == 0) {
			clearTT(&TT);
			clearETT(&ETT);
			clearKillers(128);
			clearHistory();
			movestogo = 25;
		}
		else if (strcmp(splitstr[0], "go") == 0) {
			int searchdepth = 100;
			//movetime = 2147483646;
			int movetime = INT_MAX / 100;

			if (strcmp(splitstr[1], "depth") == 0) {
				searchdepth = atoi(splitstr[2]);
			}
			assert(searchdepth >= 1);
			int strictmovetime = 0;
			wtime = -1;
			btime = -1;
			for (int i = 1; i < splitstrend; i++) {
				if (strcmp(splitstr[i], "wtime") == 0) {
					wtime = atoi(splitstr[i + 1]);
					if (origwtime == -1) origwtime = wtime;
				}
				if (strcmp(splitstr[i], "btime") == 0) {
					btime = atoi(splitstr[i + 1]);
					if (origbtime == -1) origbtime = btime;
				}
				if (strcmp(splitstr[i], "movestogo") == 0) {
					movestogo = atoi(splitstr[i + 1]);
				}
			}
			if (pos.tomove == WHITE) {
				if (wtime != -1) movetime = wtime / min(25, max(2, movestogo));
			}
			else {
				if (btime != -1) movetime = btime / min(25, max(2, movestogo));
			}
			if (strcmp(splitstr[1], "movetime") == 0) {
				movetime = atoi(splitstr[2]) * .95;
				strictmovetime = 1;
			}
			nodesSearched = 0;
			search(pos, searchdepth, movetime, strictmovetime);
		}
		else if (strcmp(splitstr[0], "isready") == 0) {
			printf("readyok\n");
		}
		else if (strcmp(splitstr[0], "quit") == 0) {
			keeprunning = 0;
		}
		else if ((strcmp(splitstr[0], "position") == 0) && (strcmp(splitstr[1], "startpos") == 0)) {
			parsefen(&pos, "startpos"); // set start position
			movestackend = 0;
			if (strcmp(splitstr[2], "moves") == 0) {
				// make all moves given by position command
				for (int i = 3; i < splitstrend; i++) {
					// make move
					makeMovestr(splitstr[i], &pos);
				}
			}
		}
		else if ((strcmp(splitstr[0], "position") == 0) && (strcmp(splitstr[1], "fen") == 0)) {
			char fen[1024] = "";
			int readingfen = 1;
			for (int i = 2; i < splitstrend; i++) {
				if (!readingfen) {
					makeMovestr(splitstr[i], &pos);
				}
				if (strcmp(splitstr[i], "moves") == 0) {
					parsefen(&pos, fen);
					readingfen = 0;
				}
				if (readingfen) {
					strcat(fen, splitstr[i]);
					if (i != (splitstrend - 1)) strcat(fen, " ");
				}
			}
			if (readingfen) {
				parsefen(&pos, fen);
			}
			posstack[0] = pos;
		}

		// raven-specific commands

		else if (strcmp(splitstr[0], "test") == 0) {
			if (strcmp(splitstr[1], "flip") == 0) {
				runTestsFlip();
			}
			else if (strcmp(splitstr[1], "nps") == 0) {
				runTestsNPS();
			}
			else runTestsAll();
		}
		else if (strcmp(splitstr[0], "flip") == 0) {
			pos = flipBoard(&pos);
			printf("Flipped board\n");
		}
		else if (strcmp(splitstr[0], "col") == 0) {
			printf("%d\n", getColour(&pos, atoi(splitstr[1])));
		}
		else if (strcmp(splitstr[0], "moves") == 0) {
			for (int i = 1; i < splitstrend; i++) {
				// make move
				makeMovestr(splitstr[i], &pos);
			}
		}
		else if (strcmp(splitstr[0], "eval") == 0) {
			printf("tapered: %d\n", taperedEval(&pos));
		}
		else if (strcmp(splitstr[0], "perft") == 0) {
			int depth;
			U64 pnodes;
			U64 nps;
			depth = atoi(splitstr[1]);
			for (int i = 1; i <= depth; i++) {
				clock_t begin = clock();
				pnodes = perft(&pos, i);
				clock_t end = clock();
				double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
				nps = pnodes / time_spent;
				printf("info depth %d nodes %" PRIu64 " time %d nps %" PRIu64 "\n", i, pnodes, (int)(1000 * time_spent), nps);
			}

			printf("nodes %" PRIu64 "\n", pnodes);
		}
		else if (strcmp(splitstr[0], "sperft") == 0) {
			int depth = atoi(splitstr[1]);
			splitperft(&pos, depth);
		}
		else if (strcmp(splitstr[0], "move") == 0) {
			makeMovestr(splitstr[1], &pos);
		}
		else if (strcmp(splitstr[0], "legalmoves") == 0) {
			struct move moves[MAX_MOVES];
			int num_moves = genMoves(&pos, moves, 0);
			int j;
			printf("%d num moves\n", num_moves);
			printf("%d --\n", num_moves);
			for (j = 0; j < num_moves; j++) {
				printf("%d - %s\n", j, movetostr(moves[j]));
			}
		}
		else if (strcmp(splitstr[0], "unmove") == 0) {
			unmakeMove(&pos);
		}
		else if (strcmp(splitstr[0], "dspBB") == 0) {
			printf("Displaying %s bitboard.\n", splitstr[1]);
			dspBBstr(splitstr[1], pos);
		}
		else if (strcmp(splitstr[0], "board") == 0) {
			dspBoard(&pos);
		}
		else if (strcmp(splitstr[0], "set") == 0) {
			//setPiece(&pos,atoi(splitstr[1]),splitstr[2][0]);;
		}
		else if (strcmp(splitstr[0], "piece") == 0) {
			printf("%c\n", getPiece(&pos, atoi(splitstr[1])));
		}
	}
}