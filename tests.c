
#include <stdio.h>
#include "tests.h"
#include "position.h"
#include "globals.h"
#include "search.h"
#include "makemove.h"

void runTestsAll() {
	runTestsMakeMove();
	testRunBetaCutoffs();
}
void testRunBetaCutoffs() {
	struct position pos;
	numbetacutoffs = 0;
	numinstantbetacutoffs = 0;
	parsefen(&pos,"startpos");
	search(pos,6,3000);
	printf("Beta cutoff rate: %.2f%%",(float)(numinstantbetacutoffs * (100 / (float)numbetacutoffs)));
	printf("\n");
}
void runTestsMakeMove() {
	printf("Running make move tests:\n\n");
	struct position pos;
	
	parsefen(&pos,"startpos");
	printf("White normal move test: ");
	makeMovestr("e2e4",&pos);
	if ((getPiece(&pos,E4) == 'P') && (getPiece(&pos,E2) == '0')) {
		printf("Passed\n");
	}
	else printf("Failed\n");
	
	parsefen(&pos,"startpos");
	printf("Black normal move test: ");
	makeMovestr("e2e4",&pos);
	makeMovestr("e7e5",&pos);
	if ((getPiece(&pos,E5) == 'p') && (getPiece(&pos,E7) == '0')) {
		printf("Passed\n");
	}
	else printf("Failed\n");
	
	parsefen(&pos,"startpos");
	printf("White en passant move test: ");
	makeMovestr("e2e5",&pos);
	makeMovestr("d7d5",&pos);
	makeMovestr("e5d6",&pos);
	if ((getPiece(&pos,D6) == 'P') && (getPiece(&pos,D5) == '0')) {
		printf("Passed\n");
	}
	else printf("Failed\n");
	
	parsefen(&pos,"startpos");
	printf("Black en passant move test: ");
	makeMovestr("a2a3",&pos);
	makeMovestr("d7d4",&pos);
	makeMovestr("e2e4",&pos);
	makeMovestr("d4e3",&pos);
	if ((getPiece(&pos,E3) == 'p') && (getPiece(&pos,E4) == '0')) {
		printf("Passed\n");
	}
	else printf("Failed\n");
	
	parsefen(&pos,"startpos");
	printf("White promotion test: ");
	makeMovestr("e2e7",&pos);
	makeMovestr("a7a6",&pos);
	makeMovestr("e7f8r",&pos);
	
	if ((getPiece(&pos,F8) == 'R') && (getPiece(&pos,E7) == '0')) {
		printf("Passed\n");
	}
	else printf("Failed\n");
	
	parsefen(&pos,"startpos");
	printf("Black promotion test: ");
	makeMovestr("a2a3",&pos);
	makeMovestr("d7d2",&pos);
	makeMovestr("a3a4",&pos);
	makeMovestr("d2c1r",&pos);
	
	
	if ((getPiece(&pos,C1) == 'r') && (getPiece(&pos,D2) == '0')) {
		printf("Passed\n");
	}
	else printf("Failed\n");
	
	parsefen(&pos,"r3k2r/8/8/8/8/8/8/R3K2R w - -");
	printf("White kingside castling test: ");
	makeMovestr("e1g1",&pos);
	
	if ((getPiece(&pos,G1) == 'K') && (getPiece(&pos,F1) == 'R')) {
		printf("Passed\n");
	}
	else printf("Failed\n");

	parsefen(&pos,"r3k2r/8/8/8/8/8/8/R3K2R w - -");
	printf("White queenside castling test: ");
	makeMovestr("e1c1",&pos);
	
	if ((getPiece(&pos,C1) == 'K') && (getPiece(&pos,D1) == 'R')) {
		printf("Passed\n");
	}
	else printf("Failed\n");
	
	parsefen(&pos,"r3k2r/8/8/8/8/8/8/R3K2R b - -");
	printf("Black kingside castling test: ");
	makeMovestr("e8g8",&pos);
	
	if ((getPiece(&pos,G8) == 'k') && (getPiece(&pos,F8) == 'r')) {
		printf("Passed\n");
	}
	else printf("Failed\n");
		parsefen(&pos,"r3k2r/8/8/8/8/8/8/R3K2R b - -");
		
	printf("Black queenside castling test: ");
	makeMovestr("e8c8",&pos);
	
	if ((getPiece(&pos,C8) == 'k') && (getPiece(&pos,D8) == 'r')) {
		printf("Passed\n");
	}
	else printf("Failed\n");
	
}