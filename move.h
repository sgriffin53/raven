#ifndef MOVE_H
#define MOVE_H

#include "assert.h"

struct move {
	int from;
	int to;
	char prom;
	char cappiece;
};

static inline char* movetostr(struct move move) {
	assert(move.to >= 0 && move.to <= 63);
	assert(move.from >= 0 && move.from <= 63);

	char returnstring[6];
	char startsquarefile = (char)(getfile(move.from) + 97);
	char startsquarerank = (char)(7 - getrank(move.from) + 49);
	char endsquarefile = (char)(getfile(move.to) + 97);
	char endsquarerank = (char)(7 - getrank(move.to) + 49);
	returnstring[0] = startsquarefile;
	returnstring[1] = startsquarerank;
	returnstring[2] = endsquarefile;
	returnstring[3] = endsquarerank;
	returnstring[4] = move.prom;
	returnstring[5] = 0;
	return strdup(returnstring);
}

#endif
