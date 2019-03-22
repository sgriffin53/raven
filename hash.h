
#include <stdio.h>
#include <stdlib.h>
#include "Trappist\definitions.h"
#include "Trappist\functions.h"

U64 pieceHash[13][120];
U64 turnHash;
U64 castleHash[16];

static U64 rand64() {
	U64 r = 0;
	int i;
	for (i=0; i<64; i++) {
		r = r*2 + rand()%2;
	}
	return r;
}

void initZobrist() {
	int i, j;
	
	for(i = 0; i < 13; i++) {
		for(j = 0; j < 120; j++) {
			pieceHash[i][j] = rand64();
		}
	}
	
	turnHash = rand64();

	for(i = 0; i < 16; i++) {
		castleHash[i] = rand64();
	}
}
int pieceintval(char inpiece) {
	if (inpiece == 'p') return 0;
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
	return 12;
}
U64 generateHash(struct position *pos) {
	int square;
	int piece;
	U64 zobrist = 0;
	
	for(square = 0; square < 64; square++) {
		piece = pieceintval(pos->board[square]);
		if(pos->board[square] != '0') {
			zobrist ^= pieceHash[piece][square];
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