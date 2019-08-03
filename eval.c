#include "eval.h"
#include "position.h"
#include "PST.h"
#include "attacks.h"
#include "bitboards.h"
#include "magicmoves.h"
#include "globals.h"
#include "misc.h"
#include <stdio.h>
#include <stdlib.h>

const int arrCenterManhattanDistance[64] = { // char is sufficient as well, also unsigned
  6, 5, 4, 3, 3, 4, 5, 6,
  5, 4, 3, 2, 2, 3, 4, 5,
  4, 3, 2, 1, 1, 2, 3, 4,
  3, 2, 1, 0, 0, 1, 2, 3,
  3, 2, 1, 0, 0, 1, 2, 3,
  4, 3, 2, 1, 1, 2, 3, 4,
  5, 4, 3, 2, 2, 3, 4, 5,
  6, 5, 4, 3, 3, 4, 5, 6
};

int pieceval(const char inpiece) {
	switch (inpiece) {
		case 'p':
		case 'P': return 100;
		case 'n':
		case 'N': return 300;
		case 'b':
		case 'B': return 300;
		case 'r':
		case 'R': return 525;
		case 'q':
		case 'Q': return 900;
		case 'k':
		case 'K': return 9999;
	}
	/*
	if (inpiece == 'p') return 100;
	if (inpiece == 'n') return 300;
	if (inpiece == 'b') return 300;
	if (inpiece == 'r') return 525;
	if (inpiece == 'q') return 900;
	if (inpiece == 'k') return 9999;
	if (inpiece == 'P') return 100;
	if (inpiece == 'N') return 300;
	if (inpiece == 'B') return 300;
	if (inpiece == 'R') return 525;
	if (inpiece == 'Q') return 900;
	if (inpiece == 'K') return 9999;
*/
	assert(0);

	return 0;
}

