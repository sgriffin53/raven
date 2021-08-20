
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "position.h"
#include "globals.h"
#include "move.h"
#include "sort.h"
#include "eval.h"
#include "attacks.h"
#include "makemove.h"
#include "PST.h"
#include "search.h"

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
	switch (piece) {
		case PAWN: return 1;
		case BISHOP: return 2;
		case KNIGHT: return 2;
		case ROOK: return 3;
		case QUEEN: return 4;
		case KING: return 5;
	}
	/*
	piece = tolower(piece);
	if (piece == 'k') return 5;
	if (piece == 'q') return 4;
	if (piece == 'r') return 3;
	if (piece == 'n') return 2;
	if (piece == 'b') return 2;
	if (piece == 'p') return 1;
	 */
	assert(0);
	return 0;
}

int mvvlva(char piece, char cappiece) {
	return 10 * capval(cappiece) - capval(piece);
}

int sortScore(struct position *pos, struct move *move, struct move TTmove, int ply) {
	// Score
	char cappiece = move->cappiece;
	char piece = move->piece;
	int histval = history[pos->tomove][move->from][move->to];
	int butterflyval = butterfly[pos->tomove][move->from][move->to];
	double histscore = (double)histval;
	if (butterflyval != 0) histscore = (double)histval / (double)butterflyval;
	
	struct move prevmove = movestack[movestackend - 1];
	struct move countermove = countermoves[prevmove.from][prevmove.to];
	if (TTmove.from != -1
		&& (move->from == TTmove.from) && (move->to == TTmove.to) && (move->prom == TTmove.prom)) {
			return 5000000;
	}
	else if (cappiece != NONE
		&& cappiece >= piece) {
			return 1000000 + mvvlva(piece, cappiece);
	}
	else if (cappiece != NONE
		&& piece > cappiece) {
			return 700000 + mvvlva(piece, cappiece);
	}
	else if ((killers[ply][0].to == move->to) && (killers[ply][0].from == move->from) && (killers[ply][0].prom == move->prom)) {
		return 900000;
	}
	else if ((killers[ply - 2][0].to == move->to) && (killers[ply - 2][0].from == move->from) && (killers[ply - 2][0].prom == move->prom)) {
		return 875000;
	}
	else if ((killers[ply][1].to == move->to) && (killers[ply][1].from == move->from) && (killers[ply][1].prom == move->prom)) {
		return 850000;
	}
	else if ((killers[ply - 2][1].to == move->to) && (killers[ply - 2][1].from == move->from) && (killers[ply - 2][1].prom == move->prom)) {
		return 825000;
	}
	else if (histscore > 0.0) {
		histscore = 1000.0 + histscore * 100.0;
		if (histscore > 700000.0) {
			histscore = 700000.0;
		}
		return (int)histscore;
	}
	return 0;
}
void rootSortMoves(struct position *pos, struct move *moves, const int num_moves, struct move TTmove) {
	int scores[num_moves];
	for (int i = 0; i < num_moves; i++) {
		scores[i] = rootNodesSearched[moves[i].from][moves[i].to];
		if (TTmove.from != -1
			&& (moves[i].from == TTmove.from) && (moves[i].to == TTmove.to) && (moves[i].prom == TTmove.prom)) {
				scores[i] = INT_MAX;
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
void sortMoves(struct position *pos, struct move *moves, const int num_moves, struct move TTmove, int ply) {
	assert(moves);
	assert(pos);
	assert(num_moves < MAX_MOVES);
	int scores[num_moves];
	
	int bestSEE = 0;
	struct move bestSEEmove = {.to=-1, .from=-1, .prom=-1};
	int SEEvalue[num_moves];
	
	// Order capture with highest SEE value (> 0) above other captures
	for (int i = 0; i < num_moves; i++) {
		if (moves[i].cappiece == NONE) continue;
		int SEEval = SEEcapture(pos, moves[i].from, moves[i].to, pos->tomove);
		if (SEEval > bestSEE) {
			bestSEEmove = moves[i];
			bestSEE = SEEval;
		}
		SEEvalue[i] = SEEval;
	}
	
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
		else if (bestSEEmove.from == moves[i].from && bestSEEmove.to == moves[i].to && bestSEEmove.prom == moves[i].prom) {
			scores[i] = 4000000;
		}
		else if (cappiece != NONE
			&& SEEvalue[i] >= 0) {
				scores[i] = 1000000 + mvvlva(piece, cappiece);
		}
		else if (cappiece != NONE
			&& SEEvalue[i] < 0) {
				scores[i] = 700000 + mvvlva(piece, cappiece);
		}
		else if ((killers[ply][0].to == moves[i].to) && (killers[ply][0].from == moves[i].from) && (killers[ply][0].prom == moves[i].prom)) {
			scores[i] = 900000;
		}
		else if ((killers[ply - 2][0].to == moves[i].to) && (killers[ply - 2][0].from == moves[i].from) && (killers[ply - 2][0].prom == moves[i].prom)) {
			scores[i] = 875000;
		}
		else if ((killers[ply][1].to == moves[i].to) && (killers[ply][1].from == moves[i].from) && (killers[ply][1].prom == moves[i].prom)) {
			scores[i] = 850000;
		}
		else if ((killers[ply - 2][1].to == moves[i].to) && (killers[ply - 2][1].from == moves[i].from) && (killers[ply - 2][1].prom == moves[i].prom)) {
			scores[i] = 825000;
		}
		else if (moves[i].from == countermove.from && moves[i].to == countermove.to) {
			scores[i] = 800000;
		}
		else if (histscore > 0.0) {
			histscore = 1000.0 + histscore * 100.0;
			if (histscore > 700000.0) {
				histscore = 700000.0;
			}
			scores[i] = (int)histscore;
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
