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
int pieceintvals[2][6] = {
	{ 0, 1, 2, 3, 4, 5, },
	{ 7, 8, 9, 10, 11, 12 }
};
int pieceintval(int inpiece, int piececol) {
	
	
	
	return pieceintvals[piececol][inpiece];
	return 0;
}

U64 generateHash(struct position *pos) {
	assert(pos);
	U64 zobrist = 0;
	U64 BBoccupied = pos->colours[BLACK] | pos->colours[WHITE];
	while (BBoccupied != 0) {
		int square = __builtin_ctzll(BBoccupied);
		BBoccupied &= BBoccupied - 1;
		int sqpiece = getPiece(pos,square);
		int sqpiececol = getPieceCol(pos, square);
		
		int piece = pieceintval(sqpiece, sqpiececol);
		zobrist ^= pieceHash[piece][square];
	}
	/*
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
	*/
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