int taperedEval(struct position *pos) {
	assert(pos);
	
	/*
	U64 hash = generateHash(pos);
	struct ETTentry ETTdata = getETTentry(&ETT,hash);
	if (ETTdata.hash == hash) {
		return ETTdata.eval;
	}
	*/
	int pawnPhase = 0;
	int knightPhase = 1;
	int bishopPhase = 1;
	int rookPhase = 2;
	int queenPhase = 4;
	int openingEval = 0;
	int endgameEval = 0;
	int material = 0;
	int pstvalO, pstvalE;
	U64 BBoccupied = (pos->BBwhitepieces | pos->BBblackpieces);
	
	int num_BP = __builtin_popcountll(pos->BBblackpieces & pos->BBpawns);
	int num_BN = __builtin_popcountll(pos->BBblackpieces & pos->BBknights);
	int num_BB = __builtin_popcountll(pos->BBblackpieces & pos->BBbishops);
	int num_BR = __builtin_popcountll(pos->BBblackpieces & pos->BBrooks);
	int num_BQ = __builtin_popcountll(pos->BBblackpieces & pos->BBqueens);
	int num_WP = __builtin_popcountll(pos->BBwhitepieces & pos->BBpawns);
	int num_WN = __builtin_popcountll(pos->BBwhitepieces & pos->BBknights);
	int num_WB = __builtin_popcountll(pos->BBwhitepieces & pos->BBbishops);
	int num_WR = __builtin_popcountll(pos->BBwhitepieces & pos->BBrooks);
	int num_WQ = __builtin_popcountll(pos->BBwhitepieces & pos->BBqueens);
	
	// Piece values
    int white_pieces = num_WP*pieceval('P')   +
                       num_WN*pieceval('N') +
                       num_WB*pieceval('B') +
                       num_WR*pieceval('R')   +
                       num_WQ*pieceval('Q');

    int black_pieces = num_BP*pieceval('p')   +
                       num_BN*pieceval('n') +
                       num_BB*pieceval('b') +
                       num_BR*pieceval('r')   +
                       num_BQ*pieceval('q');

    openingEval += white_pieces - black_pieces;
	endgameEval += white_pieces - black_pieces;
	material += white_pieces - black_pieces;
	

	while (BBoccupied != 0) {
		int square = __builtin_ctzll(BBoccupied);
		//BBoccupied &= ~(1ULL << square);
		BBoccupied &= BBoccupied - 1;
		char piece = getPiece(pos,square);
		//int piececol;
		//if ((piece >= 'a') && (piece <= 'z')) {
		//	piececol = BLACK;
		//}
		//else piececol = WHITE;
	//	int pval = pieceval(piece);
	//	if ((piece >= 'a') && (piece <= 'z')) {
	//		pval = -pval;
	//	}
		pstvalO = PSTval(piece,square,'O');
		pstvalE = PSTval(piece,square,'E');
		openingEval += pstvalO;
		endgameEval += pstvalE;
		//endgameEval += pval;
		//openingEval += pval;
		//material += pval;
		/*
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
		*/
	}
	
	// side to move bonus
	
	if (pos->tomove == WHITE) {
		openingEval += 20;
		endgameEval += 20;
	}
	else {
		openingEval -= 20;
		endgameEval -= 20;
	}
	
	// bonus for pieces being near enemy king
	
	// white pieces attacking black king

	int enemykingpos = pos->Bkingpos;
	U64 BBkingdist1 = BBkingattacks(pos->BBkings & (1ULL << enemykingpos)); // fill 1 square away
	U64 BBattackers = BBkingdist1 & (pos->BBwhitepieces & (pos->BBqueens | pos->BBrooks | pos->BBknights | pos->BBpawns));
	/*
	while (BBattackers) {
		int square = __builtin_ctzll(BBattackers);
		BBattackers &= ~(1ULL << square);
		openingEval += 20;
		endgameEval += 20;
	}
	 */
	openingEval += 20 * __builtin_popcountll(BBattackers);
	endgameEval += 20 * __builtin_popcountll(BBattackers);
	U64 BBkingdist2 = BBkingattacks(BBkingdist1); // fill 2 squares away
	BBkingdist2 = BBkingdist2 & ~(BBkingdist1);
	BBattackers = BBkingdist2 & (pos->BBwhitepieces & (pos->BBrooks | pos->BBqueens | pos->BBknights | pos->BBpawns));
	/*
	while (BBattackers) {
		int square = __builtin_ctzll(BBattackers);
		BBattackers &= ~(1ULL << square);
		openingEval += 0;
		endgameEval += 40;
	}
	 */
	endgameEval += 40 * __builtin_popcountll(BBattackers);
	U64 BBkingdist3 = BBkingattacks(BBkingdist2); // fill 3 squares away
	BBkingdist3 = BBkingdist3 & ~(BBkingdist2);
	BBattackers = BBkingdist3 & (pos->BBwhitepieces & (pos->BBrooks | pos->BBqueens | pos->BBknights | pos->BBpawns));
	/*
	while (BBattackers) {
		int square = __builtin_ctzll(BBattackers);
		BBattackers &= ~(1ULL << square);
		openingEval += 10;
		endgameEval += 10;
	}
	 */
	openingEval += 10 * __builtin_popcountll(BBattackers);
	endgameEval += 10 * __builtin_popcountll(BBattackers);
	U64 BBkingdist4 = BBkingattacks(BBkingdist3); // fill 4 squares away
	BBkingdist4 = BBkingdist4 & ~(BBkingdist3);
	BBattackers = BBkingdist4 & (pos->BBwhitepieces & (pos->BBrooks | pos->BBqueens | pos->BBknights | pos->BBpawns));
	/*
	while (BBattackers) {
		int square = __builtin_ctzll(BBattackers);
		BBattackers &= ~(1ULL << square);
		openingEval += 10;
		endgameEval += 10;
	}
	*/
	openingEval += 10 * __builtin_popcountll(BBattackers);
	endgameEval += 10 * __builtin_popcountll(BBattackers);
	// black pieces attacking white king
	enemykingpos = pos->Wkingpos;
	BBkingdist1 = BBkingattacks(pos->BBkings & (1ULL << enemykingpos)); // fill 1 square away
	BBattackers = BBkingdist1 & (pos->BBblackpieces & (pos->BBqueens | pos->BBrooks | pos->BBknights | pos->BBpawns));
	/*
	while (BBattackers) {
		int square = __builtin_ctzll(BBattackers);
		BBattackers &= ~(1ULL << square);
		openingEval -= 20;
		endgameEval -= 20;
	}
	 */
	openingEval -= 20 * __builtin_popcountll(BBattackers);
	endgameEval -= 20 * __builtin_popcountll(BBattackers);
	
	BBkingdist2 = BBkingattacks(BBkingdist1); // fill 2 squares away
	BBkingdist2 = BBkingdist2 & ~(BBkingdist1);
	BBattackers = BBkingdist2 & (pos->BBblackpieces & (pos->BBrooks | pos->BBqueens | pos->BBknights | pos->BBpawns));
	/*
	while (BBattackers) {
		int square = __builtin_ctzll(BBattackers);
		BBattackers &= ~(1ULL << square);
		openingEval -= 0;
		endgameEval -= 40;
	}
	 */
	endgameEval -= 40 * __builtin_popcountll(BBattackers);
	BBkingdist3 = BBkingattacks(BBkingdist2); // fill 3 squares away
	BBkingdist3 = BBkingdist3 & ~(BBkingdist2);
	BBattackers = BBkingdist3 & (pos->BBblackpieces & (pos->BBrooks | pos->BBqueens | pos->BBknights | pos->BBpawns));
	/*
	while (BBattackers) {
		int square = __builtin_ctzll(BBattackers);
		BBattackers &= ~(1ULL << square);
		openingEval -= 10;
		endgameEval -= 10;
	}
	 */
	openingEval -= 10 * __builtin_popcountll(BBattackers);
	endgameEval -= 10 * __builtin_popcountll(BBattackers);
	BBkingdist4 = BBkingattacks(BBkingdist3); // fill 4 squares away
	BBkingdist4 = BBkingdist4 & ~(BBkingdist3);
	BBattackers = BBkingdist4 & (pos->BBblackpieces & (pos->BBrooks | pos->BBqueens | pos->BBknights | pos->BBpawns));
	/*
	while (BBattackers) {
		int square = __builtin_ctzll(BBattackers);
		BBattackers &= ~(1ULL << square);
		openingEval -= 10;
		endgameEval -= 10;
	}
	 */
	openingEval -= 10 * __builtin_popcountll(BBattackers);
	endgameEval -= 10 * __builtin_popcountll(BBattackers);
	
	
	
	
	
	// passed pawns
	
	//int WpassedRankBonus[8] = {0,10,40,50,60,80,100,0};
	//int BpassedRankBonus[8] = {0,100,80,60,50,40,10,0};
	

	int WpassedRankBonus[8] = {0, 10, 10, 15, 25, 80, 120, 0};
	int BpassedRankBonus[8] = {0, 120, 80, 25, 15, 10, 10, 0};
	
	U64 BBwhitePP = 0ULL;
	U64 BBblackPP = 0ULL;
	
	U64 BBwhitepawns = (pos->BBwhitepieces & pos->BBpawns);
	while (BBwhitepawns) {
		
		// passed pawns
		
		int square = __builtin_ctzll(BBwhitepawns);
		//BBwhitepawns &= ~(1ULL << square);
		BBwhitepawns &= BBwhitepawns - 1;
		U64 BBpiece = (1ULL << square);
		U64 BBmidsquare = BBpiece;
		U64 BBchecksquares = 0ULL;
		int startrank = getrank(square);
		int rank = startrank;
		//printf("\n%d\n",rank);
		U64 BBenemypawns = 0ULL;
		while (rank < 6) {
			BBchecksquares |= noWeOne(BBmidsquare);
			BBchecksquares |= northOne(BBmidsquare);
			BBchecksquares |= noEaOne(BBmidsquare);
			BBmidsquare = northOne(BBmidsquare);
			BBenemypawns = (BBchecksquares & (pos->BBblackpieces & pos->BBpawns));
			if (BBenemypawns) break;
			rank++;
		}
		if (BBenemypawns == 0) {
			// pawn is passed
			
			BBwhitePP |= square; // add square to bb of white passed pawns
			int bonus = WpassedRankBonus[startrank];
			openingEval += 0.5 * bonus;
			endgameEval += 1 * bonus;
			
			// bonus for bishop on same colour square as promotion square
			/*
			int promsquare = fileranktosquareidx(getfile(square),7);
			U64 BBpromsquare = 1ULL << promsquare;
			U64 BBislight = BBpromsquare & BBlightsquares; // if prom square is light
			U64 BBsamecolbishops;
			if (BBislight) BBsamecolbishops = pos->BBbishops & pos->BBwhitepieces & BBlightsquares;
			else BBsamecolbishops = pos->BBbishops & pos->BBwhitepieces & BBdarksquares;
			if (BBsamecolbishops) endgameEval += 50;
			 */
		}
		
		// pawn chain bonus
		U64 BBpawnattacks = BBpawnEastAttacksB(BBpiece) | BBpawnWestAttacksB(BBpiece);
		if ((BBpawnattacks & BBwhitepawns)) {
			openingEval += 20;
			endgameEval += 20;
		}
	}
	
	U64 BBblackpawns = (pos->BBblackpieces & pos->BBpawns);
	while (BBblackpawns) {
		// passed pawns
		int square = __builtin_ctzll(BBblackpawns);
		//BBblackpawns &= ~(1ULL << square);
		BBblackpawns &= BBblackpawns - 1;
		U64 BBpiece = (1ULL << square);
		U64 BBmidsquare = BBpiece;
		U64 BBchecksquares = 0ULL;
		int startrank = getrank(square);
		int rank = startrank;
		//printf("\n%d\n",rank);
		U64 BBenemypawns = 0ULL;
		while (rank > 1) {
			BBchecksquares |= soWeOne(BBmidsquare);
			BBchecksquares |= southOne(BBmidsquare);
			BBchecksquares |= soEaOne(BBmidsquare);
			BBmidsquare = southOne(BBmidsquare);
			BBenemypawns = (BBchecksquares & (pos->BBwhitepieces & pos->BBpawns));
			if (BBenemypawns) break;
			rank--;
		}
		if (BBenemypawns == 0) {
			BBblackPP |= square;
			int bonus = BpassedRankBonus[startrank];
			openingEval -= 0.5 * bonus;
			endgameEval -= 1 * bonus;
			
			// bonus for bishop on same colour square as promotion square
			/*
			int promsquare = fileranktosquareidx(getfile(square),0);
			U64 BBpromsquare = 1ULL << promsquare;
			U64 islight = BBpromsquare & BBlightsquares; // if prom square is light
			U64 BBsamecolbishops;
			if (islight) BBsamecolbishops = pos->BBbishops & pos->BBblackpieces & BBlightsquares;
			else BBsamecolbishops = pos->BBbishops & pos->BBblackpieces & BBdarksquares;
			if (BBsamecolbishops) endgameEval -= 50;
			 */
		}
		
		// pawn chain bonus
		
		// pawn chain bonus
		U64 BBpawnattacks = BBpawnEastAttacksW(BBpiece) | BBpawnWestAttacksW(BBpiece);
		if ((BBpawnattacks & BBblackpawns)) {
			openingEval -= 20;
			endgameEval -= 20;
		}
	}
	
	// penalty for passed pawns being blocked
	/*
	// white
	openingEval -= 10 * __builtin_popcountll((BBwhitePP<<8) & (pos->BBwhitepieces | pos->BBblackpieces));
	endgameEval -= 20 * __builtin_popcountll((BBwhitePP<<8) & (pos->BBwhitepieces | pos->BBblackpieces));
	
	// black
	openingEval += 10 * __builtin_popcountll((BBblackPP>>8) & (pos->BBwhitepieces | pos->BBblackpieces));
	endgameEval += 20 * __builtin_popcountll((BBblackPP>>8) & (pos->BBwhitepieces | pos->BBblackpieces));
	*/
	// king gets a bonus for how close it is to enemy passed pawns in the endgame
	/*
	if (isEndgame(pos)) {
		if (pos->tomove == BLACK) {
			while (BBwhitePP) {
				int square = __builtin_ctzll(BBwhitePP);
				BBwhitePP &= ~(1ULL << square);
				int kingpos = pos->Bkingpos;
				int xdist = abs(getfile(kingpos) - getfile(square));
				int ydist = abs(getrank(kingpos) - getrank(square));
				int dist = max(xdist, ydist);
				int movestopromote = 7 - getrank(square);
				if (dist > movestopromote) {
					endgameEval += 100;
				}
				else {
					//endgameEval -= 40;
				}
				//endgameEval -= (10 * (6 - dist));
			}
		}	
		else if (pos->tomove == WHITE) {
			while (BBblackPP) {
				int square = __builtin_ctzll(BBblackPP);
				BBblackPP &= ~(1ULL << square);
				int kingpos = pos->Wkingpos;
				int xdist = abs(getfile(kingpos) - getfile(square));
				int ydist = abs(getrank(kingpos) - getrank(square));
				int dist = max(xdist, ydist);
				int movestopromote = getrank(square);
				if (dist > movestopromote) {
					endgameEval -= 100;	
				}
				else {
				//	endgameEval += 40;
				}
				//endgameEval += (10 * (6 - dist));
			}
		}
	}
	*/
	
	// king gets a bonus for being next to a friendly passed pawn
	/*
	if (isEndgame(pos)) {
		if (pos->tomove == BLACK) {
			while (BBblackPP) {
				int square = __builtin_ctzll(BBblackPP);
				BBblackPP &= ~(1ULL << square);
				int kingpos = pos->Bkingpos;
				int xdist = abs(getfile(kingpos) - getfile(square));
				int ydist = abs(getrank(kingpos) - getrank(square));
				int dist = max(xdist, ydist);
				if (dist == 1) {
					endgameEval -= 40;
				}
				//endgameEval -= (10 * (6 - dist));
			}
		}	
		else if (pos->tomove == WHITE) {
			while (BBwhitePP) {
				int square = __builtin_ctzll(BBwhitePP);
				BBwhitePP &= ~(1ULL << square);
				int kingpos = pos->Wkingpos;
				int xdist = abs(getfile(kingpos) - getfile(square));
				int ydist = abs(getrank(kingpos) - getrank(square));
				int dist = max(xdist, ydist);
				if (dist == 1) {
					endgameEval += 40;
				}
				//endgameEval -= (10 * (6 - dist));
			}
		}
	}
	*/
	// bonus for king being near opponent's king in endgame
	/*
	int winningside = 2;
	if (isEndgame(pos)) {
		if (material > 0) {
			winningside = WHITE;
		}
		else if (material < 0) winningside = BLACK;
		if (pos->tomove == WHITE && winningside == WHITE) {
			int xdist = abs(getfile(pos->Wkingpos) - getfile(pos->Bkingpos));
			int ydist = abs(getrank(pos->Wkingpos) - getrank(pos->Bkingpos));
			int dist = max(xdist, ydist);
			endgameEval += (6 - dist) * 20;
		}
		if (pos->tomove == BLACK && winningside == BLACK) {
			int xdist = abs(getfile(pos->Wkingpos) - getfile(pos->Bkingpos));
			int ydist = abs(getrank(pos->Wkingpos) - getrank(pos->Bkingpos));
			int dist = max(xdist, ydist);
			endgameEval -= (6 - dist) * 20;
		}
	}
	 */
	// give bonus for kings being close to the winning side in endgames
	
	int winningside;
	if (isEndgame(pos)) {
		if (material > 0) {
			winningside = WHITE;
		}
		else if (material < 0) winningside = BLACK;
		if (winningside == WHITE) {
			endgameEval += arrCenterManhattanDistance[pos->Bkingpos] * 10;
			int xdist = abs(getfile(pos->Wkingpos) - getfile(pos->Bkingpos));
			int ydist = abs(getrank(pos->Wkingpos) - getrank(pos->Bkingpos));
			int dist = max(xdist, ydist);
			endgameEval += (6 - dist) * 10;
		}
		else if (winningside == BLACK) {
			endgameEval -= arrCenterManhattanDistance[pos->Wkingpos] * 10;
			int xdist = abs(getfile(pos->Wkingpos) - getfile(pos->Bkingpos));
			int ydist = abs(getrank(pos->Wkingpos) - getrank(pos->Bkingpos));
			int dist = max(xdist, ydist);
			endgameEval -= (6 - dist) * 10;
		}
	}
	
	// bishop and knight mate
	/*
	U64 BBwhitenonBNmaterial = (pos->BBpawns | pos->BBqueens | pos->BBrooks) & pos->BBwhitepieces;
	U64 BBblackmaterial = (pos->BBpawns | pos->BBqueens | pos->BBbishops | pos->BBknights | pos->BBrooks) & pos->BBblackpieces;
	
	if (!BBwhitenonBNmaterial && !BBblackmaterial && num_WB == 1 && num_WN == 1) {
		// KBN vs K endgame, white has the B+N
		// penalty for king going into wrong corner, bonus for king going into right corner
		int islight = pos->BBbishops & pos->BBwhitepieces & BBlightsquares;
		int distx = abs(getfile(pos->Bkingpos) - getfile(A1));
		int disty = abs(getrank(pos->Bkingpos) - getrank(A1));
		int distfromA1 = max(distx, disty);
		distx = abs(getfile(pos->Bkingpos) - getfile(H8));
		disty = abs(getrank(pos->Bkingpos) - getrank(H8));
		int distfromH8 = max(distx, disty);
		distx = abs(getfile(pos->Bkingpos) - getfile(A8));
		disty = abs(getrank(pos->Bkingpos) - getrank(A8));
		int distfromA8 = max(distx, disty);
		distx = abs(getfile(pos->Bkingpos) - getfile(H1));
		disty = abs(getrank(pos->Bkingpos) - getrank(H1));
		int distfromH1 = max(distx, disty);
		int closestcorner;
		int closestdist = 10;
		int corners[4] = { A1, A8, H1, H8 };
		int cornerscores[4] = { distfromA1, distfromA8, distfromH1, distfromH8 };
		for (int i = 0;i < 4;i++) {
			int cdist = cornerscores[i];
			if (cdist < closestdist) {
				closestdist = cdist;
				closestcorner = corners[i];
			}
		}
		if (closestcorner == H8 || closestcorner == A1) { // enemy king is on a dark corner square
			if (islight) endgameEval -= 60 * (4 - closestdist);
			else endgameEval += 60 * (4 - closestdist);
		}
		if (closestcorner == A8 || closestcorner == H1) { // enemy king is on a light corner square
			if (islight) endgameEval += 60 * (4 - closestdist);
			else endgameEval -= 60 * (4 - closestdist);
		}
	}
	U64 BBblacknonBNmaterial = (pos->BBpawns | pos->BBqueens | pos->BBrooks) & pos->BBblackpieces;
	U64 BBwhitematerial = (pos->BBpawns | pos->BBqueens | pos->BBbishops | pos->BBknights | pos->BBrooks) & pos->BBwhitepieces;
	if (!BBblacknonBNmaterial && !BBwhitematerial && num_BB == 1 && num_BN == 1) {
		// KBN vs K endgame, white has the B+N
		// penalty for king going into wrong corner, bonus for king going into right corner
		int islight = pos->BBbishops & pos->BBblackpieces & BBlightsquares;
		int distfromA1, distfromH8, distfromA8, distfromH1;
		int distx = abs(getfile(pos->Wkingpos) - getfile(A1));
		int disty = abs(getrank(pos->Wkingpos) - getrank(A1));
		distfromA1 = max(distx, disty);
		distx = abs(getfile(pos->Wkingpos) - getfile(H8));
		disty = abs(getrank(pos->Wkingpos) - getrank(H8));
		distfromH8 = max(distx, disty);
		distx = abs(getfile(pos->Wkingpos) - getfile(A8));
		disty = abs(getrank(pos->Wkingpos) - getrank(A8));
		distfromA8 = max(distx, disty);
		distx = abs(getfile(pos->Wkingpos) - getfile(H1));
		disty = abs(getrank(pos->Wkingpos) - getrank(H1));
		distfromH1 = max(distx, disty);
		int closestcorner;
		int closestdist = 10;
		int corners[4] = { A1, A8, H1, H8 };
		int cornerscores[4] = { distfromA1, distfromA8, distfromH1, distfromH8 };
		for (int i = 0;i < 4;i++) {
			int cdist = cornerscores[i];
			if (cdist < closestdist) {
				closestdist = cdist;
				closestcorner = corners[i];
			}
		}
		if (closestcorner == H8 || closestcorner == A1) { // enemy king is on a dark corner square
			if (islight) endgameEval += 30 * (6 - closestdist);
			else endgameEval -= 30 * (6 - closestdist);
		}
		if (closestcorner == A8 || closestcorner == H1) { // enemy king is on a light corner square
			if (islight) endgameEval -= 30 * (6 - closestdist);
			else endgameEval += 30 * (6 - closestdist);
		}
	}
	 */
	 
	// king and rook mate
	
	U64 BBwhitenonrookmaterial = (pos->BBpawns | pos->BBqueens | pos->BBbishops | pos->BBknights) & pos->BBwhitepieces;
	U64 BBblackmaterial = (pos->BBpawns | pos->BBqueens | pos->BBbishops | pos->BBknights | pos->BBrooks) & pos->BBblackpieces;
	if  (!BBwhitenonrookmaterial && !BBblackmaterial && num_WR == 1) {
		// KR vs K endgame, white has the rook
		// give a bonus for the enemy king's centre manhattan distance
		//endgameEval += arrCenterManhattanDistance[pos->Bkingpos] * 10;
		int xdist = abs(getfile(pos->Wkingpos) - getfile(pos->Bkingpos));
		int ydist = abs(getrank(pos->Wkingpos) - getrank(pos->Bkingpos));
		
		if (xdist == 2 && ydist == 0) {
			// king is opposing enemy king two files away
			endgameEval += 200;
			// check if enemy king is on same file as rook
			int square = __builtin_ctzll(pos->BBwhitepieces & pos->BBrooks);
			if (getfile(pos->Bkingpos) == getfile(square)) {
				endgameEval += 300;
			}
		}
		if (ydist == 2 && xdist == 0) {
			// king is opposing enemy king two ranks away
			endgameEval += 200;
			// check if enemy king is on same rank as rook
			int square = __builtin_ctzll(pos->BBwhitepieces & pos->BBrooks);
			if (getrank(pos->Bkingpos) == getrank(square)) {
				endgameEval += 300;
			}
		}
	} 
	U64 BBblacknonrookmaterial = (pos->BBpawns | pos->BBqueens | pos->BBbishops | pos->BBknights) & pos->BBblackpieces;
	U64 BBwhitematerial = (pos->BBpawns | pos->BBqueens | pos->BBbishops | pos->BBknights | pos->BBrooks) & pos->BBwhitepieces;
	//printf("%d\n",num_BR);
	if  (!BBblacknonrookmaterial == 1 && !BBwhitematerial && num_BR == 1) {
		// KR vs K endgame, black has the rook
		
		// give a bonus for the enemy king's centre manhattan distance
		//endgameEval += arrCenterManhattanDistance[pos->Wkingpos] * 10;
		
		int xdist = abs(getfile(pos->Wkingpos) - getfile(pos->Bkingpos));
		int ydist = abs(getrank(pos->Wkingpos) - getrank(pos->Bkingpos));
		
		if (xdist == 2 && ydist == 0) {
			// king is opposing enemy king two files away
			endgameEval -= 200;
			// check if enemy king is on same file as rook
			int square = __builtin_ctzll(pos->BBblackpieces & pos->BBrooks);
			if (getfile(pos->Wkingpos) == getfile(square)) {
				endgameEval -= 300;
			}
		}
		if (ydist == 2 && xdist == 0) {
			// king is opposing enemy king two ranks away
			endgameEval -= 200;
			// check if enemy king is on same rank as rook
			int square = __builtin_ctzll(pos->BBblackpieces & pos->BBrooks);
			if (getrank(pos->Wkingpos) == getrank(square)) {
				endgameEval -= 300;
			}
		}
	} 
	
	// bonus for rooks trapping kings on the edge
	/*
	int kingonedgebonus = 15;
	if (pos->tomove == WHITE) {
		int enemykingpos = pos->Bkingpos;
		int rank = getrank(enemykingpos);
		int file = getfile(enemykingpos);
		if (rank == 7) { // 8th rank
			if (!(pos->BBpawns & BBrank7) && (pos->BBrooks & pos->BBwhitepieces & BBrank7)) { // no pawns on 7th rank, white rook(s) on 7th rank
				endgameEval += kingonedgebonus;
			}
		}
		else if (rank == 0) { // 1st rank
			if (!(pos->BBpawns & BBrank1) && (pos->BBrooks & pos->BBwhitepieces & BBrank1)) { // no pawns on 1st rank, white rook(s) on 1st rank
				endgameEval += kingonedgebonus;
			}
		}
		if (file == 0) { // A file
			if ((!pos->BBpawns & BBfileB) && (pos->BBrooks & pos->BBwhitepieces & BBfileB)) {
				endgameEval += kingonedgebonus;
			}
		}
		else if (file == 7) { // H file
			if ((!pos->BBpawns & BBfileG) && (pos->BBrooks & pos->BBwhitepieces & BBfileG)) {
				endgameEval += kingonedgebonus;
			}
		}
	}
	 
	else if (pos->tomove == BLACK) {
		int enemykingpos = pos->Wkingpos;
		int rank = getrank(enemykingpos);
		int file = getfile(enemykingpos);
		if (rank == 7) { // 8th rank
			if (!(pos->BBpawns & BBrank7) && (pos->BBrooks & pos->BBblackpieces & BBrank7)) { // no pawns on 7th rank, black rook(s) on 7th rank
				endgameEval -= kingonedgebonus;
			}
		}
		else if (rank == 0) { // 1st rank
			if (!(pos->BBpawns & BBrank1) && (pos->BBrooks & pos->BBblackpieces & BBrank1)) { // no pawns on 1st rank, black rook(s) on 1st rank
				endgameEval -= kingonedgebonus;
			}
		}
		if (file == 0) { // A file
			if ((!pos->BBpawns & BBfileB) && (pos->BBrooks & pos->BBblackpieces & BBfileB)) {
				endgameEval -= kingonedgebonus;
			}
		}
		else if (file == 7) { // H file
			if ((!pos->BBpawns & BBfileG) && (pos->BBrooks & pos->BBblackpieces & BBfileG)) {
				endgameEval -= kingonedgebonus;
			}
		}
	}
	*/
	 
	// loop to check for doubled pawns and rooks on open files
	
	for (int i = 0;i < 8;i++) {
		// doubled pawns
		// white pawns
		U64 BBfilemask = BBfileA << i;
		
		U64 BBallpawnsonfile = BBfilemask & pos->BBpawns;
		U64 BBallrooksonfile = BBfilemask & pos->BBrooks;
		if (!BBallpawnsonfile && !BBallrooksonfile) continue;
		
		U64 BBWpawnsonfile = BBfilemask & (pos->BBwhitepieces & pos->BBpawns);
		
		U64 BBisdoubled = BBWpawnsonfile & (BBWpawnsonfile-1);
		if (BBisdoubled) {
			openingEval -= 16;
			endgameEval -= 16;
		}
		// black pawns
		U64 BBBpawnsonfile = BBfilemask & (pos->BBblackpieces & pos->BBpawns);
		BBisdoubled = BBBpawnsonfile & (BBBpawnsonfile-1);
		if (BBisdoubled) {
			openingEval += 16;
			endgameEval += 16;
		}
		
		// isolated pawns

		if (BBWpawnsonfile) {
			U64 BBleftpawns = westOne(BBfilemask) & (pos->BBwhitepieces & pos->BBpawns);
			U64 BBrightpawns = eastOne(BBfilemask) & (pos->BBwhitepieces & pos->BBpawns);
			if (BBleftpawns == 0 && BBrightpawns == 0) {
				openingEval -= 6;
				endgameEval -= 6;
			}
		}
		if (BBBpawnsonfile) {
			U64 BBleftpawns = westOne(BBfilemask) & (pos->BBblackpieces & pos->BBpawns);
			U64 BBrightpawns = eastOne(BBfilemask) & (pos->BBblackpieces & pos->BBpawns);
			if (BBleftpawns == 0 && BBrightpawns == 0) {
				openingEval += 6;
				endgameEval += 6;
			}
		}
		// rooks on open files
		U64 BBpawnsonfile = BBfilemask & pos->BBpawns;
		// white rook on open file
		U64 BBWrooksonfile = BBfilemask & (pos->BBrooks & pos->BBwhitepieces);
		if (BBWrooksonfile) {
			if (BBpawnsonfile == 0) {
				// white rook on open file
				openingEval += 48;
				endgameEval += 16;
			}
			if ((BBWpawnsonfile == 0) && (BBBpawnsonfile)) {
				// white rook on semi-open file with black pawns
				openingEval += 6;
				endgameEval += 6;
			}
			/*
			if ((BBWpawnsonfile) && (!BBBpawnsonfile)) {
				// white rook on semi-open file with white pawns
				openingEval += 12;
				endgameEval += 12;
			}
			 */
		}
		// black rooks on open file
		U64 BBBrooksonfile = BBfilemask & (pos->BBrooks & pos->BBblackpieces);
		if (BBBrooksonfile) {
			if (BBpawnsonfile == 0) {
				// black rook on open file
				openingEval -= 48;
				endgameEval -= 16;
			}
			if ((BBBpawnsonfile == 0) && (BBWpawnsonfile)) {
				// black rook on semi-open file with white pawns
				openingEval -= 6;
				endgameEval -= 6;
			}
			/*
			if ((BBBpawnsonfile) && (!BBWpawnsonfile)) {
				// black rook on semi-open file with black pawns
				openingEval -= 12;
				endgameEval -= 12;
			}
			 */
		}

		// rooks on same file as queen
		U64 BBWqueensonfile = BBfilemask & (pos->BBqueens & pos->BBwhitepieces);
		U64 BBBqueensonfile = BBfilemask & (pos->BBqueens & pos->BBblackpieces);
		if (BBWrooksonfile) {
			if (BBBqueensonfile) {
				openingEval += 40;
				endgameEval += 40;
			}
		}
		if (BBBrooksonfile) {
			if (BBWqueensonfile) {
				openingEval -= 40;
				endgameEval -= 40;
			}
		}
	}

	// pawn shield
	
	// white pawn shield
	
	int Wkingpos = pos->Wkingpos;
	BBkingdist1 = BBkingattacks((1ULL << Wkingpos)); // fill 1 square away
	U64 BBpawnshield = BBkingdist1 & (pos->BBwhitepieces & pos->BBpawns);
	/*
	while (BBpawnshield) {
		int square = __builtin_ctzll(BBpawnshield);
		BBpawnshield &= ~(1ULL << square);
		openingEval += 30;
		//endgameEval += 0;
	}
	 */
	openingEval += 30 * __builtin_popcountll(BBpawnshield);
	
	// black pawn shield
	
	int Bkingpos = pos->Bkingpos;
	BBkingdist1 = BBkingattacks((1ULL << Bkingpos)); // fill 1 square away
	BBpawnshield = BBkingdist1 & (pos->BBblackpieces & pos->BBpawns);
	/*
	while (BBpawnshield) {
		int square = __builtin_ctzll(BBpawnshield);
		BBpawnshield &= ~(1ULL << square);
		openingEval -= 30;
		//endgameEval -= 0;
	}
	 */
	openingEval -= 30 * __builtin_popcountll(BBpawnshield);
	
	// king safety - bonus for friendly pieces around the king
	/*
	// white
	
	BBkingdist1 = BBkingattacks(1ULL << pos->Wkingpos); // fill 1 square away
	U64 BBfriendlypieces = (BBkingdist1 & (pos->BBwhitepieces & ~pos->BBpawns));
	openingEval += 5 * __builtin_popcountll(BBfriendlypieces);
	
	// black
	
	BBkingdist1 = BBkingattacks(1ULL << pos->Bkingpos); // fill 1 square away
	BBfriendlypieces = (BBkingdist1 & (pos->BBblackpieces & ~pos->BBpawns));
	openingEval -= 5 * __builtin_popcountll(BBfriendlypieces);
	*/
	
	/*
	if (pos->tomove == WHITE) {
		openingEval += 25;
		endgameEval += 5;
	}
	if (pos->tomove == BLACK) {
		openingEval -= 25;
		endgameEval -= 5;
	}
	*/
	// penalty for bishops having pawns of same colour
	/*
	// white 
	U64 BBbishops = pos->BBbishops & pos->BBwhitepieces;
	while (BBbishops) {
		int square = __builtin_ctzll(BBbishops);
		BBbishops &= BBbishops - 1;
		int islight = 0;
		if ((1ULL << square) & BBlightsquares) islight = 1;
		else islight = 0;
		U64 BBsamecolpawns;
		if (islight) BBsamecolpawns = pos->BBpawns & pos->BBwhitepieces & BBlightsquares;
		else BBsamecolpawns = pos->BBpawns & pos->BBwhitepieces & BBdarksquares;
		endgameEval -= 3 * __builtin_popcountll(BBsamecolpawns);
	}
	
	// black
	BBbishops = pos->BBbishops & pos->BBblackpieces;
	while (BBbishops) {
		int square = __builtin_ctzll(BBbishops);
		BBbishops &= BBbishops - 1;
		int islight = 0;
		if ((1ULL << square) & BBlightsquares) islight = 1;
		else islight = 0;
		U64 BBsamecolpawns;
		if (islight) BBsamecolpawns = pos->BBpawns & pos->BBblackpieces & BBlightsquares;
		else BBsamecolpawns = pos->BBpawns & pos->BBblackpieces & BBdarksquares;
		endgameEval += 3 * __builtin_popcountll(BBsamecolpawns);
	}
	*/
	
	// bishop pair bonus
	
	if (num_BB >= 2) {
		openingEval -= 60;
		endgameEval -= 60;
	}
	if (num_WB >= 2) {
		openingEval += 60;
		endgameEval += 60;
	}
	
	// penalties for 8 or 0 pawns
	
	if (num_WP == 0 || num_WP == 8) {
		openingEval -= 10;
		endgameEval -= 10;
	}
	if (num_BP == 0 || num_BP == 8) {
		openingEval += 10;
		endgameEval += 10;
	}
	
	// bonus for pawns in centre
	
	U64 BBWpiecesincentre = (pos->BBwhitepieces & pos->BBpawns & BBcentre);
	/*
	while (BBWpiecesincentre) {
		int square = __builtin_ctzll(BBWpiecesincentre);
		//BBWpiecesincentre &= ~(1ULL << square);
		BBWpiecesincentre &= BBWpiecesincentre - 1;
		openingEval += 20;
		endgameEval += 20;
	}
	 */
	openingEval += 20 * __builtin_popcountll(BBWpiecesincentre);
	endgameEval += 20 * __builtin_popcountll(BBWpiecesincentre);
	
	U64 BBBpiecesincentre = (pos->BBblackpieces & pos->BBpawns & BBcentre);
	/*
	while (BBBpiecesincentre) {
		int square = __builtin_ctzll(BBBpiecesincentre);
		//BBBpiecesincentre &= ~(1ULL << square);
		BBBpiecesincentre &= BBBpiecesincentre - 1;
		openingEval -= 20;
		endgameEval -= 20;
	}
	 */
	openingEval -= 20 * __builtin_popcountll(BBBpiecesincentre);
	endgameEval -= 20 * __builtin_popcountll(BBBpiecesincentre);
	
	// bonus for connected knights
	// white
	if (num_WN >= 2) {
		U64 BBWknights = (pos->BBwhitepieces & pos->BBknights);
		U64 BBattacks = BBknightattacks(BBWknights);
		U64 BBconnectedknights = BBattacks & BBWknights;
		if (BBconnectedknights) {
			openingEval += 25;
			endgameEval += 25;
			//if (BBconnectedknights & BBcentre) {
			//	openingEval += 25;
			//	endgameEval += 25;
			//}
		}
	}
	// black
	if (num_BN >= 2) {
		U64 BBBknights = (pos->BBblackpieces & pos->BBknights);
		U64 BBattacks = BBknightattacks(BBBknights);
		U64 BBconnectedknights = BBattacks & BBBknights;
		if (BBconnectedknights) {
			openingEval -= 25;
			endgameEval -= 25;
			//if (BBconnectedknights & BBcentre) {
			//	openingEval -= 25;
			//	endgameEval -= 25;
			//}
		}
	}
	
	// bonus for trading when ahead in material
	
	int whitematval = num_WN * pieceval('N') + num_WB * pieceval('B') + num_WR * pieceval('R') + num_WQ * pieceval('Q');
	int blackmatval = num_BN * pieceval('N') + num_BB * pieceval('B') + num_BR * pieceval('R') + num_BQ * pieceval('Q');
	if (whitematval > blackmatval) {
		double matimb = 1.0 - (blackmatval / whitematval);
		openingEval += matimb * 180;
		endgameEval += matimb * 180;
	}
	
	else if (blackmatval > whitematval) {
		double matimb = 1.0 - (whitematval / blackmatval);
		openingEval -= matimb * 180;
		endgameEval -= matimb * 180;
	}
	
	
	// bonus for connected rooks
	/*
	// white
	if (num_WR >= 2) {
		U64 BBrooks = (pos->BBrooks & pos->BBwhitepieces);
		U64 BBrooksstart = BBrooks;
		U64 BBoccupancy = (pos->BBwhitepieces | pos->BBblackpieces);
		while (BBrooks) {
			int square = __builtin_ctzll(BBrooks);
			BBrooks &= ~(1ULL << square);
			int BBattacks = Rmagic(square,BBoccupancy);
			if (BBattacks & BBrooksstart) {
				openingEval += 40;
				endgameEval += 40;
				break;
			}
		}
	}
	
	// black
	if (num_BR >= 2) {
		U64 BBrooks = (pos->BBrooks & pos->BBblackpieces);
		U64 BBrooksstart = BBrooks;
		U64 BBoccupancy = (pos->BBwhitepieces | pos->BBblackpieces);
		while (BBrooks) {
			int square = __builtin_ctzll(BBrooks);
			BBrooks &= ~(1ULL << square);
			int BBattacks = Rmagic(square,BBoccupancy);
			if (BBattacks & BBrooksstart) {
				openingEval -= 40;
				endgameEval -= 40;
				break;
			}
		}
	}
	 */
	// knights protected by pawns
	
	// white
	
	BBwhitepawns = (pos->BBwhitepieces & pos->BBpawns);
	U64 BBwhiteknights = (pos->BBwhitepieces & pos->BBknights);
	while (BBwhiteknights) {
		int square = __builtin_ctzll(BBwhiteknights);
		//BBwhiteknights &= ~(1ULL << square);
		BBwhiteknights &= BBwhiteknights - 1;
		if ((BBpawnWestAttacksB(1ULL << square) & BBwhitepawns) || (BBpawnEastAttacksB(1ULL << square) & BBwhitepawns)) {
			openingEval += 20;
			endgameEval += 20;
		}
	}
	
	// black
	
	BBblackpawns = (pos->BBblackpieces & pos->BBpawns);
	U64 BBblackknights = (pos->BBblackpieces & pos->BBknights);
	while (BBblackknights) {
		int square = __builtin_ctzll(BBblackknights);
		//BBblackknights &= ~(1ULL << square);
		BBblackknights &= BBblackknights - 1;
		if ((BBpawnWestAttacksW(1ULL << square) & BBblackpawns) || (BBpawnEastAttacksW(1ULL << square) & BBblackpawns)) {
			openingEval -= 20;
			endgameEval -= 20;
		}
	}
	
	// bishops protected by pawns
	
	// white
	
	U64 BBwhitebishops = (pos->BBwhitepieces & pos->BBbishops);
	while (BBwhitebishops) {
		int square = __builtin_ctzll(BBwhitebishops);
		//BBwhitebishops &= ~(1ULL << square);
		BBwhitebishops &= BBwhitebishops - 1;
		if ((BBpawnWestAttacksB(1ULL << square) & BBwhitepawns) || (BBpawnEastAttacksB(1ULL << square) & BBwhitepawns)) {
			openingEval += 20;
			endgameEval += 20;
		}
	}
	
	// black
	
	U64 BBblackbishops = (pos->BBblackpieces & pos->BBbishops);
	while (BBblackbishops) {
		int square = __builtin_ctzll(BBblackbishops);
		//BBblackbishops &= ~(1ULL << square);
		BBblackbishops &= BBblackbishops - 1;
		if ((BBpawnWestAttacksW(1ULL << square) & BBblackpawns) || (BBpawnEastAttacksW(1ULL << square) & BBblackpawns)) {
			openingEval -= 20;
			endgameEval -= 20;
		}
	}
	
	
	// mobility bonuses
	
	int Wmobility = mobility(pos,WHITE);
	int Bmobility = mobility(pos,BLACK);
	
	openingEval += Wmobility * 5;
	endgameEval += Wmobility * 5;
	
	openingEval -= Bmobility * 5;
	endgameEval -= Bmobility * 5;
	

	// knight value decreases as pawns disappear
	
	openingEval -= num_WN * (16 - (num_WP + num_BP)) * 4;
	endgameEval -= num_WN * (16 - (num_WP + num_BP)) * 4;
	openingEval += num_BN * (16 - (num_WP + num_BP)) * 4;
	endgameEval += num_BN * (16 - (num_WP + num_BP)) * 4;
	
	//bishop value increases as pawns disappear
	//openingEval += num_WB * (16 - (num_WP + num_BP)) * 16;
	//endgameEval += num_WB * (16 - (num_WP + num_BP)) * 16;
	//openingEval -= num_BB * (16 - (num_WP + num_BP)) * 16;
	//endgameEval -= num_BB * (16 - (num_WP + num_BP)) * 16;
	/*
	openingEval += __builtin_popcountll(pos->BBwhitepieces & BBbigcentre) * 24;
	endgameEval += __builtin_popcountll(pos->BBwhitepieces & BBbigcentre) * 8;
	
	openingEval -= __builtin_popcountll(pos->BBblackpieces & BBbigcentre) * 24;
	endgameEval -= __builtin_popcountll(pos->BBblackpieces & BBbigcentre) * 8;
	*/
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
	//addETTentry(&ETT,hash,eval);
	return eval;
}

