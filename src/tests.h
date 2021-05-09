#ifndef TESTS_H
#define TESTS_H

void runTestsAll();
void runTestsMakeMove();
void testRunBetaCutoffs();
void runTestsFlip();
void runTestsNPS();
void runTestsPerft();
void runTestsIsCheck();
void runTestsMoveConsistency();

struct IsCheckPair {
	int true;
	char fen[256];
};
#endif