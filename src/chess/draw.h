#ifndef DRAW_H
#define DRAW_H

#include <assert.h>
#include "../globals.h"
#include "position.h"
#include "hash.h"
#include "bitboards.h"

static inline int isThreefold(struct position *pos) {
	assert(pos);
	assert(pos->halfmoves >= 0);
	U64 curposhash = generateHash(pos);
	currenthash = curposhash;
	if (pos->halfmoves <= 4) return 0;
	int numrepeats = 0;
	for (int i = (posstackend - 1);(i > (posstackend - 1 - pos->halfmoves - 1)) && i >= 0;i-=2) {
		U64 checkhash = hashstack[i];
		if (checkhash == 0) {
			struct position checkpos = posstack[i];
			checkhash = generateHash(&checkpos);
			hashstack[i] = checkhash;
		}
		if (checkhash == curposhash) {
			numrepeats++;
			if (numrepeats >= 3) {
				return 1;
			}
		}
	}
	return 0;
}

static inline int isInsufficientMaterial(struct position *pos) {
	if (!(pos->pieces[PAWN] | pos->pieces[KNIGHT] | pos->pieces[BISHOP] | pos->pieces[ROOK] | pos->pieces[QUEEN])) {
		// king vs king
		return 1;
	}
	if (!(pos->pieces[PAWN] | pos->pieces[KNIGHT] | pos->pieces[ROOK] | pos->pieces[QUEEN])) {
		if (__builtin_popcountll(pos->pieces[BISHOP]) == 1) {
			// KB vs K
			return 1;
		}
		else if (__builtin_popcountll(pos->colours[BLACK] & pos->pieces[BISHOP]) == 1 && __builtin_popcountll(pos->colours[WHITE] & pos->pieces[BISHOP]) == 1) {
			if (pos->colours[BLACK] & pos->pieces[BISHOP] & BBlightsquares && pos->colours[WHITE] & pos->pieces[BISHOP] & BBlightsquares) {
				// both have only LSB
				return 1;
			}
			if (pos->colours[BLACK] & pos->pieces[BISHOP] & BBdarksquares && pos->colours[WHITE] & pos->pieces[BISHOP] & BBdarksquares) {
				// both have only DSB
				return 1;
			}
		}
	}
	if (!(pos->pieces[PAWN] | pos->pieces[BISHOP] | pos->pieces[ROOK] | pos->pieces[QUEEN])) {
		if (__builtin_popcountll(pos->pieces[KNIGHT]) == 1) {
			// KN vs K
			return 1;
		}
	}
	return 0;
}
#endif
