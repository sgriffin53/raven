#ifndef DRAW_H
#define DRAW_H

#include <assert.h>
#include "globals.h"
#include "position.h"
#include "hash.h"

static inline int isThreefold(const struct position *pos) {
	assert(pos);
	assert(pos->halfmoves >= 0);
	if (pos->halfmoves <= 4) return 0;
	int numrepeats = 0;
	U64 curposhash = generateHash(pos);
	currenthash = curposhash;
	//for (int i = (posstackend - 1);(i > (posstackend - 1 - pos->halfmoves - 1)) && i >= 0;i-=2) {
	int start = (posstackend - 1 - pos->halfmoves - 1 - 1);
	if (start < 0) {
		if (pos->tomove == WHITE) start = 0;
		else start = 1;
		
	}
	int end = (posstackend);
	for (int i = start;i < end;i+=2) {
		//printf("%d %d - %d %d %d\n",posstackend,pos->halfmoves,start,i,end);
		struct position checkpos = posstack[i];
		U64 curhash;
		if (hashstack[i] != 0) {
			curhash = hashstack[i];
		}
		else {
			curhash = generateHash(&checkpos);
			hashstack[i] = curhash;
		}
		if (curhash == curposhash) {
			numrepeats++;
			if (numrepeats >= 3) {
				return 1;
			}
		}
	}
	return 0;
}

static inline void clearHashstack(int halfmoves) {
	for (int i = 0;i < halfmoves;i++) {
		hashstack[i] = 0;
	}
}
#endif
