#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "position.h"

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
	pos->Wcastled = 0;
	pos->Bcastled = 0;
	
	
	int n = 0;
	token = strtok(fen," ");
	while (token != NULL) {
		strcpy(splitstr[n],token);
		n++;
		token = strtok(NULL, " ");
	}
	
	pos->pieces[PAWN] = 0ULL;
	pos->pieces[KNIGHT] = 0ULL;
	pos->pieces[BISHOP] = 0ULL;
	pos->pieces[ROOK] = 0ULL;
	pos->pieces[QUEEN] = 0ULL;
	pos->pieces[KING] = 0ULL;
	pos->colours[WHITE] = 0ULL;
	pos->colours[BLACK] = 0ULL;
	
	
	int j = 0;
	for (int i = 0;i < (int)strlen(splitstr[0]);i++) {
		char letter = splitstr[0][i];
		
		//get rank and file of A1 = 0 board from A8 = 0 board
		int realrank = 7 - (j/8);
		int realfile = j % 8;
		int a = fileranktosquareidx(realfile,realrank);
		switch (letter) {
			case 'p': {
				pos->pieces[PAWN] |= (1ULL << a);
				pos->colours[BLACK] |= (1ULL << a);
				break;
			}
			case 'n': {
				pos->pieces[KNIGHT] |= (1ULL << a);
				pos->colours[BLACK] |= (1ULL << a);
				break;
			}
			case 'b': {
				pos->pieces[BISHOP] |= (1ULL << a);
				pos->colours[BLACK] |= (1ULL << a);
				break;
			}
			case 'r': {
				pos->pieces[ROOK] |= (1ULL << a);
				pos->colours[BLACK] |= (1ULL << a);
				break;
			}
			case 'q': {
				pos->pieces[QUEEN] |= (1ULL << a);
				pos->colours[BLACK] |= (1ULL << a);
				break;
			}
			case 'k': {
				pos->pieces[KING] |= (1ULL << a);
				pos->colours[BLACK] |= (1ULL << a);
				pos->Bkingpos = a;
				break;
			}
			case 'P': {
				pos->pieces[PAWN] |= (1ULL << a);
				pos->colours[WHITE] |= (1ULL << a);
				break;
			}
			case 'N': {
				pos->pieces[KNIGHT] |= (1ULL << a);
				pos->colours[WHITE] |= (1ULL << a);
				break;
			}
			case 'B': {
				pos->pieces[BISHOP] |= (1ULL << a);
				pos->colours[WHITE] |= (1ULL << a);
				break;
			}
			case 'R': {
				pos->pieces[ROOK] |= (1ULL << a);
				pos->colours[WHITE] |= (1ULL << a);
				break;
			}
			case 'Q': {
				pos->pieces[QUEEN] |= (1ULL << a);
				pos->colours[WHITE] |= (1ULL << a);
				break;
			}
			case 'K': {
				pos->pieces[KING] |= (1ULL << a);
				pos->colours[WHITE] |= (1ULL << a);
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
	pos->irrevidx = 0;
	pos->irrev[pos->irrevidx].epsquare = pos->epsquare;
	pos->irrev[pos->irrevidx].WcastleQS = pos->WcastleQS;
	pos->irrev[pos->irrevidx].WcastleKS = pos->WcastleKS;
	pos->irrev[pos->irrevidx].BcastleQS = pos->BcastleQS;
	pos->irrev[pos->irrevidx].WcastleKS = pos->BcastleKS;
	pos->irrev[pos->irrevidx].Wcastled = pos->Wcastled;
	pos->irrev[pos->irrevidx].Bcastled = pos->Bcastled;
	pos->irrev[pos->irrevidx].halfmoves = pos->halfmoves;
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
	for (int i = PAWN;i <= KING;i++) {
		if (pos->pieces[i] & BBsquare) return i;
	}
	return NONE;
}
int getColour(struct position *pos, int sq) {
	U64 BBsquare = (1ULL << sq);
	for (int i = 0;i < 2;i++) {
		if (pos->colours[i] & BBsquare) return i;
	}
	return NONE;
}
void setPiece(struct position *pos, int sq, int colour, char piece) {
	assert(pos);
	assert(sq >= 0 && sq <= 63);
	U64 BBsquare = (1ULL << sq);
	//clear bitboard of old square
	char oldpiece = getPiece(pos,sq);
	pos->pieces[oldpiece] &= ~BBsquare;
	//clear white and black piece bitboards of that square
	pos->colours[WHITE] &= ~BBsquare;
	pos->colours[BLACK] &= ~BBsquare;
	if (piece != NONE) pos->pieces[piece] |= BBsquare;
	if (piece == KING) {
		if (colour == WHITE) pos->Wkingpos = sq;
		else if (colour == BLACK) pos->Bkingpos = sq;
	}
	if (colour == WHITE) {
		pos->colours[WHITE] |= BBsquare;
	}
	else if (colour == BLACK) pos->colours[BLACK] |= BBsquare;
	
}
void dspBBstr(char* BBstr,struct position pos) {
	if (strcmp(BBstr,"pawns") == 0) {
		dspBB(pos.pieces[PAWN]);
	}
	if (strcmp(BBstr,"knights") == 0) {
		dspBB(pos.pieces[KNIGHT]);
	}
	if (strcmp(BBstr,"bishops") == 0) {
		dspBB(pos.pieces[BISHOP]);
	}
	if (strcmp(BBstr,"rooks") == 0) {
		dspBB(pos.pieces[ROOK]);
	}
	if (strcmp(BBstr,"queens") == 0) {
		dspBB(pos.pieces[QUEEN]);
	}
	if (strcmp(BBstr,"kings") == 0) {
		dspBB(pos.pieces[KING]);
	}
	if (strcmp(BBstr,"Wpawns") == 0) {
		dspBB((pos.pieces[PAWN] & pos.colours[WHITE]));
	}
	if (strcmp(BBstr,"Wknights") == 0) {
		dspBB((pos.pieces[KNIGHT] & pos.colours[WHITE]));
	}
	if (strcmp(BBstr,"Wbishops") == 0) {
		dspBB((pos.pieces[BISHOP] & pos.colours[WHITE]));
	}
	if (strcmp(BBstr,"Wrooks") == 0) {
		dspBB((pos.pieces[ROOK] & pos.colours[WHITE]));
	}
	if (strcmp(BBstr,"Wqueens") == 0) {
		dspBB((pos.pieces[QUEEN] & pos.colours[WHITE]));
	}
	if (strcmp(BBstr,"Wkings") == 0) {
		dspBB((pos.pieces[KING] & pos.colours[WHITE]));
	}
	if (strcmp(BBstr,"Bpawns") == 0) {
		dspBB((pos.pieces[PAWN] & pos.colours[BLACK]));
	}
	if (strcmp(BBstr,"Bknights") == 0) {
		dspBB((pos.pieces[KNIGHT] & pos.colours[BLACK]));
	}
	if (strcmp(BBstr,"Bbishops") == 0) {
		dspBB((pos.pieces[BISHOP] & pos.colours[BLACK]));
	}
	if (strcmp(BBstr,"Brooks") == 0) {
		dspBB((pos.pieces[ROOK] & pos.colours[BLACK]));
	}
	if (strcmp(BBstr,"Bqueens") == 0) {
		dspBB((pos.pieces[QUEEN] & pos.colours[BLACK]));
	}
	if (strcmp(BBstr,"Bkings") == 0) {
		dspBB((pos.pieces[KING] & pos.colours[BLACK]));
	}
	if (strcmp(BBstr,"blackpieces") == 0) {
		dspBB(pos.colours[BLACK]);
	}
	if (strcmp(BBstr,"whitepieces") == 0) {
		dspBB(pos.colours[WHITE]);
	}
	if (strcmp(BBstr,"pieces") == 0) {
		dspBB((pos.colours[WHITE] | pos.colours[BLACK]));
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
			if (piece == NONE) piece = ' ';
			else if (piece == PAWN) piece = 'P';
			else if (piece == KNIGHT) piece = 'N';
			else if (piece == BISHOP) piece = 'B';
			else if (piece == ROOK) piece = 'R';
			else if (piece == QUEEN) piece = 'Q';
			else if (piece == KING) piece = 'K';
			if (getColour(pos, sq) == BLACK) piece = tolower(piece);
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
	//printf("In check: %d",isCheck(pos));
	printf("\n");
}