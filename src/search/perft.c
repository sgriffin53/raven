#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include "../chess/hash.h"
#include "../chess/position.h"
#include "../chess/move.h"
#include "../chess/movegen.h"
#include "../chess/makemove.h"
#include "../chess/attacks.h"
#include "perft.h"

U64 perft(struct position *pos, int depth) {
	assert(pos);
	assert(depth>=0);

	if (depth == 0) return 1;

	U64 nodes = 0;
	struct move moves[MAX_MOVES];
	const int n_moves = genMoves(pos,moves, 0);

	for (int i = 0; i < n_moves;i++) {
		makeMove(&moves[i], pos);
		pos->tomove = !pos->tomove;
		const int incheck = isCheck(pos);
		if (incheck) {
			unmakeMove(pos);
			continue;
		}
		pos->tomove = !pos->tomove;

		nodes += perft(pos, depth - 1);

		unmakeMove(pos);
	}

	return nodes;
}



U64 splitperft(struct position *pos, int depth) {
	struct move moves[MAX_MOVES];
	U64 total_nodes = 0;

	if (depth == 0) return 1;

	const int n_moves = genMoves(pos,moves, 0);

	for (int i = 0; i < n_moves;i++) {
		makeMove(&moves[i], pos);
		pos->tomove = !pos->tomove;
		const int incheck = isCheck(pos);
		if (incheck) {
			unmakeMove(pos);
			continue;
		}
		pos->tomove = !pos->tomove;

		const U64 nodes = perft(pos,depth - 1);

		printf("%s - %" PRIu64 "\n",movetostr(moves[i]), nodes);
		total_nodes += nodes;

		unmakeMove(pos);
	}
	return total_nodes;
}