int mobility(struct position *pos, int side) {
	U64 BBsidepieces;
	if (side == WHITE) BBsidepieces = pos->BBwhitepieces;
	else BBsidepieces = pos->BBblackpieces;
	U64 BBallowed = ~BBsidepieces;
	U64 BBoccupied = pos->BBwhitepieces | pos->BBblackpieces;
	U64 BBmoves = 0;
	U64 BBcopy = 0;
	int from = 0;
	
	// Knights
	BBcopy = pos->BBknights & BBsidepieces;
	while(BBcopy)
	{
		from = __builtin_ctzll(BBcopy);
		BBmoves |= BBknightattacks(from) & BBallowed;
		BBcopy &= BBcopy-1;
	}

	// Bishops and Queens
	BBcopy = (pos->BBbishops | pos->BBqueens) & BBsidepieces;
	while(BBcopy)
	{
		from = __builtin_ctzll(BBcopy);
		BBmoves |= Bmagic(from, BBoccupied) & BBallowed;
		BBcopy &= BBcopy-1;
	}
	
	// Rooks and Queens
	BBcopy = (pos->BBrooks | pos->BBqueens) & BBsidepieces;
	while(BBcopy)
	{
		from = __builtin_ctzll(BBcopy);
		BBmoves |= Rmagic(from, BBoccupied) & BBallowed;
		BBcopy &= BBcopy-1;
	}
	
	int count = __builtin_popcountll(BBmoves);
	U64 BBcentreattacks = BBmoves & BBcentre;
	count += __builtin_popcountll(BBcentreattacks);
	return count;
	
}
int isEndgame(struct position *pos) {
	int numpieces = 1;
	//BBoccupied = pos->BBwhitepieces | pos->BBblackpieces;
	U64 BBoccupied = pos->BBwhitepieces;
	if (pos->tomove == BLACK) BBoccupied = pos->BBblackpieces;
	while (BBoccupied != 0) {
		int square = __builtin_ctzll(BBoccupied);
		//BBoccupied &= ~(1ULL << square);
		BBoccupied &= BBoccupied - 1;
		char piece = getPiece(pos,square);
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
	/*
	for (int i=0;i<64;i++) {
		char piece = getPiece(pos,i);
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
	 */
	if (numpieces <= 3) return 1;
	return 0;
}
int evalBoard(struct position *pos) {
	
	int num_BP = __builtin_popcountll(pos->BBblackpieces & pos->BBpawns);
	int num_BN = __builtin_popcountll(pos->BBblackpieces & pos->BBknights);
	int num_BB = __builtin_popcountll(pos->BBblackpieces & pos->BBbishops);
	int num_BR = __builtin_popcountll(pos->BBblackpieces & pos->BBrooks);
	int num_BQ = __builtin_popcountll(pos->BBblackpieces & pos->BBqueens);
	int num_WP = __builtin_popcountll(pos->BBwhitepieces & pos->BBpawns);
	int num_WN = __builtin_popcountll(pos->BBwhitepieces & pos->BBknights);
	int num_WB = __builtin_popcountll(pos->BBwhitepieces & pos->BBbishops);
	int num_WR = __builtin_popcountll(pos->BBwhitepieces & pos->BBrooks);
	int num_WQ = __builtin_popcountll(pos->BBwhitepieces & pos->BBqueens);
	
	// Piece values
    int white_pieces = num_WP*pieceval('P')   +
                       num_WN*pieceval('N') +
                       num_WB*pieceval('B') +
                       num_WR*pieceval('R')   +
                       num_WQ*pieceval('Q');

    int black_pieces = num_BP*pieceval('p')   +
                       num_BN*pieceval('n') +
                       num_BB*pieceval('b') +
                       num_BR*pieceval('r')   +
                       num_BQ*pieceval('q');

	int material = white_pieces - black_pieces;
	int score = material;
	U64 BBoccupied = (pos->BBwhitepieces | pos->BBblackpieces);
	while (BBoccupied != 0) {
		int square = __builtin_ctzll(BBoccupied);
		char piece = getPiece(pos,square);
		//BBoccupied &= ~(1ULL << square);
		BBoccupied &= BBoccupied - 1;
		int pstscoreO = PSTval(piece,square,'O');
		int pstscoreE = PSTval(piece,square,'E');
		int pval = (pstscoreO + pstscoreE) / 2;
		//if ((piece >= 'a') && (piece <= 'z')) {
		//	pval = -pval;
		//}
		score += pval;
	}
	if (pos->tomove == BLACK) return -score;
	else return score;
	
}
/*
int evalBoard(struct position *pos) {
	assert(pos);
	int score = 0;
	int pval;
	U64 BBoccupied = (pos->BBwhitepieces | pos->BBblackpieces);
	while (BBoccupied != 0) {
		int square = __builtin_ctzll(BBoccupied);
		//BBoccupied &= ~(1ULL << square);
		BBoccupied &= BBoccupied - 1;
		char piece = getPiece(pos,square);
		pval = pieceval(piece);
		if ((piece >= 'a') && (piece <= 'z')) {
			pval = -pval;
		}
		//int pstscore = PSTval(piece,square,'O');
		score += pval;
		//score += pstscore;
		
	}
	if (pos->tomove == BLACK) return -score;
	return score;
}
*/