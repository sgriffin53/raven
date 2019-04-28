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
	for (int i = (posstackend - 1);(i > (posstackend - 1 - pos->halfmoves - 1)) && i >= 0;i-=2) {
		struct position checkpos = posstack[i];
		if (generateHash(&checkpos) == curposhash) {
			numrepeats++;
			if (numrepeats >= 3) {
				return 1;
			}
		}
	}
	return 0;
}

#endif
