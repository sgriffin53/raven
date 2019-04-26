#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "functions.h"
#include "movegen_black.h"
#include "movegen_white.h"

int genLegalMoves(const struct position *pos, struct move *moves) {
	assert(pos);
	assert(moves);
	int num_moves = 0;
	if (pos->tomove == WHITE) num_moves += genMoves_W(pos,&moves[num_moves]);
	else num_moves += genMoves_B(pos,&moves[num_moves]);
	return num_moves;
}
#endif
