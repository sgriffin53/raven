#ifndef TESTS_H
#define TESTS_H

#include "chess/position.h"
#include "chess/move.h"

void runTestsTTentry();
void runTestsAll();
void runTestsMakeMove();
void testRunBetaCutoffs();
void runTestsFlip();
void runTestsNPS();
void runTestsPerft();
void runTestsIsCheck();
void runTestsMoveConsistency();
void moveConsistencyAsserts(struct position *pos, struct move *move);
struct IsCheckPair {
	int true;
	char fen[256];
};
struct TTtestEntry {
	char fen[256];
	U64 hash;
	int depth;
	int flag;
	struct move bestmove;
	int bestscore;
};
#endif