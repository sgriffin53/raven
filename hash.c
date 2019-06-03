#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "hash.h"
#include "position.h"

U64 pieceHash[13][64];
U64 turnHash;
U64 castleHash[4];

#define RAND_64 ((uint64_t)(rand() & 0x7FFF) ^ \
 				((uint64_t)(rand() & 0x7FFF) << 15) ^ \
 				((uint64_t)(rand() & 0x7FFF) << 30) ^ \
 				((uint64_t)(rand() & 0x7FFF) << 45) ^ \
 				((uint64_t)(rand() & 0xF) << 60))

void initZobrist() {
	for(int i = 0; i < 13; i++) {
		for(int j = 0; j < 64; j++) {
			pieceHash[i][j] = RAND_64;
		}
	}

	turnHash = RAND_64;

	for(int i = 0; i < 4; i++) {
		castleHash[i] = RAND_64;
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
	//printf("-- %c\n",inpiece);
	assert(0);
	return 0;
}

U64 generateHash(struct position *pos) {
	assert(pos);
	U64 zobrist = 0;

	for(int square = 0; square < 64; square++) {
		char sqpiece = getPiece(pos,square);
		if(sqpiece != '0') {
			//char squarepiece = pos->board[square];
			//printf("piece: %c %d\n",squarepiece,(int)squarepiece);
			int piece = pieceintval(sqpiece);
			zobrist ^= pieceHash[piece][square];
			//printf("piece hash: %s %c %" PRIu64 "\n",squareidxtostr(square),pos->board[square],pieceHash[piece][square]);
		}
	}

	if(pos->tomove == WHITE) {
		zobrist ^= turnHash;
	}

	if(pos->epsquare != -1) {
		zobrist ^= pieceHash[0][pos->epsquare];
	}

	if (pos->WcastleKS) zobrist ^= castleHash[0];
	if (pos->WcastleQS) zobrist ^= castleHash[1];
	if (pos->BcastleKS) zobrist ^= castleHash[2];
	if (pos->BcastleQS) zobrist ^= castleHash[3];


	return zobrist;
}