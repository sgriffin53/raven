#ifndef MOVE_H
#define MOVE_H

#include <string.h>
#include "position.h"
#include "assert.h"

#define MAX_MOVES 2048

enum MOVETYPE{ NORMAL, DOUBLE, CAPTURE, EN_PASSANT, PROMO, PROMO_CAPTURE , KSC, QSC };

struct move {
	int from;
	int to;
	char prom;
	char piece;
	char cappiece;
	char type;
};

static inline char* movetostr(struct move move) {
	assert(move.to >= 0 && move.to <= 63);
	assert(move.from >= 0 && move.from <= 63);

	char returnstring[6];
	char startsquarefile = (char)(getfile(move.from) + 97);
	char startsquarerank = (char)(getrank(move.from) + 49);
	char endsquarefile = (char)(getfile(move.to) + 97);
	char endsquarerank = (char)(getrank(move.to) + 49);
	int prom = 0;
	if (move.prom == KNIGHT) prom = 'n';
	else if (move.prom == BISHOP) prom = 'b';
	else if (move.prom == ROOK) prom = 'r';
	else if (move.prom == QUEEN) prom = 'q';
	returnstring[0] = startsquarefile;
	returnstring[1] = startsquarerank;
	returnstring[2] = endsquarefile;
	returnstring[3] = endsquarerank;
	returnstring[4] = prom;
	returnstring[5] = 0;
	return strdup(returnstring);
}

#endif