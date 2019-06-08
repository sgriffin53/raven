#ifndef SORT_H
#define SORT_H

#include <ctype.h>
#include <stdio.h>

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

void sortMoves(struct position *pos, struct move *moves, const int num_moves, struct move TTmove, int ply) {
	assert(moves);
	assert(pos);
	assert(num_moves < MAX_MOVES);
	int scores[MAX_MOVES] = {0};

	// Score
	for (int i = 0; i < num_moves; ++i) {
		char cappiece = getPiece(pos,moves[i].to);
		char piece = getPiece(pos,moves[i].from);
		scores[i] = 0;
		if (TTmove.from != -1) {
			if ((moves[i].from == TTmove.from) && (moves[i].to == TTmove.to) && (moves[i].prom == TTmove.prom)) {
				scores[i] = 500000;
			}
		}
		if ((killers[ply][0].to == moves[i].to) && (killers[ply][0].from == moves[i].from) && (killers[ply][0].prom == moves[i].prom)) {
			scores[i] = 900;
		}
		
		else if ((killers[ply][1].to == moves[i].to) && (killers[ply][1].from == moves[i].from) && (killers[ply][1].prom == moves[i].prom)) {
			scores[i] = 800;
		}
		if (cappiece != '0') {
			//if (piece == '0') dspboard(*pos);
			//printf("%s %c %c\n",movetostr(moves[i]),piece,cappiece);
			scores[i] = 1000 + mvvlva(piece, cappiece);
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

#endif
