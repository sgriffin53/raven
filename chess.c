# include <stdio.h>
# include <string.h>

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

void dspboard(struct position pos) {
	char piece;
	char board[65];
	strcpy(board,pos.board);
	printf("\n");
	printf("  +---+---+---+---+---+---+---+---+\n");
	printf("8 |");
	for (int i = 0;i<64;i++) {
		if ( ((i%8) == 0) && (i != 0) ) {
			printf("\n");
			printf("  +---+---+---+---+---+---+---+---+\n");
			printf("%d",(8 - i/8));
			printf(" |");
		}
		piece = board[i];
		if (piece == '0') piece = ' ';
		printf(" %c |", piece);
	}
	printf("\n  +---+---+---+---+---+---+---+---+");
	printf("\n    A   B   C   D   E   F   G   H  \n");
	printf("Side to move: ");
	if (pos.tomove == 0) printf("Black");
	else printf("White");
	printf("\n");
}
int strsquaretoidx(char square[]) {
	int file, rank;
	file = (int)square[0] - 97;
	rank = (int)square[1] - 49;
	rank = 7 - rank;
	return (((rank) << 3) | (file));
}

struct position makeMove(struct move move, struct position pos) {
	pos.board[move.to] = pos.board[move.from];
	pos.board[move.from] = '0';
	pos.tomove = !pos.tomove;
	//strcpy(pos.board[move.to],pos.board[move.from]);
	//strcpy(pos.board[move.from],"0");
	return pos;
}
struct position makeMovestr(char move[], struct position pos) {
	int startsquareidx;
	int endsquareidx;
	char startsquare[3];
	char endsquare[3];
	char prompiece[2];
	startsquare[0] = move[0];
	startsquare[1] = move[1];
	startsquare[2] = 0;
	endsquare[0] = move[2];
	endsquare[1] = move[3];
	endsquare[2] = 0;
	prompiece[0] = move[4];
	prompiece[1] = 0;
	startsquareidx = strsquaretoidx(startsquare);
	endsquareidx = strsquaretoidx(endsquare);
	struct move moveobj = {.from=startsquareidx,.to=endsquareidx,.prom=prompiece[0]};
	return makeMove(moveobj, pos);
}
int main() {
	char instr[100];
	char splitstr[100][100];
	char * token;
	int i;
	struct position pos = {.epsquare=-1,.board={'r','n','b','q','k','b','n','r',
				'p','p','p','p','p','p','p','p',
				'0','0','0','0','0','0','0','0',
				'0','0','0','0','0','0','0','0',
				'0','0','0','0','0','0','0','0',
				'0','0','0','0','0','0','0','0',
				'P','P','P','P','P','P','P','P',
				'R','N','B','Q','K','B','N','R'},.WcastleQS=1,.WcastleKS=1,.BcastleQS=1,.BcastleKS=1,
				.tomove=1,.Wkingpos=60,.Bkingpos=4};
	while (1) {
		// read input from stdin
		fgets(instr, 100, stdin);
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
		if (strcmp(splitstr[0],"uci") == 0) printf("id name Raven 0.30\nid author JimmyRustles\nuciok\n");
		if (strcmp(splitstr[0],"board") == 0) dspboard(pos);
		if (strcmp(splitstr[0],"move") == 0) pos = makeMovestr(splitstr[1], pos);
	}
		
}