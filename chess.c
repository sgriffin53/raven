# include <stdio.h>
# include <string.h>
# include <ctype.h>
# include <stdlib.h>
# include <time.h>
# include <assert.h>
# include <inttypes.h>

# define WHITE 1
# define BLACK 0
# define MAX_MOVES 2048
# define MATE_SCORE 99999

enum SQUARE{A8,B8,C8,D8,E8,F8,G8,H8,
			A7,B7,C7,D7,E7,F7,G7,H7,
			A6,B6,C6,D6,E6,F6,G6,H6,
			A5,B5,C5,D5,E5,F5,G5,H5,
			A4,B4,C4,D4,E4,F4,G4,H4,
			A3,B3,C3,D3,E3,F3,G3,H3,
			A2,B2,C2,D2,E2,F2,G2,H2,
			A1,B1,C1,D1,E1,F1,G1,H1};

typedef unsigned long long U64;

struct position {
	int epsquare;
	char board[64];
	int WcastleQS;
	int WcastleKS; 
	int BcastleKS;
	int BcastleQS;
	int tomove;
	int Wkingpos;
	int Bkingpos;
	int halfmoves;
};
struct move {
	int from;
	int to;
	char prom;
};

struct position posstack[1024];
int posstackend = 0;
int nodesSearched = 0;

int getrank(int square) {
	assert(square >= 0 && square <= 63);
	return (int)(square / 8);
}

int getfile(int square) {
	assert(square >= 0 && square <= 63);
	return square % 8;
}

char* squareidxtostr(int square) {
	assert(square >= 0 && square <= 63);
	char returnstring[3];
	char squarefile = (char)(getfile(square) + 97);
	char squarerank = (char)(7 - getrank(square) + 49);
	returnstring[0] = squarefile;
	returnstring[1] = squarerank;
	returnstring[2] = 0;
	return strdup(returnstring);
}

# include "board.h"
# include "hash.h"
# include "functions.h"
# include "TT.h"
# include "makemove.h"
# include "movegen.h"
# include "PST.h"
# include "eval.h"
# include "search.h"
# include "perft.h"

struct position setstartpos() {
	struct position pos = {.epsquare=-1,.board={'r','n','b','q','k','b','n','r',
				'p','p','p','p','p','p','p','p',
				'0','0','0','0','0','0','0','0',
				'0','0','0','0','0','0','0','0',
				'0','0','0','0','0','0','0','0',
				'0','0','0','0','0','0','0','0',
				'P','P','P','P','P','P','P','P',
				'R','N','B','Q','K','B','N','R'},.WcastleQS=1,.WcastleKS=1,.BcastleQS=1,.BcastleKS=1,
				.tomove=WHITE,.Wkingpos=60,.Bkingpos=4,.halfmoves=0};
	return pos;
}
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
	int splitstrend;
	int keeprunning = 1;
	struct position pos = setstartpos(); // set start position
	posstack[0] = pos;
	posstackend = 1;
	
	initZobrist();
	
	initPTT(&PTT);
	initTT(&TT);
	
	while (keeprunning) {
		// read input from stdin
		fgets(instr, 8192, stdin);
		// removes newline character from input
		instr[strcspn(instr, "\n")] = 0;
		//split instr into tokens into splitstr by space
		token = strtok(instr," ");
		
		int i = 0;
		while (token != NULL) {
			strcpy(splitstr[i],token);
			i++;
			token = strtok(NULL, " ");
		}
		splitstrend = i; // position of end of splitstr array
		
		if (strcmp(splitstr[0],"legalmoves") == 0) {
			
			struct move moves[MAX_MOVES];
			int num_moves = genLegalMoves(&pos,moves);
			int kingpos;
			sortMoves(&pos,moves,num_moves);
			for (int i = 0;i < num_moves;i++) {
				makeMove(&moves[i], &pos);
				pos.tomove = !pos.tomove;
				if (pos.tomove == WHITE) kingpos = pos.Wkingpos;
				else kingpos = pos.Bkingpos;
				int incheck = isCheck(&pos,kingpos);
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
			movetime = 2147483646;
			
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
			
			struct move bestmove = search(pos,searchdepth,movetime);
			
			printf("bestmove %s\n",movetostr(bestmove));
			fflush(stdout);
		}
		
		else if (strcmp(splitstr[0],"isready") == 0) {
			printf("readyok\n");
			fflush(stdout);
		}
		
		else if (strcmp(splitstr[0],"uci") == 0) {
			printf("id name Raven 0.30\nid author JimmyRustles\nuciok\n");
			fflush(stdout);
		}
		
		else if (strcmp(splitstr[0],"board") == 0) dspboard(pos);
		
		else if (strcmp(splitstr[0],"move") == 0) makeMovestr(splitstr[1], &pos);
		
		else if (strcmp(splitstr[0],"unmove") == 0) unmakeMove(&pos);
		
		else if (strcmp(splitstr[0],"eval") == 0) {
			printf("score: %d",taperedEval(&pos));
			fflush(stdout);
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
		
		else if ( (strcmp(splitstr[0],"position") == 0) && (strcmp(splitstr[1],"fen") == 0) ) {
			char fen[1024] = "";
			for (i = 2;i < splitstrend;i++) {
				strcat(fen,splitstr[i]);
				if (i != (splitstrend - 1)) strcat(fen," ");
			}
			pos = parsefen(fen);
			posstack[0] = pos;
		}
		
		else if ( (strcmp(splitstr[0],"position") == 0) && (strcmp(splitstr[1],"startpos") == 0) ) {
			pos = setstartpos(); // set start position
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
	free(TT.entries);
	free(PTT.entries);
	
	return 0;
}