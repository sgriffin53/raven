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

typedef unsigned long long U64;
struct position {
	int epsquare;
	char board[65];
	int WcastleQS;
	int WcastleKS; 
	int BcastleKS;
	int BcastleQS;
	int tomove;
	int Wkingpos;
	int Bkingpos;
};
struct move {
	int from;
	int to;
	char prom;
};

struct position posstack[1024];
int posstackend = 0;
int nodesSearched = 0;

# include "functions.h"
# include "board.h"
# include "makemove.h"
# include "movegen.h"
# include "PST.h"
# include "engine.h"

struct position setstartpos() {
	struct position pos = {.epsquare=-1,.board={'r','n','b','q','k','b','n','r',
				'p','p','p','p','p','p','p','p',
				'0','0','0','0','0','0','0','0',
				'0','0','0','0','0','0','0','0',
				'0','0','0','0','0','0','0','0',
				'0','0','0','0','0','0','0','0',
				'P','P','P','P','P','P','P','P',
				'R','N','B','Q','K','B','N','R'},.WcastleQS=1,.WcastleKS=1,.BcastleQS=1,.BcastleKS=1,
				.tomove=WHITE,.Wkingpos=60,.Bkingpos=4};
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
	int i;
	int splitstrend;
	struct position pos = setstartpos(); // set start position
	posstack[0] = pos;
	posstackend = 1;
	int keeprunning = 1;
	while (keeprunning) {
		// read input from stdin
		fgets(instr, 8192, stdin);
		// removes newline character from input
		instr[strcspn(instr, "\n")] = 0;
		//split instr into tokens into splitstr by space
		token = strtok(instr," ");
		i = 0;
		while (token != NULL) {
			strcpy(splitstr[i],token);
			i++;
			token = strtok(NULL, " ");
		}
		splitstrend = i; // position of end of splitstr array
		if (strcmp(splitstr[0],"legalmoves") == 0) {
			struct move moves[MAX_MOVES];
			int num_moves = genLegalMoves(&pos,moves);
			i = 0;
			int kingpos;
			for (i = 0;i < num_moves;i++) {
				makeMove(&moves[i], &pos);
				pos.tomove = !pos.tomove;
				if (pos.tomove == WHITE) kingpos = pos.Wkingpos;
				if (pos.tomove == BLACK) kingpos = pos.Bkingpos;
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
		if (strcmp(splitstr[0],"quit") == 0) keeprunning = 0;
		if (strcmp(splitstr[0],"go") == 0) {
			int searchdepth = 25;
			if (strcmp(splitstr[1],"depth") == 0) {
				searchdepth = atoi(splitstr[2]);
			}
			assert(searchdepth >= 1);
			nodesSearched = 0;
			struct move bestmove = search(pos,searchdepth,3000);
			fflush(stdout);
			printf("bestmove %s\n",movetostr(bestmove));
			fflush(stdout);
		}
		if (strcmp(splitstr[0],"isready") == 0) {
			printf("readyok\n");
			fflush(stdout);
		}
		if (strcmp(splitstr[0],"uci") == 0) {
			printf("id name Raven 0.30\nid author JimmyRustles\nuciok\n");
			fflush(stdout);
		}
		if (strcmp(splitstr[0],"board") == 0) dspboard(pos);
		if (strcmp(splitstr[0],"move") == 0) makeMovestr(splitstr[1], &pos);
		if (strcmp(splitstr[0],"unmove") == 0) unmakeMove(&pos);
		if (strcmp(splitstr[0],"eval") == 0) {
			printf("score: %d",evalBoard(&pos));
			fflush(stdout);
		}
		if (strcmp(splitstr[0],"perft") == 0) {
			int depth;
			U64 pnodes;
			U64 nps;
			depth = atoi(splitstr[1]);
			i = 1;
			while (i <= depth) {
				clock_t begin = clock();
				pnodes = perft(&pos,i);
				clock_t end = clock();
				double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
				nps = pnodes / time_spent;
				printf("info depth %d nodes %" PRIu64 " time %f nps %" PRIu64 "\n",i,pnodes,time_spent,nps);
				//printf("info depth %d nodes %llu\n",i,pnodes);
				fflush(stdout);
				i++;
			}
			//printf("nodes %llu\n",pnodes);
			printf("nodes %" PRIu64 "\n", pnodes);
			//printf("nodes %" PRIu64 "\n", pnodes);
			fflush(stdout);
		}
		if (strcmp(splitstr[0],"PST") == 0) {
			char piece = splitstr[1][0];
			printf("%d\n",PSTval(piece, atoi(splitstr[2])));
		}
		if (strcmp(splitstr[0],"rank") == 0) printf("%d\n",getrank(atoi(splitstr[1])));
		if (strcmp(splitstr[0],"file") == 0) printf("%d\n",getfile(atoi(splitstr[1])));
		
		if (strcmp(splitstr[0],"sperft") == 0) {
			int depth = atoi(splitstr[1]);
			splitperft(&pos,depth);
		}
		if ( (strcmp(splitstr[0],"position") == 0) && (strcmp(splitstr[1],"fen") == 0) ) {
			char fen[1024] = "";
			i = 2;
			while (i < splitstrend) {
				strcat(fen,splitstr[i]);
				if (i != (splitstrend - 1)) strcat(fen," ");
				i++;
			}
			pos = parsefen(fen);
			posstack[0] = pos;
		}
		if ( (strcmp(splitstr[0],"position") == 0) && (strcmp(splitstr[1],"startpos") == 0) ) {
			pos = setstartpos(); // set start position
			posstack[0] = pos;
			posstackend = 1;
			if (strcmp(splitstr[2],"moves") == 0) {
				// make all moves given by position command
				i = 3;
				while (i < splitstrend) {
					// make move
					makeMovestr(splitstr[i],&pos);
					i++;
				}
			}
		}
	}		
}