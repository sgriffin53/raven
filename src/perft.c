#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include "position.h"
#include "move.h"
#include "movegen.h"
#include "makemove.h"
#include "attacks.h"
#include "tests.h"

U64 perft(struct position *pos, int depth) {
	assert(pos);
	assert(depth>=0);

	if (depth == 0) return 1;

	U64 nodes = 0;
	struct move moves[MAX_MOVES];
	const int n_moves = genMoves(pos,moves, 0);

	for (int i = 0; i < n_moves;i++) {
		struct position origpos = *pos;
		makeMove(&moves[i], pos);
		pos->tomove = !pos->tomove;
		const int incheck = isCheck(pos);
		if (incheck) {
			unmakeMove(&moves[i], pos);
			continue;
		}
		pos->tomove = !pos->tomove;

		nodes += perft(pos, depth - 1);

		unmakeMove(&moves[i], pos);
		if (!posMatch(pos, &origpos)) {
			printf("move: %s\n", movetostr(moves[i]));
			printf("orig pos:\n");
			dspBoard(&origpos);
			printf("after unmake:\n");
			dspBoard(pos);
		}
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
			unmakeMove(&moves[i], pos);
			continue;
		}
		pos->tomove = !pos->tomove;

		const U64 nodes = perft(pos,depth - 1);

		printf("%s - %" PRIu64 "\n",movetostr(moves[i]), nodes);
		total_nodes += nodes;

		unmakeMove(&moves[i], pos);
	}
	return total_nodes;
}