
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "tests.h"
#include "chess/position.h"
#include "globals.h"
#include "search/search.h"
#include "chess/makemove.h"
#include "search/eval.h"
#include "search/perft.h"
#include "chess/movegen.h"
#include "chess/hash.h"
#include "search/TT.h"

void runTestsAll() {
	runTestsFlip();
	runTestsMakeMove();
	testRunBetaCutoffs();
}

void runTestsTTentry() {
	printf("Running TT entry test.\n");
	struct TTtestEntry positions[6] = {
		{ .fen="startpos", .hash=0, .depth=6, .bestmove={.from=E2, .to=E4, .piece=PAWN, .cappiece=NONE, .prom=NONE}, .flag=LOWERBOUND, .bestscore=60 },
		{ .fen="1Q2Q2Q/8/3ppp2/1R1pkp1R/3ppp2/8/1B2Q2B/4K3 b - - 0 1", .hash=0, .depth=12, .bestmove={.from=F4, .to=F3, .piece=PAWN, .cappiece=NONE, .prom=NONE}, .flag=EXACT, .bestscore=321 },
		{ .fen="4k3/8/2B5/8/8/8/8/4K3 b - - 0 1", .hash=0, .depth=3, .bestmove={.from=E8, .to=F8, .piece=KING, .cappiece=NONE, .prom=NONE}, .flag=UPPERBOUND, .bestscore=504 },
		{ .fen="4k3/3R4/3N4/8/8/8/8/4K3 b - - 0 1", .hash=0, .depth=1, .bestmove={.from=E8, .to=D7, .piece=KING, .cappiece=ROOK, .prom=NONE}, .flag=EXACT, .bestscore=511 },
		{ .fen="4k3/8/2Q5/8/8/8/8/4K3 b - - 0 1", .hash=0, .depth=90, .bestmove={.from=E8, .to=F7, .piece=KING, .cappiece=NONE, .prom=NONE}, .flag=LOWERBOUND, .bestscore=-420 }
		
	};
	for (int i = 0;i < 5;i++) {

		char fen[64];
		strcpy(fen, positions[i].fen);
		struct position pos;
		parsefen(&pos, fen);
		U64 hash = generateHash(&pos);
		positions[i].hash = hash;
		addTTentry(&TT, hash, positions[i].depth, positions[i].flag, positions[i].bestmove, positions[i].bestscore);
		struct TTentry TTdata = getTTentry(&TT, hash);
		if (TTdata.hash != positions[i].hash) {
			printf("Failed\n");
			return;
		}
		if (TTdata.depth != positions[i].depth) {
			printf("Failed\n");
			return;
		}
		if (TTdata.bestmove.from != positions[i].bestmove.from
			|| TTdata.bestmove.to != positions[i].bestmove.to
			|| TTdata.bestmove.piece != positions[i].bestmove.piece
			|| TTdata.bestmove.cappiece != positions[i].bestmove.cappiece
			|| TTdata.bestmove.prom != positions[i].bestmove.prom) {
			printf("Failed\n");
			return;
		}
		if (TTdata.flag != positions[i].flag) {
			printf("Failed\n");
			return;
		}
		if (TTdata.score != positions[i].bestscore) {
			printf("Failed\n");
			return;
		}
	}
	printf("All TT entry tests passed. 5 positions tested.\n");
}
void moveConsistencyAsserts(struct position *pos, struct move *move) {
	assert(getPiece(pos, move->to) >= PAWN && getPiece(pos, move->to) <= NONE);
	assert(getPiece(pos, move->from) >= PAWN && getPiece(pos, move->from) <= NONE);
	assert(getPiece(pos, move->to) != KING);
	assert(getPiece(pos, move->to) == move->cappiece);
	assert(getPiece(pos, move->from) == move->piece);
	assert(move->to >= 0 && move->to <= 63);
	assert(move->from >= 0 && move->from <= 63);
	assert(move->to != move->from);
	assert(move->prom != PAWN);
	assert(move->prom != KING);
}
void runTestsMoveConsistency() {
	printf("Running move consistency test.\n");
	char positions[18][64] = {
		"startpos",
		"1Q2Q2Q/8/3ppp2/1R1pkp1R/3ppp2/8/1B2Q2B/4K3 b - - 0 1",
		"4k3/8/3N4/8/8/8/8/4K3 b - - 0 1",
		"4k3/8/2B5/8/8/8/8/4K3 b - - 0 1",
		"4k3/8/2Q5/8/8/8/8/4K3 b - - 0 1",
		"4k3/8/4Q3/8/8/8/8/4K3 b - - 0 1",
		"4k3/8/4R3/8/8/8/8/4K3 b - - 0 1",
		"1R2k3/8/8/8/8/8/8/4K3 b - - 0 1",
		"4k3/3P4/8/8/8/8/8/4K3 b - - 0 1",
		"4k3/5P2/8/8/8/8/8/4K3 b - - 0 1",
		"4K3/8/3n4/8/8/8/8/4k3 w - - 0 1",
		"4K3/8/2b5/8/8/8/8/4k3 w - - 0 1",
		"4K3/8/4q3/8/8/8/8/4k3 w - - 0 1",
		"4K3/8/2q5/8/8/8/8/4k3 w - - 0 1",
		"2r1K3/8/8/8/8/8/8/4k3 w - - 0 1",
		"4K3/8/4r3/8/8/8/8/4k3 w - - 0 1",
		"8/3p4/4K3/8/8/8/8/4k3 w - - 0 1",
		"8/5p2/4K3/8/8/8/8/4k3 w - - 0 1"
	};
	for (int i = 0;i < 18;i++) {
		
		char fen[64];
		strcpy(fen, positions[i]);
		struct position pos;
		parsefen(&pos, fen);
		struct move moves[MAX_MOVES];
		int num_moves = genMoves(&pos, moves, 0);
		U64 hash = generateHash(&pos);
		
		for (int j = 0;j < num_moves;j++) {
			int cappiece = getPiece(&pos, moves[j].to);
			int piece = getPiece(&pos, moves[j].from);
			int origtomove = pos.tomove;
			// before making move
			moveConsistencyAsserts(&pos, &moves[j]);
			if (piece != PAWN) assert(moves[j].prom == NONE);
			if (moves[j].prom != NONE) assert(piece == PAWN);
			if (pos.tomove == WHITE && piece == PAWN && moves[j].prom != NONE) {
				assert(getrank(moves[j].from) == 6);
			}
			else if (pos.tomove == BLACK && piece == PAWN && moves[j].prom != NONE) {
				assert(getrank(moves[j].from) == 1);
			}
				
			
			makeMove(&moves[j],&pos);
			
			pos.tomove = !pos.tomove;
			if (isCheck(&pos)) {
				unmakeMove(&pos);
				continue;
			}
			
			pos.tomove = !pos.tomove;
			
			assert(pos.tomove != origtomove);
			
			U64 newhash = generateHash(&pos);
			assert(newhash != hash);
			
			// after making move
			
			if (moves[i].prom == NONE) {
				assert(getPiece(&pos, moves[j].to) == piece);
			}
			assert(getPiece(&pos, moves[j].from) == NONE);
			if (moves[i].prom != NONE) {
				assert(getPiece(&pos, moves[j].to) == moves[j].prom);
			}
			
			unmakeMove(&pos);
			assert(pos.tomove == origtomove);
			U64 afterhash = generateHash(&pos);
			assert(afterhash == hash);
		}
	}
	printf("All move consistency tests passed. 18 positions tested.\n");
}
void runTestsIsCheck() {
	printf("Running is_check test.\n");
	struct IsCheckPair positions[18] = {
		{.true=0, .fen="startpos"},
		{.true=0, .fen="1Q2Q2Q/8/3ppp2/1R1pkp1R/3ppp2/8/1B2Q2B/4K3 b - - 0 1"},
		{.true=1, .fen="4k3/8/3N4/8/8/8/8/4K3 b - - 0 1"},
		{.true=1, .fen="4k3/8/2B5/8/8/8/8/4K3 b - - 0 1"},
		{.true=1, .fen="4k3/8/2Q5/8/8/8/8/4K3 b - - 0 1"},
		{.true=1, .fen="4k3/8/4Q3/8/8/8/8/4K3 b - - 0 1"},
		{.true=1, .fen="4k3/8/4R3/8/8/8/8/4K3 b - - 0 1"},
		{.true=1, .fen="1R2k3/8/8/8/8/8/8/4K3 b - - 0 1"},
		{.true=1, .fen="4k3/3P4/8/8/8/8/8/4K3 b - - 0 1"},
		{.true=1, .fen="4k3/5P2/8/8/8/8/8/4K3 b - - 0 1"},
		{.true=1, .fen="4K3/8/3n4/8/8/8/8/4k3 w - - 0 1"},
		{.true=1, .fen="4K3/8/2b5/8/8/8/8/4k3 w - - 0 1"},
		{.true=1, .fen="4K3/8/4q3/8/8/8/8/4k3 w - - 0 1"},
		{.true=1, .fen="4K3/8/2q5/8/8/8/8/4k3 w - - 0 1"},
		{.true=1, .fen="2r1K3/8/8/8/8/8/8/4k3 w - - 0 1"},
		{.true=1, .fen="4K3/8/4r3/8/8/8/8/4k3 w - - 0 1"},
		{.true=1, .fen="8/3p4/4K3/8/8/8/8/4k3 w - - 0 1"},
		{.true=1, .fen="8/5p2/4K3/8/8/8/8/4k3 w - - 0 1"}
	};
	for (int i = 0;i < 18;i++) {
		int expected = positions[i].true;
		char fen[256];
		strcpy(fen, positions[i].fen);
		//char fen[256] = positions[i].fen;
		struct position pos;
		parsefen(&pos, fen);
		int result = isCheck(&pos);
		if (expected != result) {
			printf("Failed on %s\n",fen);
			return;
		}
	}
	printf("All is_check tests passed. 18 positions tested.\n");
}
void runTestsPerft() {
	printf("Running perft test.\n");
	char path[256] = "tests\\perftsuite.epd";
	
	char *token;
	char splitstr[12][200];
	
    FILE *fp;
    char str[1024];
	
    fp = fopen(path, "r");
    if (fp == NULL){
        printf("Could not open file %s",path);
		return;
    }
	int totpositions = 0;
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
		int expected[7];
		for (int i = 0;i < 6;i++) expected[i] = -1;
		expected[0] = atoi(splitstr[7]);
		if (splitstrend >= 10) expected[1] = atoi(splitstr[9]);
		if (splitstrend >= 12) expected[2] = atoi(splitstr[11]);
		if (splitstrend >= 14) expected[3] = atoi(splitstr[13]);
		if (splitstrend >= 16) expected[4] = atoi(splitstr[15]);
		if (splitstrend >= 18) expected[5] = atoi(splitstr[17]);
		expected[6] = -1;
		parsefen(&pos, fen);
		printf("%s\n", fen);
		//printf("D1: %d D2: %d D3: %d\n", expected[0], expected[1], expected[2]);
		int i = 0;
		while (expected[i] != -1) {
			int nodes = perft(&pos, i+1);
			if (nodes != expected[i]) {
				printf("failed on %s at depth %d\n", fen, i+1);
				return;
			}
			i++;
		}
		totpositions++;
		
	}
	printf("All perft tests passed: %d positions.\n", totpositions);
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
		return;
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
