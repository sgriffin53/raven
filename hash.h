#ifndef HASH_H
#define HASH_H

#include <stdio.h>
#include <stdlib.h>
#include "Trappist\definitions.h"
#include "Trappist\functions.h"

U64 pieceHash[13][64];
U64 turnHash;
U64 castleHash[4];

U64 rand64() {
	U64 r = 0;
	for (int i=0; i<64; i++) {
		r = r*2 + rand()%2;
	}
	return r;
}
void initZobrist() {

	for(int i = 0; i < 13; i++) {
		for(int j = 0; j < 64; j++) {
			pieceHash[i][j] = rand64();
		}
	}
	
	turnHash = rand64();

	for(int i = 0; i < 4; i++) {
		castleHash[i] = rand64();
	}
}
/*
int pieceintval(char inpiece) {
	if (inpiece == 'P') return wP;
	if (inpiece == 'N') return wN;
	if (inpiece == 'B') return wB;
	if (inpiece == 'R') return wR;
	if (inpiece == 'Q') return wQ;
	if (inpiece == 'K') return wK;
	if (inpiece == 'p') return bP;
	if (inpiece == 'n') return bN;
	if (inpiece == 'b') return bB;
	if (inpiece == 'r') return bR;
	if (inpiece == 'q') return bQ;
	if (inpiece == 'k') return bK;
	return 0;
}
*/
int pieceintval(char inpiece) {
	if (inpiece == 'p') return 12;
	if (inpiece == 'n') return 1;
	if (inpiece == 'b') return 2;
	if (inpiece == 'r') return 3;
	if (inpiece == 'q') return 4;
	if (inpiece == 'k') return 5;
	if (inpiece == 'P') return 6;
	if (inpiece == 'N') return 7;
	if (inpiece == 'B') return 8;
	if (inpiece == 'R') return 9;
	if (inpiece == 'Q') return 10;
	if (inpiece == 'K') return 11;
	//printf("%c\n",inpiece);
	assert(0);
	return 0;
}

U64 generateHash(struct position *pos) {
	assert(pos);
	U64 zobrist = 0;
	
	for(int square = 0; square < 64; square++) {
		if(pos->board[square] != '0') {
			int piece = pieceintval(pos->board[square]);
			zobrist ^= pieceHash[piece][square];
			//printf("piece hash: %s %c %" PRIu64 "\n",squareidxtostr(square),pos->board[square],pieceHash[piece][square]);
		}
	}

	if(pos->tomove == WHITE) {
		zobrist ^= turnHash;
	}

	if(pos->epsquare != -1) {
		zobrist ^= pieceHash[EMPTY][pos->epsquare];
	}

	if (pos->WcastleKS) zobrist ^= castleHash[0];
	if (pos->WcastleQS) zobrist ^= castleHash[1];
	if (pos->BcastleKS) zobrist ^= castleHash[2];
	if (pos->BcastleQS) zobrist ^= castleHash[3];
	

	return zobrist;
}
#endif