#ifndef PERFT_H
#define PERFT_H

#include <assert.h>
#include "position.h"
#include "move.h"
#include "movegen.h"
#include "makemove.h"
#include "attacks.h"

U64 perft(struct position *pos, int depth) {
	assert(pos);
	assert(depth>=0);

	if (depth == 0) return 1;

	//U64 hash = generateHash(pos);

//	struct PTTentry PTTdata = getPTTentry(&PTT,hash);

//	if ((PTTdata.hash == hash) && (PTTdata.depth == depth)) {
//		return PTTdata.nodes;
//	}

	U64 nodes = 0;
	struct move moves[MAX_MOVES];
	const int n_moves = genLegalMoves(pos,moves);

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

//	addPTTentry(&PTT,hash,depth,nodes);

	return nodes;
}

U64 splitperft(struct position *pos, int depth) {
	struct move moves[MAX_MOVES];
	U64 total_nodes = 0;

	if (depth == 0) return 1;

	const int n_moves = genLegalMoves(pos,moves);

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

#endif
