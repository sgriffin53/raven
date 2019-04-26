#ifndef EVAL_H
#define EVAL_H

#include "PST.h"
#include "functions.h"

int pieceval(const char inpiece) {
	if (inpiece == 'p') return 100;
	if (inpiece == 'n') return 300;
	if (inpiece == 'b') return 300;
	if (inpiece == 'r') return 500;
	if (inpiece == 'q') return 900;
	if (inpiece == 'k') return 9999;
	if (inpiece == 'P') return 100;
	if (inpiece == 'N') return 300;
	if (inpiece == 'B') return 300;
	if (inpiece == 'R') return 500;
	if (inpiece == 'Q') return 900;
	if (inpiece == 'K') return 9999;

	assert(0);
	printf("inpiece: %d\n",inpiece);

	return 0;
}

int isPawnless(const struct position *pos) {
	for (int i=0;i<64;i++) {
		char piece = pos->board[i];
		if (piece != '0') {
			if ((piece == 'P') || (piece == 'p')) {
				return 0;
			}
		}
	}
	return 1;
}
int taperedEval(const struct position *pos) {
	assert(pos);
	int num_BP = 0;
	int num_BN = 0;
	int num_BB = 0;
	int num_BR = 0;
	int num_BQ = 0;
	int num_WP = 0;
	int num_WN = 0;
	int num_WB = 0;
	int num_WR = 0;
	int num_WQ = 0;
	int pawnPhase = 0;
	int knightPhase = 1;
	int bishopPhase = 1;
	int rookPhase = 2;
	int queenPhase = 4;
	int openingEval = 0;
	int endgameEval = 0;
	int material = 0;
	for (int i = 0;i<64;i++) {
		char piece = pos->board[i];
		if (piece != '0') {
			int pval = pieceval(piece);
			if (isBlackPiece(piece)) {
				pval = -pval;
			}
			openingEval += PSTval(piece,i,'O');
			endgameEval += PSTval(piece,i,'E');
			endgameEval += pval;
			openingEval += pval;
			material += pval;
			// bonus for R and Q being 2 squares or less away from enemy king
			if ((piece == 'R') || (piece == 'Q')) {
				int enemykingpos = pos->Bkingpos;
				int kingdistx = abs(getfile(i) - getfile(enemykingpos));
				int kingdisty = abs(getrank(i) - getrank(enemykingpos));
				if ((kingdistx <= 2) && (kingdisty <= 2)) {
					openingEval += 30;
					endgameEval += 30;
				}
			}
			if ((piece == 'r') || (piece == 'q')) {
				int enemykingpos = pos->Wkingpos;
				int kingdistx = abs(getfile(i) - getfile(enemykingpos));
				int kingdisty = abs(getrank(i) - getrank(enemykingpos));
				if ((kingdistx <= 2) && (kingdisty <= 2)) {
					openingEval -= 30;
					endgameEval -= 30;
				}
			}
			switch (piece) {
				case 'p': num_BP += 1; break;
				case 'n': num_BN += 1; break;
				case 'b': num_BB += 1; break;
				case 'r': num_BR += 1; break;
				case 'q': num_BQ += 1; break;
				case 'P': num_WP += 1; break;
				case 'N': num_WN += 1; break;
				case 'B': num_WB += 1; break;
				case 'R': num_WR += 1; break;
				case 'Q': num_WQ += 1; break;
			}
		}
	}
	if (num_BB >= 2) {
		openingEval -= 30;
		endgameEval -= 30;
	}
	if (num_WB >= 2) {
		openingEval += 30;
		endgameEval += 30;
	}
	int totalPhase = pawnPhase * 16 + knightPhase * 4 + bishopPhase*4 + rookPhase*4 + queenPhase*2;
	int phase = totalPhase;

	phase -= num_WP * pawnPhase;
	phase -= num_WN * knightPhase;
	phase -= num_WB * bishopPhase;
	phase -= num_WR * rookPhase;
	phase -= num_WQ * queenPhase;
	phase -= num_BP * pawnPhase;
	phase -= num_BN * knightPhase;
	phase -= num_BB * bishopPhase;
	phase -= num_BR * rookPhase;
	phase -= num_BQ * queenPhase;

	phase = (phase * 256 + (totalPhase / 2)) / totalPhase;

	int eval = ((openingEval * (256 - phase)) + (endgameEval * phase)) / 256;
	//printf("%d %d %d\n",openingEval,endgameEval,eval);
	if (pos->tomove == BLACK) eval = -eval;
	return eval;
}
int isEndgame(const struct position *pos) {
	int numpieces = 1;
	for (int i=0;i<64;i++) {
		char piece = pos->board[i];
		if (piece != '0') {
			if (pos->tomove == WHITE) {
				if ((piece == 'N') || (piece == 'B') || (piece == 'R') || (piece == 'Q')) {
					numpieces++;
					if (numpieces > 3) return 0;
				}
			}
			else {
				if ((piece == 'n') || (piece == 'b') || (piece == 'r') || (piece == 'q')) {
					numpieces++;
					if (numpieces > 3) return 0;
				}
			}
		}
	}
	if (numpieces <= 3) return 1;
	return 0;
}
int evalBoard(const struct position *pos) {
	assert(pos);
	int score = 0;
	for (int i = 0;i<64;i++) {
		char piece = pos->board[i];
		if (piece != '0') {
			int pval = pieceval(piece);
			if ((piece >= 'a') && (piece <= 'z')) {
				pval = -pval;
			}
			int pstscore = PSTval(piece,i,'O');
			score += pstscore;
			score += pval;
		}
	}
	if (pos->tomove == BLACK) return -score;
	return score;
}
#endif
