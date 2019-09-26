#ifndef MOVEGEN_HclTabCtrl
#define MOVEGEN_H

#include "attacks.h"
#include "position.h"
#include "move.h"

int genMoves(struct position *pos, struct move *moves, int forqsearch);
int genKingMoves(struct position *pos, int square, struct move *moves, int forqsearch);
int genKnightMoves(struct position *pos, int square, struct move *moves, int forqsearch);
int genRookMoves(struct position *pos, int square, struct move *moves, int forqsearch);
int genQueenMoves(struct position *pos, int square, struct move *moves, int forqsearch);
int genBishopMoves(struct position *pos, int square, struct move *moves, int forqsearch);
int genPawnMoves(struct position *pos, int square, struct move *moves, int forqsearch);
U64 pinnedPieces(struct position *pos);
U64 xrayBishopAttacks(const int sqr, const U64 occupied, const U64 myPieces);
U64 xrayRookAttacks(const int sqr, const U64 occupied, const U64 myPieces);
U64 line(const int a, const int b);
#endif