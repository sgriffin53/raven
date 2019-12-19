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
int genAllPawnMoves(struct position *pos, int square, struct move *moves, int forqsearch);

#endif