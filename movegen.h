#ifndef MOVEGEN_H
#define MOVEGEN_H

#include <assert.h>
#include "position.h"
#include "move.h"

#define MAX_MOVES 2048

int genMoves_W(const struct position *pos, struct move *moves);
int genMoves_B(const struct position *pos, struct move *moves);

static inline int genLegalMoves(const struct position *pos, struct move *moves) {
	assert(pos);
	assert(moves);
    assert(legalPos(pos));
	int num_moves = 0;
	if (pos->tomove == WHITE) num_moves += genMoves_W(pos,&moves[num_moves]);
	else num_moves += genMoves_B(pos,&moves[num_moves]);
	return num_moves;
}

#endif
