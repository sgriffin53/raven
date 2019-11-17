
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "position.h"
#include "globals.h"
#include "move.h"
#include "sort.h"
#include "eval.h"
#include "attacks.h"
#include "makemove.h"
#include "PST.h"

struct movescore {
	struct move move;
	int score;
};
int compar(const void *aa, const void *bb) { const struct movescore *a = aa, *b = bb; return b->score - a->score; }

const int arrCenterManhattanDistancesort[64] = { // char is sufficient as well, also unsigned
  3, 3, 3, 3, 3, 3, 3, 3,
  3, 2, 2, 2, 2, 2, 2, 3,
  3, 2, 1, 1, 1, 1, 2, 3,
  3, 2, 1, 0, 0, 1, 2, 3,
  3, 2, 1, 0, 0, 1, 2, 3,
  3, 2, 1, 1, 1, 1, 2, 3,
  3, 2, 2, 2, 2, 2, 2, 3,
  3, 3, 3, 3, 3, 3, 3, 3
};

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
	int scores[num_moves];
	// Score
	for (int i = 0; i < num_moves; i++) {

		scores[i] = 0;
		char cappiece = moves[i].cappiece;
		char piece = moves[i].piece;
		int histval = history[pos->tomove][moves[i].from][moves[i].to];
		int butterflyval = butterfly[pos->tomove][moves[i].from][moves[i].to];
		double histscore = (double)histval;
		if (butterflyval != 0) histscore = (double)histval / (double)butterflyval;
		
		struct move prevmove = movestack[movestackend - 1];
		struct move countermove = countermoves[prevmove.from][prevmove.to];
		if (TTmove.from != -1
			&& (moves[i].from == TTmove.from) && (moves[i].to == TTmove.to) && (moves[i].prom == TTmove.prom)) {
				scores[i] = 5000000;
		}
		else if (cappiece != '0'
			&& (capval(cappiece) >= capval(piece))) {
				scores[i] = 1000000 + mvvlva(piece, cappiece);
		}
		else if (cappiece != '0'
			&& capval(cappiece) < capval(piece)) {
				scores[i] = 700000 + mvvlva(piece,cappiece);
		}
		else if ((killers[ply][0].to == moves[i].to) && (killers[ply][0].from == moves[i].from) && (killers[ply][0].prom == moves[i].prom)) {
			scores[i] = 900000;
		}
		
		else if ((killers[ply][1].to == moves[i].to) && (killers[ply][1].from == moves[i].from) && (killers[ply][1].prom == moves[i].prom)) {
			scores[i] = 850000;
		}
		else if (histscore > 0.0) {
			histscore = 1000.0 + histscore * 100.0;
			if (histscore > 700000.0) {
				histscore = 700000.0;
			}
			scores[i] = (int)histscore;
		}
		if (moves[i].from == countermove.from && moves[i].to == countermove.to) {
			scores[i] += 350000;
		}
	}
	
	// selection sort
	
	int i, j;
	for (i = 0;i < num_moves - 1;i++) {
		int min_idx = i;
		for (j = i+1; j < num_moves;j++) {
			if (scores[j] > scores[min_idx]) min_idx = j;
		}
		int scorecopy = scores[i];
		scores[i] = scores[min_idx];
		scores[min_idx] = scorecopy;
		
		struct move movecopy = moves[i];
		moves[i] = moves[min_idx];
		moves[min_idx] = movecopy;
	}
	
}
