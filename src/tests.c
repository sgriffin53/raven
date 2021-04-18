
#include <stdio.h>
#include <time.h>
#include "tests.h"
#include "chess/position.h"
#include "globals.h"
#include "search/search.h"
#include "chess/makemove.h"
#include "search/eval.h"

void runTestsAll() {
	runTestsFlip();
	runTestsMakeMove();
	testRunBetaCutoffs();
}
void runTestsNPS() {
	printf("Running NPS test.\n");
	char path[256] = "tests\\nps30.epd";
	
	char *token;
	char splitstr[12][200];
	
    FILE *fp;
    char str[1024];
	
    fp = fopen(path, "r");
    if (fp == NULL){
        printf("Could not open file %s",path);
    }
	U64 totbetacutoffs;
	U64 totinstantbetacutoffs;
	int numentries = 0;
	
	// read entries
	
	totNodesSearched = 0;
	clock_t start = clock();
	silentsearch = 1;
	
    while (fgets(str, 1024, fp) != NULL) {
		//if (numentries >= 4000000) break; // limit number of entries
        //printf("%s", str);
		char fen[1024];
		str[strcspn(str, "\n")] = 0;
		//split str into tokens into splitstr by space
		token = strtok(str," ");

		int splitstrend = 0;
		while (token != NULL) {
			strcpy(splitstr[splitstrend],token);
			splitstrend++;
			token = strtok(NULL, " ");
		}
		
		strcpy(fen,splitstr[0]);
		strcat(fen, " ");
		strcat(fen, splitstr[1]);
		strcat(fen, " ");
		strcat(fen, splitstr[2]);
		strcat(fen, " ");
		strcat(fen, splitstr[3]);
		struct position pos;
		parsefen(&pos, fen);
		search(pos, 11, 100000, 1);
		totNodesSearched += nodesSearched;
		totbetacutoffs += (U64)numbetacutoffs;
		totinstantbetacutoffs += (U64)numinstantbetacutoffs;
		if (numentries % 5 == 0 && numentries > 0) printf("%d positions searched, %lu nodes.\n", numentries, totNodesSearched); // give regular updates if we're loading a large file
		numentries++;
		
	}
	clock_t end = clock();
	double time_spent = (double)(end - start) / CLOCKS_PER_SEC;
	int time_spentms = (int)(time_spent * 1000);
	int nps = totNodesSearched / time_spent;
	printf("Tested %d positions\n", numentries);
	printf("nodes Searched: %lu\n",totNodesSearched);
	printf("Time: %d\n", time_spentms / 1000);
	printf("NPS: %d\n", nps);
	printf("Beta cutoff rate: %.2f%%",(float)(totinstantbetacutoffs * (100 / (float)totbetacutoffs)));
	fclose(fp);
	silentsearch = 0;
}
void runTestsFlip() {
	
	printf("Running colour flipping test.\n");
	char path[256] = "tests\\flip.epd";
	
	char *token;
	char splitstr[12][200];
	
    FILE *fp;
    char str[1024];
	
    fp = fopen(path, "r");
    if (fp == NULL){
        printf("Could not open file %s",path);
    }
	
	int numentries = 0;
	int numpassed = 0;
	int numfailed = 0;
	
	// read entries
	 
    while (fgets(str, 1024, fp) != NULL) {
		//if (numentries >= 4000000) break; // limit number of entries
        //printf("%s", str);
		char fen[1024];
		str[strcspn(str, "\n")] = 0;
		//split str into tokens into splitstr by space
		token = strtok(str," ");

		int splitstrend = 0;
		while (token != NULL) {
			strcpy(splitstr[splitstrend],token);
			splitstrend++;
			token = strtok(NULL, " ");
		}
		
		strcpy(fen,splitstr[0]);
		strcat(fen, " ");
		strcat(fen, splitstr[1]);
		strcat(fen, " ");
		strcat(fen, splitstr[2]);
		strcat(fen, " ");
		strcat(fen, splitstr[3]);
		struct position pos;
		parsefen(&pos, fen);
		int eval = taperedEval(&pos);
		pos = flipBoard(&pos);
		int neweval = taperedEval(&pos);
		//printf("%s [", fen);
		if (eval == neweval) {
			numpassed++;
		//	printf("Passed] ");
		}
		else { 
		//	printf("Failed] ");
			printf("%s [Failed] (%d, %d)\n", fen, eval, neweval);
			numfailed++;
		}
		//printf("(%d, %d)\n", eval, neweval);
		if (numentries % 10000 == 0 && numentries > 0) printf("%d entries tested.\n", numentries); // give regular updates if we're loading a large file
		numentries++;
		
	}
	printf("Tested %d positions\n", numentries);
	printf("Passed: %d\n",numpassed);
	printf("Failed: %d\n", numfailed);
	printf("Total: %d\n",numentries);
	fclose(fp);
}
void testRunBetaCutoffs() {
	struct position pos;
	numbetacutoffs = 0;
	numinstantbetacutoffs = 0;
	parsefen(&pos,"startpos");
	search(pos,13,13000,0);
	printf("Beta cutoff rate: %.2f%%",(float)(numinstantbetacutoffs * (100 / (float)numbetacutoffs)));
	printf("\n");
}
void runTestsMakeMove() {
	printf("Running make move tests:\n\n");
	struct position pos;
	
	parsefen(&pos,"startpos");
	printf("White normal move test: ");
	makeMovestr("e2e4",&pos);
	if ((getPiece(&pos,E4) == PAWN) && (getPiece(&pos,E2) == NONE)) {
		printf("Passed\n");
	}
	else printf("Failed\n");
	
	parsefen(&pos,"startpos");
	printf("Black normal move test: ");
	makeMovestr("e2e4",&pos);
	makeMovestr("e7e5",&pos);
	if ((getPiece(&pos,E5) == PAWN) && (getPiece(&pos,E7) == NONE)) {
		printf("Passed\n");
	}
	else printf("Failed\n");
	
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