#ifndef MOVEGEN_HclTabCtrl
#define MOVEGEN_H

#include "attacks.h"
#include "position.h"
#include "move.h"

int genMoves(struct position *pos, struct move *moves);
int genKingMoves(struct position *pos, int square, struct move *moves);
int genKnightMoves(struct position *pos, int square, struct move *moves);
int genRookMoves(struct position *pos, int square, struct move *moves);
int genQueenMoves(struct position *pos, int square, struct move *moves);
int genBishopMoves(struct position *pos, int square, struct move *moves);
int genPawnMoves(struct position *pos, int square, struct move *moves);
#endif