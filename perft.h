#ifndef PERFT_H
#define PERFT_H

#include "TT.h"
#include "movegen.h"
#include "makemove.h"

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
	int n_moves = genLegalMoves(pos,moves);
	int kingpos;
	
	for (int i = 0; i < n_moves;i++) {
		makeMove(&moves[i], pos);
		pos->tomove = !pos->tomove;
		int incheck = isCheck(pos);
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
int splitperft(struct position *pos, int depth) {
	struct move moves[MAX_MOVES];
	int nodes = 0;
	int kingpos;
	
	if (depth == 0) return 1;
	
	int n_moves = genLegalMoves(pos,moves);
	
	for (int i = 0; i < n_moves;i++) {
		makeMove(&moves[i], pos);
		pos->tomove = !pos->tomove;
		int incheck = isCheck(pos);
		if (incheck) {
			unmakeMove(pos);
			continue;
		}
		pos->tomove = !pos->tomove;
		
		printf("%s - %" PRIu64 "\n",movetostr(moves[i]), perft(pos,depth - 1));
		nodes += perft(pos,depth - 1);
		
		unmakeMove(pos);
	}
	return nodes;
}
#endif