#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "position.h"
#include "hash.h"
#include "globals.h"
#include "attacks.h"

int getrank(int square) {
	assert(square >= 0 && square <= 63);
	return square / 8;
}

int getfile(int square) {
	assert(square >= 0 && square <= 63);
	return square % 8;
}

void parsefen(struct position *pos, const char *ofen) {
	if (strncmp(ofen, "startpos", 8) == 0) {
		parsefen(pos, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
		return;
	}
	char fen[strlen(ofen)];
	strcpy(fen, ofen);

	char splitstr[100][8192];
	char * token;

	// set blank position
	memset(pos, 0, sizeof(struct position));
	
	pos->epsquare = -1;
	pos->WcastleKS = 0;
	pos->WcastleQS = 0;
	pos->BcastleKS = 0;
	pos->BcastleQS = 0;
	
	
	int n = 0;
	token = strtok(fen," ");
	while (token != NULL) {
		strcpy(splitstr[n],token);
		n++;
		token = strtok(NULL, " ");
	}
	
	pos->BBpawns = 0ULL;
	pos->BBknights = 0ULL;
	pos->BBbishops = 0ULL;
	pos->BBrooks = 0ULL;
	pos->BBqueens = 0ULL;
	pos->BBkings = 0ULL;
	pos->BBwhitepieces = 0ULL;
	pos->BBblackpieces = 0ULL;
	
	
	int j = 0;
	for (int i = 0;i < (int)strlen(splitstr[0]);i++) {
		char letter = splitstr[0][i];
		
		//get rank and file of A1 = 0 board from A8 = 0 board
		int realrank = 7 - (j/8);
		int realfile = j % 8;
		//printf("%c %d %d\n",letter,realfile,realrank);
		int a = fileranktosquareidx(realfile,realrank);
		switch (letter) {
			case 'p': {
				pos->BBpawns |= (1ULL << a);
				pos->BBblackpieces |= (1ULL << a);
				break;
			}
			case 'n': {
				pos->BBknights |= (1ULL << a);
				pos->BBblackpieces |= (1ULL << a);
				break;
			}
			case 'b': {
				pos->BBbishops |= (1ULL << a);
				pos->BBblackpieces |= (1ULL << a);
				break;
			}
			case 'r': {
				pos->BBrooks |= (1ULL << a);
				pos->BBblackpieces |= (1ULL << a);
				break;
			}
			case 'q': {
				pos->BBqueens |= (1ULL << a);
				pos->BBblackpieces |= (1ULL << a);
				break;
			}
			case 'k': {
				pos->BBkings |= (1ULL << a);
				pos->BBblackpieces |= (1ULL << a);
				pos->Bkingpos = a;
				break;
			}
			case 'P': {
				pos->BBpawns |= (1ULL << a);
				pos->BBwhitepieces |= (1ULL << a);
				break;
			}
			case 'N': {
				pos->BBknights |= (1ULL << a);
				pos->BBwhitepieces |= (1ULL << a);
				break;
			}
			case 'B': {
				pos->BBbishops |= (1ULL << a);
				pos->BBwhitepieces |= (1ULL << a);
				break;
			}
			case 'R': {
				pos->BBrooks |= (1ULL << a);
				pos->BBwhitepieces |= (1ULL << a);
				break;
			}
			case 'Q': {
				pos->BBqueens |= (1ULL << a);
				pos->BBwhitepieces |= (1ULL << a);
				break;
			}
			case 'K': {
				pos->BBkings |= (1ULL << a);
				pos->BBwhitepieces |= (1ULL << a);
				pos->Wkingpos = a;
				break;
			}
			case '/': j--; break;
			case '1' : break;
			case '2' : j++; break;
			case '3' : j+=2; break;
			case '4' : j+=3; break;
			case '5' : j+=4; break;
			case '6' : j+=5; break;
			case '7' : j+=6; break;
			case '8' : j+=7; break;
		}
		j++;
	}
	if (strcmp(splitstr[1],"w") == 0) pos->tomove = WHITE;
	if (strcmp(splitstr[1],"b") == 0) pos->tomove = BLACK;

	for (size_t i = 0;i < strlen(splitstr[2]);i++) {
		if (splitstr[2][i] == 'K') pos->WcastleKS = 1;
		else if (splitstr[2][i] == 'Q') pos->WcastleQS = 1;
		else if (splitstr[2][i] == 'k') pos->BcastleKS = 1;
		else if (splitstr[2][i] == 'q') pos->BcastleQS = 1;
	}

	if (strcmp(splitstr[3],"-") != 0) {
		//en passant square given
		pos->epsquare = strsquaretoidx(splitstr[3]);
	}

	pos->halfmoves = atoi(splitstr[4]);
	if (splitstr[4][0] == '-') pos->halfmoves = 0;
	posstack[0] = *pos;
	posstackend = 1;
}
int fileranktosquareidx(int file,int rank) {
	return (rank) * 8 + file;
}
int strsquaretoidx(char square[]) {
	int file = (int)square[0] - 97;
	int rank = (int)square[1] - 49;
	return fileranktosquareidx(file,rank);
}
char getPiece(struct position *pos, int sq) {
	assert(pos);
	assert(sq >= 0 && sq <= 63);
	U64 BBsquare = (1ULL << sq);
	if (pos->BBpawns & BBsquare) {
		if (pos->BBwhitepieces & BBsquare) return 'P';
		else return 'p';
	}
	if (pos->BBknights & BBsquare) {
		if (pos->BBwhitepieces & BBsquare) return 'N';
		else return 'n';
	}
	if (pos->BBbishops & BBsquare) {
		if (pos->BBwhitepieces & BBsquare) return 'B';
		else return 'b';
	}
	if (pos->BBrooks & BBsquare) {
		if (pos->BBwhitepieces & BBsquare) return 'R';
		else return 'r';
	}
	if (pos->BBqueens & BBsquare) {
		if (pos->BBwhitepieces & BBsquare) return 'Q';
		else return 'q';
	}
	if (pos->BBkings & BBsquare) {
		if (pos->BBwhitepieces & BBsquare) return 'K';
		else return 'k';
	}
	return '0';
}
void setPiece(struct position *pos, int sq, char piece) {
	assert(piece);
	assert(pos);
	assert(sq >= 0 && sq <= 63);
	U64 BBsquare = (1ULL << sq);
	//clear bitboard of old square
	char oldpiece = getPiece(pos,sq);

	if ((oldpiece == 'p') || (oldpiece == 'P')) {
		pos->BBpawns &= ~BBsquare;
	}
	else if ((oldpiece == 'n') || (oldpiece == 'N')) {
		pos->BBknights &= ~BBsquare;
	}
	else if ((oldpiece == 'b') || (oldpiece == 'B')) {
		pos->BBbishops &= ~BBsquare;
	}
	else if ((oldpiece == 'r') || (oldpiece == 'R')) {
		pos->BBrooks &= ~BBsquare;
	}
	else if ((oldpiece == 'q') || (oldpiece == 'Q')) {
		pos->BBqueens &= ~BBsquare;
	}
	else if ((oldpiece == 'k') || (oldpiece == 'K')) {
		pos->BBkings &= ~BBsquare;
	}
	//clear white and black piece bitboards of that square
	pos->BBwhitepieces &= ~BBsquare;
	pos->BBblackpieces &= ~BBsquare;
	//white pieces
	if (piece == 'P') {
		pos->BBpawns |= BBsquare;
		pos->BBwhitepieces |= BBsquare;
	}
	else if (piece == 'N') {
		pos->BBknights |= BBsquare;
		pos->BBwhitepieces |= BBsquare;
	}
	else if (piece == 'B') {
		pos->BBbishops |= BBsquare;
		pos->BBwhitepieces |= BBsquare;
	}
	else if (piece == 'R') {
		pos->BBrooks |= BBsquare;
		pos->BBwhitepieces |= BBsquare;
	}
	else if (piece == 'Q') {
		pos->BBqueens |= BBsquare;
		pos->BBwhitepieces |= BBsquare;
	}
	else if (piece == 'K') {
		pos->BBkings |= BBsquare;
		pos->BBwhitepieces |= BBsquare;
		pos->Wkingpos = sq;
	}
	//black pieces
	else if (piece == 'p') {
		pos->BBpawns |= BBsquare;
		pos->BBblackpieces |= BBsquare;
	}
	else if (piece == 'n') {
		pos->BBknights |= BBsquare;
		pos->BBblackpieces |= BBsquare;
	}
	else if (piece == 'b') {
		pos->BBbishops |= BBsquare;
		pos->BBblackpieces |= BBsquare;
	}
	else if (piece == 'r') {
		pos->BBrooks |= BBsquare;
		pos->BBblackpieces |= BBsquare;
	}
	else if (piece == 'q') {
		pos->BBqueens |= BBsquare;
		pos->BBblackpieces |= BBsquare;
	}
	else if (piece == 'k') {
		pos->BBkings |= BBsquare;
		pos->BBblackpieces |= BBsquare;
		pos->Bkingpos = sq;
	}
	//empty square
	/*
	else if (piece == '0') {
		pos->BBpawns &= ~BBsquare;
		pos->BBknights &= ~BBsquare;
		pos->BBbishops &= ~BBsquare;
		pos->BBrooks &= ~BBsquare;
		pos->BBqueens &= ~BBsquare;
		pos->BBkings &= ~BBsquare;
		pos->BBwhitepieces &= ~BBsquare;
		pos->BBblackpieces &= ~BBsquare;
	}
	 */
	
}
void dspBBstr(char* BBstr,struct position pos) {
	if (strcmp(BBstr,"pawns") == 0) {
		dspBB(pos.BBpawns);
	}
	if (strcmp(BBstr,"knights") == 0) {
		dspBB(pos.BBknights);
	}
	if (strcmp(BBstr,"bishops") == 0) {
		dspBB(pos.BBbishops);
	}
	if (strcmp(BBstr,"rooks") == 0) {
		dspBB(pos.BBrooks);
	}
	if (strcmp(BBstr,"queens") == 0) {
		dspBB(pos.BBqueens);
	}
	if (strcmp(BBstr,"kings") == 0) {
		dspBB(pos.BBkings);
	}
	if (strcmp(BBstr,"Wpawns") == 0) {
		dspBB((pos.BBpawns & pos.BBwhitepieces));
	}
	if (strcmp(BBstr,"Wknights") == 0) {
		dspBB((pos.BBknights & pos.BBwhitepieces));
	}
	if (strcmp(BBstr,"Wbishops") == 0) {
		dspBB((pos.BBbishops & pos.BBwhitepieces));
	}
	if (strcmp(BBstr,"Wrooks") == 0) {
		dspBB((pos.BBrooks & pos.BBwhitepieces));
	}
	if (strcmp(BBstr,"Wqueens") == 0) {
		dspBB((pos.BBqueens & pos.BBwhitepieces));
	}
	if (strcmp(BBstr,"Wkings") == 0) {
		dspBB((pos.BBkings & pos.BBwhitepieces));
	}
	if (strcmp(BBstr,"Bpawns") == 0) {
		dspBB((pos.BBpawns & pos.BBblackpieces));
	}
	if (strcmp(BBstr,"Bknights") == 0) {
		dspBB((pos.BBknights & pos.BBblackpieces));
	}
	if (strcmp(BBstr,"Bbishops") == 0) {
		dspBB((pos.BBbishops & pos.BBblackpieces));
	}
	if (strcmp(BBstr,"Brooks") == 0) {
		dspBB((pos.BBrooks & pos.BBblackpieces));
	}
	if (strcmp(BBstr,"Bqueens") == 0) {
		dspBB((pos.BBqueens & pos.BBblackpieces));
	}
	if (strcmp(BBstr,"Bkings") == 0) {
		dspBB((pos.BBkings & pos.BBblackpieces));
	}
	if (strcmp(BBstr,"blackpieces") == 0) {
		dspBB(pos.BBblackpieces);
	}
	if (strcmp(BBstr,"whitepieces") == 0) {
		dspBB(pos.BBwhitepieces);
	}
	if (strcmp(BBstr,"pieces") == 0) {
		dspBB((pos.BBwhitepieces | pos.BBblackpieces));
	}
}
void dspBB(U64 BB) {
	
	printf("\n");
	printf("  +---+---+---+---+---+---+---+---+\n");
	printf("8 |");
	
	int rank = 0;
	int file = 0;
	int sq = 0;
	for (rank = 7;rank >= 0;rank--) {
		for (file = 0;file <= 7;file++) {
			sq = fileranktosquareidx(file,rank);
			if ((1ULL << sq) & BB) {
				printf(" 1 |");
			}
			else {
				printf(" 0 |");
			}
		}
		printf("\n");
		printf("  +---+---+---+---+---+---+---+---+\n");
		if ((sq/8) != 0) {
			printf("%d",(sq/8));
			printf(" |");
		}
	}
	printf("    A   B   C   D   E   F   G   H  \n");
}

void dspBoard(struct position *pos) {
	
	printf("\n");
	printf("  +---+---+---+---+---+---+---+---+\n");
	printf("8 |");
	
	int rank = 0;
	int file = 0;
	int sq = 0;
	for (rank = 7;rank >= 0;rank--) {
		for (file = 0;file <= 7;file++) {
			sq = fileranktosquareidx(file,rank);
			char piece = getPiece(pos,sq);
			if (piece == '0') piece = ' ';
			printf(" %c |",piece);
		}
		printf("\n");
		printf("  +---+---+---+---+---+---+---+---+\n");
		if ((sq/8) != 0) {
			printf("%d",(sq/8));
			printf(" |");
		}
	}
	printf("    A   B   C   D   E   F   G   H  \n");
	printf("Side to move: ");
	if (pos->tomove == 0) printf("Black");
	else printf("White");
	printf("\n");
	printf("EP Square: %d",pos->epsquare);
	printf("\n");
	printf("White King pos: %d",pos->Wkingpos);
	printf("\n");
	printf("Black King pos: %d",pos->Bkingpos);
	printf("\n");
	printf("White castling QS: %d",pos->WcastleQS);
	printf("\n");
	printf("White castling KS: %d",pos->WcastleKS);
	printf("\n");
	printf("Black castling QS: %d",pos->BcastleQS);
	printf("\n");
	printf("Black castling KS: %d",pos->BcastleKS);
	printf("\n");
	printf("Half moves: %d",pos->halfmoves);
	printf("\n");
	printf("Hash: %"PRIu64,generateHash(pos));
	printf("\n");
	int kingpos;
	if (pos->tomove == WHITE) kingpos = pos->Wkingpos;
	else kingpos = pos->Bkingpos;
	printf("In check: %d",isCheck(pos));
	printf("\n");
}
