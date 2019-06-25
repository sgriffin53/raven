#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>
#include "TT.h"
#include "position.h"
#include "move.h"
#include "makemove.h"
#include "tests.h"
#include "magicmoves.h"
#include "movegen.h"
#include "perft.h"
#include "globals.h"
#include "sort.h"
#include "search.h"
#include "bitboards.h"
#include <limits.h>

int main() {
	setbuf(stdout, NULL);
	setbuf(stdin, NULL);
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stdin, NULL, _IONBF, 0);
	struct position pos;
	parsefen(&pos,"startpos");
	char instr[8192];
	char splitstr[1000][200];
	char * token;
	int keeprunning = 1;
	int wtime, btime;
	initZobrist();
	initmagicmoves();
	hashsize = 32;
	initTT(&TT);
	clearTT(&TT);
	initETT(&ETT);
	clearETT(&ETT);
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
		if (strcmp(splitstr[0],"setoption") == 0) {
			if ((strcmp(splitstr[1],"name") == 0) && (strcmp(splitstr[3],"value") == 0))   {
				char name[128];
				char value[128];
				strcpy(name,splitstr[2]);
				strcpy(value,splitstr[4]);
				if (strcmp(strlwr(name),"hash") == 0) {
					hashsize = atoi(value);
					free(TT.entries);
					initTT(&TT);
				}
				
			}
		}
		if (strcmp(splitstr[0],"test") == 0) {
			runTestsAll();
		}
		if (strcmp(splitstr[0],"test2") == 0) {
			parsefen(&pos,"8/8/8/8/6k1/K7/8/8 b - -");
			int kingpos = pos.Bkingpos;
			printf("%d\n",isAttacked(&pos, H3, !pos.tomove));
			
		}
		if (strcmp(splitstr[0],"testBB") == 0) {
			U64 BBwhitepawns = (pos.BBwhitepieces & pos.BBpawns);
			while (BBwhitepawns) {
				int square = __builtin_ctzll(BBwhitepawns);
				BBwhitepawns &= ~(1ULL << square);
				U64 BBpiece = (1ULL << square);
				U64 BBmidsquare = BBpiece;
				U64 BBchecksquares = 0ULL;
				int rank = getrank(square);
				//printf("\n%d\n",rank);
				while (rank < 6) {
					BBchecksquares |= noWeOne(BBmidsquare);
					BBchecksquares |= northOne(BBmidsquare);
					BBchecksquares |= noEaOne(BBmidsquare);
					BBmidsquare = northOne(BBmidsquare);
					rank++;
				}
				U64 BBenemypawns = (BBchecksquares & (pos.BBblackpieces & pos.BBpawns));
			}
		}
		if (strcmp(splitstr[0],"magic") == 0) {
			U64 BBrook = Rmagic(E4,pos.BBwhitepieces | pos.BBblackpieces);
			dspBB(BBrook & ~pos.BBblackpieces);
		}
		else if (strcmp(splitstr[0],"moves") == 0) {
			for (int i = 1;i < splitstrend;i++) {
				// make move
				makeMovestr(splitstr[i],&pos);
			}
		}
		if (strcmp(splitstr[0],"ucinewgame") == 0) {
			clearTT(&TT);
			clearETT(&ETT);
		}
		else if (strcmp(splitstr[0],"go") == 0) {
			int searchdepth = 100;
			//movetime = 2147483646;
			int movetime = INT_MAX / 100;

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
		if (strcmp(splitstr[0],"perft") == 0) {
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
		if (strcmp(splitstr[0],"move") == 0) {
			makeMovestr(splitstr[1],&pos);
		}
		if (strcmp(splitstr[0],"legalmoves") == 0) {
			struct move TTmove = {.to=-1,.from=-1,.prom=-1,.cappiece=-1};
			struct move moves[MAX_MOVES];
			int num_moves = genMoves(&pos,moves);
			//sortMoves(&pos, moves, num_moves, TTmove, 0);
			int j;
			printf("%d num moves\n",num_moves);
			printf("%d --\n",num_moves);
			for (j = 0;j < num_moves;j++) {
				printf("%d - %s\n",j,movetostr(moves[j]));
			}
		}
		if (strcmp(splitstr[0],"unmove") == 0) {
			unmakeMove(&pos);
		}
		if (strcmp(splitstr[0],"dspBB") == 0) {
			printf("Displaying %s bitboard.\n",splitstr[1]);
			dspBBstr(splitstr[1],pos);
		}
		if (strcmp(splitstr[0],"board") == 0) {
			dspBoard(&pos);
		}
		if (strcmp(splitstr[0],"set") == 0) {
			setPiece(&pos,atoi(splitstr[1]),splitstr[2][0]);;
		}
		if (strcmp(splitstr[0],"isready") == 0) {
			printf("readyok\n");
		}
		if (strcmp(splitstr[0],"piece") == 0) {
			printf("%c\n",getPiece(&pos,atoi(splitstr[1])));
		}

		else if (strcmp(splitstr[0],"uci") == 0) {
			printf("id name Raven 0.30\nid author Steve Griffin\n");
			printf("option name Hash type spin default 32 min 32 max 256\n");
			printf("uciok\n");
		}
		else if (strcmp(splitstr[0],"quit") == 0) {
			keeprunning = 0;
		}
		else if ( (strcmp(splitstr[0],"position") == 0) && (strcmp(splitstr[1],"startpos") == 0) ) {
			parsefen(&pos, "startpos"); // set start position
			//posstack[0] = pos;
			//posstackend = 1;
			if (strcmp(splitstr[2],"moves") == 0) {
				// make all moves given by position command
				for (int i = 3;i < splitstrend;i++) {
					// make move
					makeMovestr(splitstr[i],&pos);
				}
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
	}
	
	return 0;
}
