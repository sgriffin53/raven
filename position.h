#ifndef POSITION_H
#define POSITION_H

#include "assert.h"

typedef unsigned long long U64;

#define WHITE 1
#define BLACK 0
#define MATE_SCORE 9999

struct position {
	U64 BBwhitepieces;
	U64 BBblackpieces;
	U64 BBpawns;
	U64 BBknights;
	U64 BBbishops;
	U64 BBrooks;
	U64 BBqueens;
	U64 BBkings;
	int epsquare;
	int WcastleQS;
	int WcastleKS;
	int BcastleKS;
	int BcastleQS;
	int tomove;
	int Wkingpos;
	int Bkingpos;
	int halfmoves;
};

#include "hash.h"

enum SQUARE{A1,B1,C1,D1,E1,F1,G1,H1,
			A2,B2,C2,D2,E2,F2,G2,H2,
			A3,B3,C3,D3,E3,F3,G3,H3,
			A4,B4,C4,D4,E4,F4,G4,H4,
			A5,B5,C5,D5,E5,F5,G5,H5,
			A6,B6,C6,D6,E6,F6,G6,H6,
			A7,B7,C7,D7,E7,F7,G7,H7,
			A8,B8,C8,D8,E8,F8,G8,H8};


int getrank(int square);
int getfile(int square);
void parsefen(struct position *pos, const char *ofen);
int fileranktosquareidx(int file,int rank);
void dspBB(U64 BB);
void dspBBstr(char* BBstr,struct position pos);
int strsquaretoidx(char square[]);
char getPiece(struct position *pos,int sq);
void setPiece(struct position *pos, int sq, char piece);
void dspBoard(struct position *pos);


#endif