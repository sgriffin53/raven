#include "tests.h"
#include "position.h"
#include "move.h"
#include "makemove.h"
#include <stdio.h>

int posMatch(struct position *pos1, struct position *pos2) {
	for (int i = PAWN;i < KING;i++) {
		if (pos1->pieces[i] != pos2->pieces[i]) return 0;
	}
	if (pos1->colours[WHITE] != pos2->colours[WHITE]) return 0;
	if (pos1->colours[BLACK] != pos2->colours[BLACK]) return 0;
	if (pos1->halfmoves != pos2->halfmoves) return 0;
	if (pos1->WcastleQS != pos2->WcastleQS) return 0;
	if (pos1->WcastleKS != pos2->WcastleKS) return 0;
	if (pos1->BcastleQS != pos2->BcastleQS) return 0;
	if (pos1->BcastleKS != pos2->BcastleKS) return 0;
	if (pos1->epsquare != pos2->epsquare) return 0;
	return 1;
}
void runTestsMakeMove() {
	printf("Running make move tests:\n\n");
	struct position pos;
	
	parsefen(&pos,"startpos");
	struct position origpos = pos;
	printf("White normal move test: ");
	makeMovestr("e2e4",&pos);
	struct move testmove = {.from=E2, .to=E4, .cappiece=NONE, .prom=NONE, .type=DOUBLE };
	if ((getPiece(&pos,E4) == PAWN) && (getPiece(&pos,E2) == NONE)) {
		printf("Make: Passed\n");
	}
	else printf("Make: Failed\n");
	unmakeMove(&testmove, &pos);
	if (posMatch(&pos, &origpos)) {
		printf("Unmake: Passed\n");
	}
	else printf("Unmake: Failed\n");
	
	parsefen(&pos,"startpos");
	origpos = pos;
	struct move testmove1 = {.from=E2, .to=E4, .cappiece=NONE, .prom=NONE, .type=DOUBLE };
	struct move testmove2 = {.from=E7, .to=E5, .cappiece=NONE, .prom=NONE, .type=DOUBLE };
	printf("Black normal move test: ");
	makeMovestr("e2e4",&pos);
	makeMovestr("e7e5",&pos);
	if ((getPiece(&pos,E5) == PAWN) && (getPiece(&pos,E7) == NONE)) {
		printf("Passed\n");
	}
	else printf("Failed\n");
	unmakeMove(&testmove2, &pos);
	unmakeMove(&testmove1, &pos);
	if (posMatch(&pos, &origpos)) {
		printf("Unmake: Passed\n");
	}
	else printf("Unmake: Failed\n");
	
	parsefen(&pos,"startpos");
	printf("White en passant move test: ");
	makeMovestr("e2e5",&pos);
	makeMovestr("d7d5",&pos);
	makeMovestr("e5d6",&pos);
	if ((getPiece(&pos,D6) == PAWN) && (getPiece(&pos,D5) == NONE)) {
		printf("Passed\n");
	}
	else printf("Failed\n");
	
	parsefen(&pos,"startpos");
	printf("Black en passant move test: ");
	makeMovestr("a2a3",&pos);
	makeMovestr("d7d4",&pos);
	makeMovestr("e2e4",&pos);
	makeMovestr("d4e3",&pos);
	if ((getPiece(&pos,E3) == PAWN) && (getPiece(&pos,E4) == NONE)) {
		printf("Passed\n");
	}
	else printf("Failed\n");
	
	parsefen(&pos,"startpos");
	printf("White promotion test: ");
	makeMovestr("e2e7",&pos);
	makeMovestr("a7a6",&pos);
	makeMovestr("e7f8r",&pos);
	
	if ((getPiece(&pos,F8) == ROOK) && (getPiece(&pos,E7) == NONE)) {
		printf("Passed\n");
	}
	else printf("Failed\n");
	
	parsefen(&pos,"startpos");
	printf("Black promotion test: ");
	makeMovestr("a2a3",&pos);
	makeMovestr("d7d2",&pos);
	makeMovestr("a3a4",&pos);
	makeMovestr("d2c1r",&pos);
	
	
	if ((getPiece(&pos,C1) == ROOK) && (getPiece(&pos,D2) == NONE)) {
		printf("Passed\n");
	}
	else printf("Failed\n");
	
	parsefen(&pos,"r3k2r/8/8/8/8/8/8/R3K2R w - -");
	printf("White kingside castling test: ");
	makeMovestr("e1g1",&pos);
	
	if ((getPiece(&pos,G1) == KING) && (getPiece(&pos,F1) == ROOK)) {
		printf("Passed\n");
	}
	else printf("Failed\n");

	parsefen(&pos,"r3k2r/8/8/8/8/8/8/R3K2R w - -");
	printf("White queenside castling test: ");
	makeMovestr("e1c1",&pos);
	
	if ((getPiece(&pos,C1) == KING) && (getPiece(&pos,D1) == ROOK)) {
		printf("Passed\n");
	}
	else printf("Failed\n");
	
	parsefen(&pos,"r3k2r/8/8/8/8/8/8/R3K2R b - -");
	printf("Black kingside castling test: ");
	makeMovestr("e8g8",&pos);
	
	if ((getPiece(&pos,G8) == KING) && (getPiece(&pos,F8) == ROOK)) {
		printf("Passed\n");
	}
	else printf("Failed\n");
		parsefen(&pos,"r3k2r/8/8/8/8/8/8/R3K2R b - -");
		
	printf("Black queenside castling test: ");
	makeMovestr("e8c8",&pos);
	
	if ((getPiece(&pos,C8) == KING) && (getPiece(&pos,D8) == ROOK)) {
		printf("Passed\n");
	}
	else printf("Failed\n");
	
}