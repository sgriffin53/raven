#ifndef TESTS_C
#define TESTS_C


#include <inttypes.h>
#include <math.h>
#include "tests.h"
#include "hash.h"
#include "perft.h"
#include "position.h"
#include "search.h"
#include "move.h"
#include "globals.h"


void testRunAll() {
	printf("Running all tests\n\n");
	testRunPerft();
	testRunMakemove();
	testRunMates();
	testRunBetaCutoffs();
}

void testRunPerft() {
	struct position pos;
	parsefen(&pos,"startpos");
	posstack[0] = pos;
	U64 perftVals[5] = {20, 400, 8902, 197281, 4865609};
	printf("Running perft tests\n");
	for (int i = 0;i < 5;i++) {
		U64 nodes = perft(&pos,i+1);
		U64 expectednodes = perftVals[i];
		if (nodes == expectednodes) {
			printf("Perft depth %d test - Passed (expected %" PRIu64 ", got %" PRIu64 ")\n",i+1,expectednodes,nodes);
		}
		else {
			printf("Perft depth %d test - Failed (expected %" PRIu64 ", got %" PRIu64 ")\n",i+1,expectednodes,nodes);
		}
	}
	printf("\n");
}

void testRunMakemove() {
	struct position pos;
	parsefen(&pos,"startpos");
	posstack[0] = pos;
	printf("Running makemove tests\n");
	U64 hash = generateHash(&pos);
	makeMovestr("e2e4",&pos);
	U64 newhash = generateHash(&pos);
	if (hash != newhash) {
		printf("Makemove test - Passed\n");
	}
	else {
		printf("Makemove test - Failed\n");
	}
	unmakeMove(&pos);
	newhash = generateHash(&pos);
	if (hash == newhash) {
		printf("Unmake move test - Passed\n");
	}
	else {
		printf("Unmake move test - Failed\n");
	}
	printf("\n");
}
void testRunMates() {
	struct position pos;
	struct move chosenmove;
	printf("Running mate in N tests\n\n");
	
	// Mate in 2 test
	printf("Running mate in 2 test\n");
	parsefen(&pos,"r2qkb1r/pp2nppp/3p4/2pNN1B1/2BnP3/3P4/PPP2PPP/R2bK2R w KQkq - 1 0");
	posstack[0] = pos;
	char bestmove[5] = "d5f6";
	bestmove[4] = 0;
	nodesSearched = 0;
	chosenmove = search(pos,2,10000);
	if (strcmp(movetostr(chosenmove),bestmove) == 0) {
		printf("Mate in 2 test - Passed (expected %s, got %s)\n",bestmove,movetostr(chosenmove));
	}
	else {
		printf("Mate in 2 test - Failed (expected %s, got %s)\n",bestmove,movetostr(chosenmove));
	}
	printf("\n");
	
	// Mate in 3 test
	printf("Running mate in 3 test\n");
	parsefen(&pos,"r1b1kb1r/pppp1ppp/5q2/4n3/3KP3/2N3PN/PPP4P/R1BQ1B1R b kq - 0 1");
	posstack[0] = pos;
	strcpy(bestmove,"f8c5");
	bestmove[4] = 0;
	nodesSearched = 0;
	chosenmove = search(pos,4,10000);
	if (strcmp(movetostr(chosenmove),bestmove) == 0) {
		printf("Mate in 3 test - Passed (expected %s, got %s)\n",bestmove,movetostr(chosenmove));
	}
	else {
		printf("Mate in 3 test - Failed (expected %s, got %s)\n",bestmove,movetostr(chosenmove));
	}
	printf("\n");
	
	// Mate in 4 test
	printf("Running mate in 4 test\n");
	parsefen(&pos,"r5rk/2p1Nppp/3p3P/pp2p1P1/4P3/2qnPQK1/8/R6R w - - 1 0");
	posstack[0] = pos;
	strcpy(bestmove,"h6g7");
	bestmove[4] = 0;
	nodesSearched = 0;
	chosenmove = search(pos,4,10000);
	if (strcmp(movetostr(chosenmove),bestmove) == 0) {
		printf("Mate in 4 test - Passed (expected %s, got %s)\n",bestmove,movetostr(chosenmove));
	}
	else {
		printf("Mate in 4 test - Failed (expected %s, got %s)\n",bestmove,movetostr(chosenmove));
	}
	printf("\n");
}
void testRunBetaCutoffs() {
	struct position pos;
	struct move chosenmove;
	parsefen(&pos,"r3kb1r/pp1n1ppp/1q3n2/2pP1QN1/2P2B2/8/PP2PPPP/R3KB1R w KQkq - 3 12");
	chosenmove = search(pos,6,3000);
	printf("Beta cutoff rate: %.2f%%",(float)(numinstantbetacutoffs * (100 / (float)numbetacutoffs)));
	printf("\n");
}
#endif