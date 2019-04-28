#ifndef SORT_H
#define SORT_H

#include <ctype.h>
#include "position.h"
#include "move.h"

int capval(char piece) {
	piece = tolower(piece);
	if (piece == 'k') return 5;
	if (piece == 'q') return 4;
	if (piece == 'r') return 3;
	if (piece == 'n') return 2;
	if (piece == 'b') return 2;
	if (piece == 'p') return 1;
	assert(0);
	return 0;
}

int mvvlva(char piece, char cappiece) {
	return 10 * capval(cappiece) - capval(piece);
}

void sortMoves(const struct position *pos, struct move *moves, const int num_moves, struct move TTmove) {
	assert(moves);
	assert(pos);
	assert(num_moves < MAX_MOVES);
	int scores[MAX_MOVES] = {0};

	// Score
	for (int i = 0; i < num_moves; ++i) {
		char cappiece = moves[i].cappiece;
		char piece = pos->board[moves[i].from];
		if (TTmove.from != -1) {
			if ((moves[i].from == TTmove.from) && (moves[i].to == TTmove.to) && (moves[i].prom == TTmove.prom)) {
				scores[i] = 500000;
			}
		}
		if (cappiece != '0') {
			//if (piece == '0') dspboard(*pos);
			//printf("%s %c %c\n",movetostr(moves[i]),piece,cappiece);
			scores[i] = mvvlva(piece, cappiece);
		} else {
			scores[i] = 0;
		}
	}

	// Sort
	for (int a = 0; a < num_moves-1; ++a) {
		// Find best move
		int index = a;
		for (int b = a+1; b < num_moves; ++b) {
			if (scores[b] > scores[index]) {
				index = b;
			}
		}

		// Put best move at the front
		struct move copy = moves[index];
		moves[index] = moves[a];
		moves[a] = copy;

		// Put best score at the front
		int copy2 = scores[index];
		scores[index] = scores[a];
		scores[a] = copy2;
	}
}

int sortMovesOld(const struct position *pos,struct move *moves, const int num_moves) {
	assert(pos);
	assert(moves);
	assert(num_moves >= 0);
	assert(num_moves <= MAX_MOVES);
	struct move capmoves[num_moves];
	struct move noncapmoves[num_moves];
	int capmovesctr = 0;
	int noncapmovesctr = 0;
	for (int i=0;i<num_moves;i++) {
		if (pos->board[moves[i].to] != '0') {
			// capture
			capmoves[capmovesctr] = moves[i];
			capmovesctr++;
		}
		else {
			// not capture
			noncapmoves[noncapmovesctr] = moves[i];
			noncapmovesctr++;
		}
	}
	// rebuild moves array in order of captures first
	for (int i = 0;i < capmovesctr;i++) {
		moves[i] = capmoves[i];
	}
	for (int i=0;i < noncapmovesctr;i++) {
		moves[i+capmovesctr] = noncapmoves[i];
	}
	return num_moves;
}

#endif
