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
#include "search.h"

static const int safety_table[9][30] =
{
/*0p*/{0,  0,  0,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0},
/*1p*/{0,  0,  0,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0},
/*2p*/{0,  1,  2,  4,   7,  11,  16,  22,  29,  37,  46,  56,  67,  79,  92, 106, 121, 137, 154, 172, 191, 211, 232, 254, 277, 301, 326, 352, 379, 400},
/*3p*/{0,  2,  5,  9,  14,  20,  27,  35,  44,  54,  65,  77,  90, 104, 119, 135, 152, 170, 189, 209, 230, 252, 275, 299, 324, 350, 377, 400, 400, 400},
/*4p*/{0,  4,  8, 13,  19,  26,  34,  43,  53,  64,  76,  89, 103, 118, 134, 151, 169, 188, 208, 229, 251, 274, 298, 323, 349, 376, 400, 400, 400, 400},
/*5p*/{0,  8, 16, 25,  35,  46,  58,  71,  85, 100, 116, 133, 151, 170, 190, 211, 233, 256, 280, 305, 331, 358, 386, 400, 400, 400, 400, 400, 400, 400},
/*6p*/{0, 16, 26, 37,  49,  62,  76,  91, 107, 124, 142, 161, 181, 202, 224, 247, 271, 296, 322, 349, 377, 400, 400, 400, 400, 400, 400, 400, 400, 400},
/*7p*/{0, 32, 44, 57,  71,  86, 102, 119, 137, 156, 176, 197, 219, 242, 266, 291, 317, 344, 372, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400},
/*8p*/{0, 64, 78, 93, 109, 126, 144, 163, 183, 204, 226, 249, 273, 298, 324, 351, 379, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400}
};

// Minor piece attack bonus

int minorAttackBonus_mg(char piece) {
	switch (piece) {
		case 'p':
		case 'P': return 6;
		case 'n':
		case 'N': return 14;
		case 'b':
		case 'B': return 14;
		case 'r':
		case 'R': return 20;
		case 'q':
		case 'Q': return 22;
		case 'k':
		case 'K': return 0;
	}
}
int minorAttackBonus_eg(char piece) {
	switch (piece) {
		case 'p':
		case 'P': return 10;
		case 'n':
		case 'N': return 18;
		case 'b':
		case 'B': return 18;
		case 'r':
		case 'R': return 28;
		case 'q':
		case 'Q': return 30;
		case 'k':
		case 'K': return 0;
	}
}

// Piece mobility

int knightMgMobility[9] = {-15, -5, -1, 2, 5, 7, 9, 11, 13};

// (10 * x Pow 0.5) - 15};

int knightEgMobility[9] = {-30, -10, -2, 4, 10, 14, 18, 22, 26};

// (20 * x Pow 0.5) - 30};

int bishopMgMobility[14] = {-25, -11, -6, -1, 3, 6, 9, 12, 14, 17, 19, 21, 23, 25};

// (14 * x Pow 0.5) - 25};

int bishopEgMobility[14] = {-50, -22, -11, -2, 6, 12, 18, 24, 29, 34, 38, 42, 46, 50};

// (28 * x Pow 0.5) - 50};

int rookMgMobility[15] = {-10, -4, -2, 0, 2, 3, 4, 5, 6, 8, 8, 9, 10, 11, 12};

// (6 * x Pow 0.5) - 10};

int rookEgMobility[15] = {-50, -22, -11, -2, 6, 12, 18, 24, 29, 34, 38, 42, 46, 50, 54};

// (28 * x Pow 0.5) - 50};

int queenMgMobility[28] = {-10, -6, -5, -4, -2, -2, -1, 0, 1, 2, 2, 3, 3, 4, 4, 5, 6, 6, 6,

7, 7, 8, 8, 9, 9, 10, 10, 10};

// (4 * x Pow 0.5) - 10};

int queenEgMobility[28] = {-50, -30, -22, -16, -10, -6, -2, 2, 6, 10, 13, 16, 19, 22, 24,

27, 30, 32, 34, 37, 39, 41, 43, 45, 47, 50, 51, 53}; 

// (20 * x Pow 0.5) - 50};

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
		case 'P': return 110;
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
	
	U64 BBpawns = pos->BBpawns | pos->BBkings;
	
	while (BBpawns != 0) {
		int square = __builtin_ctzll(BBpawns);
		//BBoccupied &= ~(1ULL << square);
		BBpawns &= BBpawns - 1;
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
	/*
	// white pieces attacking black king
	
	int enemykingpos = pos->Bkingpos;
	//U64 BBkingdist1 = BBkingattacks(pos->BBkings & (1ULL << enemykingpos)); // fill 1 square away
	U64 BBkingdist1 = BBkingLookup[enemykingpos];
	U64 BBattackers = BBkingdist1 & (pos->BBwhitepieces & (pos->BBqueens | pos->BBrooks | pos->BBknights | pos->BBpawns));

	openingEval += 20 * __builtin_popcountll(BBattackers);
	endgameEval += 20 * __builtin_popcountll(BBattackers);
	U64 BBkingdist2 = BBkingfillLookup2[enemykingpos]; // fill 2 squares away
	BBkingdist2 = BBkingdist2 & ~(BBkingdist1);
	BBattackers = BBkingdist2 & (pos->BBwhitepieces & (pos->BBrooks | pos->BBqueens | pos->BBknights | pos->BBpawns));

	endgameEval += 40 * __builtin_popcountll(BBattackers);
	U64 BBkingdist3 = BBkingfillLookup3[enemykingpos]; // fill 3 squares away
	BBkingdist3 = BBkingdist3 & ~(BBkingdist2);
	BBattackers = BBkingdist3 & (pos->BBwhitepieces & (pos->BBrooks | pos->BBqueens | pos->BBknights | pos->BBpawns));

	openingEval += 10 * __builtin_popcountll(BBattackers);
	endgameEval += 10 * __builtin_popcountll(BBattackers);
	U64 BBkingdist4 = BBkingfillLookup4[enemykingpos]; // fill 4 squares away
	BBkingdist4 = BBkingdist4 & ~(BBkingdist3);
	BBattackers = BBkingdist4 & (pos->BBwhitepieces & (pos->BBrooks | pos->BBqueens | pos->BBknights | pos->BBpawns));


	openingEval += 10 * __builtin_popcountll(BBattackers);
	endgameEval += 10 * __builtin_popcountll(BBattackers);
	// black pieces attacking white king
	enemykingpos = pos->Wkingpos;
	//BBkingdist1 = BBkingattacks(pos->BBkings & (1ULL << enemykingpos)); // fill 1 square away
	BBkingdist1 = BBkingLookup[enemykingpos];
	BBattackers = BBkingdist1 & (pos->BBblackpieces & (pos->BBqueens | pos->BBrooks | pos->BBknights | pos->BBpawns));

	openingEval -= 20 * __builtin_popcountll(BBattackers);
	endgameEval -= 20 * __builtin_popcountll(BBattackers);
	
	BBkingdist2 = BBkingfillLookup2[enemykingpos]; // fill 2 squares away
	BBkingdist2 = BBkingdist2 & ~(BBkingdist1);
	BBattackers = BBkingdist2 & (pos->BBblackpieces & (pos->BBrooks | pos->BBqueens | pos->BBknights | pos->BBpawns));

	endgameEval -= 40 * __builtin_popcountll(BBattackers);
	BBkingdist3 = BBkingfillLookup3[enemykingpos]; // fill 3 squares away
	BBkingdist3 = BBkingdist3 & ~(BBkingdist2);
	BBattackers = BBkingdist3 & (pos->BBblackpieces & (pos->BBrooks | pos->BBqueens | pos->BBknights | pos->BBpawns));

	openingEval -= 10 * __builtin_popcountll(BBattackers);
	endgameEval -= 10 * __builtin_popcountll(BBattackers);
	
	BBkingdist4 = BBkingfillLookup4[enemykingpos]; // fill 4 squares away
	BBkingdist4 = BBkingdist4 & ~(BBkingdist3);
	BBattackers = BBkingdist4 & (pos->BBblackpieces & (pos->BBrooks | pos->BBqueens | pos->BBknights | pos->BBpawns));

	openingEval -= 10 * __builtin_popcountll(BBattackers);
	endgameEval -= 10 * __builtin_popcountll(BBattackers);
	*/
	
	
	
	
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
		/*
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
		 */
		U64 BBpiece = (1ULL << square);
		int startrank = getrank(square);
		U64 BBenemypawns = BBpasserLookup[WHITE][square] & (pos->BBblackpieces & pos->BBpawns);
		if (BBenemypawns == 0) {
			// pawn is passed
			
			BBwhitePP |= square; // add square to bb of white passed pawns
			int bonus = WpassedRankBonus[startrank];
			openingEval += 0.5 * bonus;
			endgameEval += 1 * bonus;
			// give a bonus/penalty for opponent/friendly king distances to the passed pawn
			
			
			int oppkingdistx = abs(getfile(pos->Bkingpos) - getfile(square));
			int oppkingdisty = abs(getrank(pos->Bkingpos) - getrank(square));
			int oppkingdist = max(oppkingdistx, oppkingdisty);
			
			int mykingdistx = abs(getfile(pos->Wkingpos) - getfile(square));
			int mykingdisty = abs(getrank(pos->Wkingpos) - getrank(square));
			int mykingdist = max(mykingdistx, mykingdisty);
			
			endgameEval += 14 * oppkingdist;
			endgameEval += -11 * mykingdist;
			/*
			// get squares front and behind pawn
			
			U64 BBnorthsquares = 0, BBsouthsquares = 0;
			U64 BBcursquare = BBpiece;
			int currank = startrank;
			while (currank <= 7) {
				// squares north
				currank++;
				BBcursquare = northOne(BBcursquare);
				BBnorthsquares |= BBcursquare;
			}
			currank = startrank;
			BBcursquare = BBpiece;
			while (currank >= 0) {
				// squares south
				currank--;
				BBcursquare = southOne(BBcursquare);
				BBsouthsquares |= BBcursquare;
			}
			// white pawn
			// friendly rooks get a bonus for being south of the pawn
			
			U64 BBwhiterooks = pos->BBwhitepieces & pos->BBrooks;
			U64 BBblackrooks = pos->BBblackpieces & pos->BBrooks;
			
			if (BBwhiterooks & BBsouthsquares) {
				openingEval += 30;
				endgameEval += 30;
			}
			// penalty for friendly rook being north of pawn
			if (BBwhiterooks & BBnorthsquares) {
				openingEval -= 30;
				endgameEval -= 30;
			}
			// penalty for opponent pawn being south of pawn
			if (BBblackrooks & BBsouthsquares) {
				openingEval -= 30;
				endgameEval -= 30;
			}
			// bonus for opponent pawn being north of pawn
			if (BBblackrooks & BBsouthsquares) {
				openingEval += 30;
				endgameEval += 30;
			}
			 */
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
		/*
		U64 BBpiece = (1ULL << square);
		U64 BBmidsquare = BBpiece;
		U64 BBchecksquares = 0ULL;
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
		 */
		U64 BBpiece = (1ULL << square);
		int startrank = getrank(square);
		U64 BBenemypawns = (BBpasserLookup[BLACK][square] & (pos->BBwhitepieces & pos->BBpawns));
		if (BBenemypawns == 0) {
			BBblackPP |= square;
			int bonus = BpassedRankBonus[startrank];
			openingEval -= 0.5 * bonus;
			endgameEval -= 1 * bonus;
			
			// give a bonus/penalty for opponent/friendly king distances to the passed pawn
			
			
			int oppkingdistx = abs(getfile(pos->Wkingpos) - getfile(square));
			int oppkingdisty = abs(getrank(pos->Wkingpos) - getrank(square));
			int oppkingdist = max(oppkingdistx, oppkingdisty);
			
			int mykingdistx = abs(getfile(pos->Bkingpos) - getfile(square));
			int mykingdisty = abs(getrank(pos->Bkingpos) - getrank(square));
			int mykingdist = max(mykingdistx, mykingdisty);
			
			endgameEval -= 14 * oppkingdist;
			endgameEval -= -11 * mykingdist;
			
			// bonus/penalty for rooks behing behind/in front of pawn
			
			// get squares front and behind pawn
			/*
			U64 BBnorthsquares = 0, BBsouthsquares = 0;
			U64 BBcursquare = BBpiece;
			int currank = startrank;
			while (currank <= 7) {
				// squares north
				currank++;
				BBcursquare = northOne(BBcursquare);
				BBnorthsquares |= BBcursquare;
			}
			currank = startrank;
			BBcursquare = BBpiece;
			while (currank >= 0) {
				// squares south
				currank--;
				BBcursquare = southOne(BBcursquare);
				BBsouthsquares |= BBcursquare;
			}
			// white pawn
			// friendly rooks get a penalty for being south of the pawn
			
			U64 BBwhiterooks = pos->BBwhitepieces & pos->BBrooks;
			U64 BBblackrooks = pos->BBblackpieces & pos->BBrooks;
			
			if (BBblackrooks & BBsouthsquares) {
				openingEval += 30;
				endgameEval += 30;
			}
			// bonus for friendly rook being north of pawn
			if (BBblackrooks & BBnorthsquares) {
				openingEval -= 30;
				endgameEval -= 30;
			}
			// bonus for opponent pawn being south of pawn
			if (BBwhiterooks & BBsouthsquares) {
				openingEval -= 30;
				endgameEval -= 30;
			}
			// penalty for opponent pawn being north of pawn
			if (BBwhiterooks & BBsouthsquares) {
				openingEval += 30;
				endgameEval += 30;
			}
			*/
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
	
	// bonus for pawns attacking enemy pieces
	
	// white
	
	/*
	 
	BBwhitepawns = pos->BBpawns & pos->BBwhitepieces;
	U64 BBpawnattacks = BBpawnattacksW(BBwhitepawns);
	U64 BBBpieces = pos->BBblackpieces & (pos->BBknights | pos->BBbishops | pos->BBrooks | pos->BBqueens);
	U64 BBBattackedpieces = BBpawnattacks & BBBpieces;
	//dspBB(BBpawnattacks);
	int attackcount =  __builtin_popcountll(BBBattackedpieces);
	if (attackcount <= 1) {
		openingEval += 6 * attackcount;
		endgameEval += 6 * attackcount;
	}
	else {
		openingEval += 30 * attackcount;
		endgameEval += 30 * attackcount;
	}
	
	// black
	
	BBblackpawns = pos->BBpawns & pos->BBblackpieces;
	BBpawnattacks = BBpawnattacksB(BBblackpawns);
	U64 BBWpieces = pos->BBwhitepieces & (pos->BBknights | pos->BBbishops | pos->BBrooks | pos->BBqueens);
	U64 BBWattackedpieces = BBpawnattacks & BBWpieces;
	attackcount =  __builtin_popcountll(BBWattackedpieces);
	if (attackcount <= 1) {
		openingEval -= 6 * attackcount;
		endgameEval -= 6 * attackcount;
	}
	else {
		openingEval -= 30 * attackcount;
		endgameEval -= 30 * attackcount;
	}
	*/
	
	// give a bonus for free passed pawns
	// pawns on the 6th or 7th rank that can advance without losing material
	
	int freepawnrankbonus[8] = {0, 0, 10, 20, 40, 60, 80, 120 };
	
	U64 BBwhitePPon4to7rank = (BBwhitePP & (BBrank4 | BBrank5 | BBrank6 | BBrank7));
	while (BBwhitePPon4to7rank) {
		int square = __builtin_ctzll(BBwhitePPon4to7rank);
		BBwhitePPon4to7rank &= ~(1ULL << square);
		int currank = getrank(square);
		int freepath = 1;
		int lastsquare = square;
		while (currank <= 7) {
			currank++;
			int advsquare = fileranktosquareidx(getfile(square), currank);
			int SEEvalue = SEEcapture(pos, lastsquare, advsquare, WHITE);
			if (SEEvalue <= 0) {
				freepath = 0;
				break;
			}
			lastsquare = advsquare;
		}
		if (freepath) {
			// pawn has a free path to advance to promotion
			openingEval += freepawnrankbonus[getrank(square)];
			endgameEval += freepawnrankbonus[getrank(square)];
		}
	}
	
	// black
	
	U64 BBblackPPon2to5rank = (BBblackPP & (BBrank2 | BBrank3 | BBrank4 | BBrank5));
	while (BBblackPPon2to5rank) {
		int square = __builtin_ctzll(BBblackPPon2to5rank);
		BBblackPPon2to5rank &= ~(1ULL << square);
		int currank = getrank(square);
		int freepath = 1;
		int lastsquare = square;
		while (currank >= 0) {
			currank--;
			int advsquare = fileranktosquareidx(getfile(square), currank);
			int SEEvalue = SEEcapture(pos, lastsquare, advsquare, BLACK);
			if (SEEvalue <= 0) {
				freepath = 0;
				break;
			}
			lastsquare = advsquare;
		}
		if (freepath) {
			// pawn has a free path to advance to promotion
			openingEval -= freepawnrankbonus[7 - getrank(square)];
			endgameEval -= freepawnrankbonus[7 - getrank(square)];
		}
	}
	
	
	// bonus for connected passed pawns
	/*
	int lastwhitepawn = 0;
	int lastblackpawn = 0;
	for (int i = 0;i < 8;i++) {
		U64 BBfilemask = BBfileA << i;
		
		// white
		if (BBwhitePP & BBfilemask) {
			// white passed pawn on file
			if (lastwhitepawn) {
				// connected to pawn to the left
				openingEval += 10 * lastwhitepawn;
				endgameEval += 20 * lastwhitepawn;
			}
			lastwhitepawn++;
		}
		else lastwhitepawn = 0;
		
		// black
		if (BBblackPP & BBfilemask) {
			// white passed pawn on file
			if (lastblackpawn) {
				// connected to pawn to the left
				openingEval -= 10 * lastblackpawn;
				endgameEval -= 20 * lastblackpawn;
			}
			lastblackpawn++;
		}
		else lastblackpawn = 0;
	}
	 */
	
	// Candidate passed pawns
	
	// Toga's method
	
	// white
	/*
	BBwhitepawns = pos->BBwhitepieces & pos->BBpawns;
	BBblackpawns = pos->BBblackpieces & pos->BBpawns;
	U64 BBwhitepawnscopy = BBwhitepawns;
	U64 BBblackpawnscopy = BBblackpawns;
	while (BBwhitepawnscopy) {
		int square = __builtin_ctzll(BBwhitepawnscopy);
		BBwhitepawnscopy &= ~(1ULL << square);
		//if (getrank(square) <= 2) continue;
		if (BBwhitePP & 1ULL << square) continue;
		int sentries = 0;
		int helpers = 0;
		U64 BBsentries = 0;
		U64 BBfront = 0;
		int rank = getrank(square);
		int file = getfile(square);
		int isopen = 1;
		while (rank < 6) {
			rank++;
			int midsquare = fileranktosquareidx(file,rank);
			BBfront |= 1ULL << midsquare;
			BBsentries |= westOne(midsquare) | eastOne(midsquare);
		}
		if (BBfront & pos->BBpawns) continue; // not open
		BBsentries &= pos->BBblackpieces & pos->BBpawns;
		sentries = __builtin_popcountll(BBsentries);
	
		U64 BBhelpers = westOne(square) | eastOne(square);
		rank = getrank(square);
		while (rank > 1) {
			rank--;
			int midsquare = fileranktosquareidx(file,rank);
			BBhelpers |= westOne(midsquare) | eastOne(midsquare);
		}
		BBhelpers &= pos->BBwhitepieces & pos->BBpawns;
		helpers = __builtin_popcountll(BBhelpers);
		if (sentries > helpers) continue;
		
		int hostile = 0;
		int friendly = 0;
		
		U64 BBhostile = BBpawnattacksW(1ULL << square) & pos->BBblackpieces & pos->BBpawns;
		hostile = __builtin_popcountll(BBhostile);
		U64 BBfriendly = BBpawnattacksB(1ULL << square) & pos->BBwhitepieces & pos->BBpawns;
		friendly = __builtin_popcountll(BBfriendly);
		if (hostile > friendly) continue;
		
		// pawn is a candidate
		//printf("candidate! %d\n", square);
		//dspBoard(pos);
		double rankbonus[8] = { 0, 0, 0, 0.1, 0.3, 0.6, 1.0, 0 };
		openingEval += (5 + 50 * rankbonus[getrank(square)]);
		endgameEval += (10 + 100 * rankbonus[getrank(square)]);
		
	}
	
	// black
	
	while (BBblackpawnscopy) {
		int square = __builtin_ctzll(BBblackpawnscopy);
		BBblackpawnscopy &= ~(1ULL << square);
		//if (getrank(square) >= 5) continue;
		if (BBblackPP & 1ULL << square) continue;
		int sentries = 0;
		int helpers = 0;
		U64 BBsentries = 0;
		int rank = getrank(square);
		int file = getfile(square);
		while (rank > 1) {
			rank--;
			int midsquare = fileranktosquareidx(file,rank);
			BBsentries |= westOne(midsquare) | eastOne(midsquare);
		}
		BBsentries &= pos->BBwhitepieces & pos->BBpawns;
		sentries = __builtin_popcountll(BBsentries);
		U64 BBfront;
		U64 BBhelpers = westOne(square) | eastOne(square);
		rank = getrank(square);
		while (rank < 6) {
			rank++;
			int midsquare = fileranktosquareidx(file,rank);
			BBfront |= 1ULL << midsquare;
			BBhelpers |= westOne(midsquare) | eastOne(midsquare);
		}
		if (BBfront & pos->BBpawns) continue; // not open
		BBhelpers &= pos->BBblackpieces & pos->BBpawns;
		helpers = __builtin_popcountll(BBhelpers);
		if (sentries > helpers) continue;
		
		int hostile = 0;
		int friendly = 0;
		
		U64 BBhostile = BBpawnattacksB(1ULL << square) & pos->BBwhitepieces & pos->BBpawns;
		hostile = __builtin_popcountll(BBhostile);
		U64 BBfriendly = BBpawnattacksW(1ULL << square) & pos->BBblackpieces & pos->BBpawns;
		friendly = __builtin_popcountll(BBfriendly);
		if (hostile > friendly) continue;
		
		// pawn is a candidate
		//printf("candidate!\n");
		double rankbonus[8] = { 0, 0, 0, 0.1, 0.3, 0.6, 1.0, 0 };
		openingEval -= (5 + 50 * rankbonus[7 - getrank(square)]);
		endgameEval -= (10 + 100 * rankbonus[7 - getrank(square)]);
	}
	 */
	// white
	/*
	BBwhitepawns = pos->BBwhitepieces & pos->BBpawns;
	BBblackpawns = pos->BBblackpieces & pos->BBpawns;
	U64 BBwhitepawnscopy = BBwhitepawns;
	U64 BBblackpawnscopy = BBblackpawns;
	while (BBwhitepawnscopy) {
		int square = __builtin_ctzll(BBwhitepawnscopy);
		BBwhitepawnscopy &= ~(1ULL << square);
		int advsquare = fileranktosquareidx(getfile(square), getrank(square) + 1);
		U64 BBattacks = BBpawnattacksW(1ULL << advsquare);
		int sentries = __builtin_popcountll(BBattacks & BBblackpawns);
		if (sentries > 0) {
			U64 BBmyattacks = BBpawnattacksB(1ULL << advsquare);
			int helpers = __builtin_popcountll(BBmyattacks & BBwhitepawns);
			if (helpers >= sentries) {
				openingEval += 50;
				endgameEval += 50;
			}
		}
	}
	
	// black
	while (BBblackpawnscopy) {
		int square = __builtin_ctzll(BBblackpawnscopy);
		BBblackpawnscopy &= ~(1ULL << square);
		int advsquare = fileranktosquareidx(getfile(square), getrank(square) - 1);
		U64 BBattacks = BBpawnattacksB(1ULL << advsquare);
		int sentries = __builtin_popcountll(BBattacks & BBwhitepawns);
		if (sentries > 0) {
			U64 BBmyattacks = BBpawnattacksW(1ULL << advsquare);
			int helpers = __builtin_popcountll(BBmyattacks & BBblackpawns);
			if (helpers >= sentries) {
				openingEval -= 50;
				endgameEval -= 50;
			}
		}
	}
	*/
	// white
	/*
	BBwhitepawns = pos->BBwhitepieces & pos->BBpawns;
	while (BBwhitepawns) {
		int square = __builtin_ctzll(BBwhitepawns);
		BBwhitepawns &= ~(1ULL << square);
		int file = getfile(square);
		U64 BBfilemask = BBfileA << file;
		U64 BBBpawnsonfile = (pos->BBblackpieces & pos->BBpawns & BBfilemask);
		if (!BBBpawnsonfile) {
			// semi open file, could be a candidate
			//BBfilewest = westOne(BBfilemask);
			//BBfileeast = eastOne(BBfilemask);
			int cursquare = square;
			U64 BBcursquare = 1ULL << square;
			// get sentries and helpers
			int sentries = 0;
			int helpers = 0;
			for (int rank = getrank(square);rank < 6;rank++) {
				cursquare = fileranktosquareidx(file, rank);
				BBcursquare = 1ULL << cursquare;
				U64 BBnw = noWeOne(BBcursquare);
				U64 BBne = noEaOne(BBcursquare);
				if (pos->BBblackpieces & pos->BBpawns & BBnw) sentries++;
				if (pos->BBblackpieces & pos->BBpawns & BBne) sentries++;
				if (pos->BBwhitepieces & pos->BBpawns & BBnw) helpers++;
				if (pos->BBwhitepieces & pos->BBpawns & BBne) helpers++;
			}
			if (helpers > sentries) {
				openingEval += 10;
				endgameEval += 30;
			}
		}
	}
	
	// black
	
	BBblackpawns = pos->BBblackpieces & pos->BBpawns;
	while (BBblackpawns) {
		int square = __builtin_ctzll(BBblackpawns);
		BBblackpawns &= ~(1ULL << square);
		int file = getfile(square);
		U64 BBfilemask = BBfileA << file;
		U64 BBWpawnsonfile = (pos->BBwhitepieces & pos->BBpawns & BBfilemask);
		if (!BBWpawnsonfile) {
			// semi open file, could be a candidate
			//BBfilewest = westOne(BBfilemask);
			//BBfileeast = eastOne(BBfilemask);
			int cursquare = square;
			U64 BBcursquare = 1ULL << square;
			// get sentries and helpers
			int sentries = 0;
			int helpers = 0;
			for (int rank = getrank(square);rank > 1;rank--) {
				cursquare = fileranktosquareidx(file, rank);
				BBcursquare = 1ULL << cursquare;
				U64 BBnw = soWeOne(BBcursquare);
				U64 BBne = soEaOne(BBcursquare);
				if (pos->BBwhitepieces & pos->BBpawns & BBnw) sentries++;
				if (pos->BBwhitepieces & pos->BBpawns & BBne) sentries++;
				if (pos->BBblackpieces & pos->BBpawns & BBnw) helpers++;
				if (pos->BBblackpieces & pos->BBpawns & BBne) helpers++;
			}
			if (helpers > sentries) {
				openingEval -= 10;
				endgameEval -= 30;
			}
		}
	}
	*/
	
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
	
	// bonus for rooks/queens being on 7th rank
	
	U64 BBwhitequeens = pos->BBwhitepieces & pos->BBqueens;
	U64 BBblackqueens = pos->BBblackpieces & pos->BBqueens;
	
	U64 BBwhiterooks = pos->BBwhitepieces & pos->BBrooks;
	U64 BBblackrooks = pos->BBblackpieces & pos->BBrooks;
	
	while (BBwhitequeens) {
		int square = __builtin_ctzll(BBwhitequeens);
		BBwhitequeens &= BBwhitequeens - 1;
		if (getrank(square) != 6) continue;
		// queen on 7th rank
		U64 BBhostilepawns = BBrank7 & pos->BBblackpieces & pos->BBpawns;
		if (!BBhostilepawns && getrank(pos->Bkingpos) != 7) continue;
		// either hostile pawns on 7th rank or king is on 8th rank
		
		openingEval += 10;
		endgameEval += 20;
	}
	
	while (BBblackqueens) {
		int square = __builtin_ctzll(BBblackqueens);
		BBblackqueens &= BBblackqueens - 1;
		if (getrank(square) != 1) continue;
		// queen on 7th rank
		U64 BBhostilepawns = BBrank2 & pos->BBwhitepieces & pos->BBpawns;
		if (!BBhostilepawns && getrank(pos->Wkingpos) != 0) continue;
		// either hostile pawns on 7th rank or king is on 8th rank
		
		openingEval -= 10;
		endgameEval -= 20;
	}
	
	while (BBwhiterooks) {
		int square = __builtin_ctzll(BBwhiterooks);
		BBwhiterooks &= BBwhiterooks - 1;
		if (getrank(square) != 6) continue;
		// rook on 7th rank
		U64 BBhostilepawns = BBrank7 & pos->BBblackpieces & pos->BBpawns;
		if (!BBhostilepawns && getrank(pos->Bkingpos) != 7) continue;
		// either hostile pawns on 7th rank or king is on 8th rank
		
		openingEval += 20;
		endgameEval += 40;
	}
	
	while (BBblackrooks) {
		int square = __builtin_ctzll(BBblackrooks);
		BBblackrooks &= BBblackrooks - 1;
		if (getrank(square) != 1) continue;
		// rook on 7th rank
		U64 BBhostilepawns = BBrank2 & pos->BBwhitepieces & pos->BBpawns;
		if (!BBhostilepawns && getrank(pos->Wkingpos) != 0) continue;
		// either hostile pawns on 7th rank or king is on 8th rank
		
		openingEval -= 20;
		endgameEval -= 40;
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
	
	//U64 BBWlastpawnsonfile = 0ULL;
	//U64 BBBlastpawnsonfile = 0ULL;
	
	//int Wislands = 0;
	//int Bislands = 0;
	
	for (int i = 0;i < 8;i++) {
		// doubled pawns
		// white pawns
		U64 BBfilemask = BBfileA << i;
		
		//U64 BBallpawnsonfile = BBfilemask & pos->BBpawns;
		//U64 BBallrooksonfile = BBfilemask & pos->BBrooks;
		//if (!BBallpawnsonfile && !BBallrooksonfile) continue;
		
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
		
		// count pawn islands
		/*
		if (BBWpawnsonfile) {
			if (BBWlastpawnsonfile == 0) {
				// start of new island
				Wislands++;
			}
		}
		
		if (BBBpawnsonfile) {
			if (BBBlastpawnsonfile == 0) {
				// start of new island
				Bislands++;
			}
		}
		
		BBWlastpawnsonfile = BBWpawnsonfile;
		BBBlastpawnsonfile = BBBpawnsonfile;
		*/
				
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
		
		//if (!BBallrooksonfile) continue;
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
			U64 BBPPonfile = (BBwhitePP & BBfilemask);
			if (BBPPonfile) {
				int square = __builtin_ctzll(BBPPonfile);
				int rooksquare = __builtin_ctzll(BBWrooksonfile);
				int isclear = 1;
				if (getrank(square) > getrank(rooksquare)) {
					for (int i = getrank(rooksquare) + 1;i < getrank(square);i--) {
						if (getPiece(pos,fileranktosquareidx(getfile(square),i)) != '0') {
							isclear = 0;
							break;
						}
					}
					if (isclear) {
						openingEval += 10;
						endgameEval += 20;
					}
				}
			}
			 */
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
			U64 BBPPonfile = (BBblackPP & BBfilemask);
			if (BBPPonfile) {
				int square = __builtin_ctzll(BBPPonfile);
				int rooksquare = __builtin_ctzll(BBBrooksonfile);
				int isclear = 1;
				if (getrank(square) < getrank(rooksquare)) {
					for (int i = getrank(rooksquare) - 1;i > getrank(square);i--) {
						if (getPiece(pos,fileranktosquareidx(getfile(square),i)) != '0') {
							isclear = 0;
							break;
						}
					}
					if (isclear) {
						openingEval -= 10;
						endgameEval -= 20;
					}
				}
			}
			 */
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
	
	// give a penalty for 2+ pawn islands
	/*
	if (Wislands > 0) {
		openingEval -= (Wislands - 1) * 35;
		endgameEval -= (Wislands - 1) * 15;
	}
	if (Bislands > 0) {
		openingEval += (Bislands - 1) * 35;
		endgameEval += (Bislands - 1) * 15;
	}
	*/
	
	/*
	// pawn storms
	
	int stormDistBonus[4] = { 0, 20, 10, 5 };
	
	// white (white king being stormed by black pawns)
	
	U64 BBA1quarter = (BBrank1 | BBrank2 | BBrank3 | BBrank4) & (BBfileA | BBfileB | BBfileC | BBfileD);
	U64 BBH1quarter = (BBrank1 | BBrank2 | BBrank3 | BBrank4) & (BBfileE | BBfileF | BBfileG | BBfileH);
	U64 BBkingquarter = 0;
	if (BBA1quarter & (1ULL << pos->Wkingpos)) BBkingquarter = BBA1quarter;
	else if (BBH1quarter & (1ULL << pos->Wkingpos)) BBkingquarter = BBH1quarter;
	
	if (BBkingquarter & (1ULL << pos->Wkingpos) & (BBrank1 | BBrank2) & ~(BBfileE | BBfileD))  {
		U64 BBstormpawns = pos->BBpawns & pos->BBblackpieces & BBkingquarter;
		while (BBstormpawns) {
			int square = __builtin_ctzll(BBstormpawns);
			BBstormpawns &= BBstormpawns - 1;
			int distx = abs(getfile(pos->Wkingpos) - getfile(square));
			int disty = abs(getrank(pos->Wkingpos) - getrank(square));
			int dist = max(distx, disty);
			openingEval -= stormDistBonus[dist];
			endgameEval -= stormDistBonus[dist];
		}
	}
	
	// black (black king being stormed by white pawns)
	
	U64 BBA8quarter = (BBrank5 | BBrank6 | BBrank7 | BBrank8) & (BBfileA | BBfileB | BBfileC | BBfileD);
	U64 BBH8quarter = (BBrank5 | BBrank6 | BBrank7 | BBrank8) & (BBfileE | BBfileF | BBfileG | BBfileH);
	BBkingquarter = 0;
	if (BBA8quarter & (1ULL << pos->Bkingpos)) BBkingquarter = BBA8quarter;
	else if (BBH8quarter & (1ULL << pos->Bkingpos)) BBkingquarter = BBH8quarter;
	
	if (BBkingquarter & (1ULL << pos->Bkingpos) & (BBrank7 | BBrank8) & ~(BBfileE | BBfileD)) {
		U64 BBstormpawns = pos->BBpawns & pos->BBwhitepieces & BBkingquarter;
		while (BBstormpawns) {
			int square = __builtin_ctzll(BBstormpawns);
			BBstormpawns &= BBstormpawns - 1;
			int distx = abs(getfile(pos->Bkingpos) - getfile(square));
			int disty = abs(getrank(pos->Bkingpos) - getrank(square));
			int dist = max(distx, disty);
			openingEval += stormDistBonus[dist];
			endgameEval += stormDistBonus[dist];
		}
	}
	*/
	// pawn shield
	
	// white pawn shield
	
	int Wkingpos = pos->Wkingpos;
	//BBkingdist1 = BBkingattacks((1ULL << Wkingpos)); // fill 1 square away
	//BBkingdist2 = BBkingattacks((1ULL << Wkingpos << 8)); // fill 3 squares nw, n, ne of king zone
	/*
	U64 BBpawnshield = noWeOne(1ULL << Wkingpos) | northOne(1ULL << Wkingpos) | noEaOne(1ULL << Wkingpos);
	BBpawnshield |= northOne(BBpawnshield);
	 */
	U64 BBpawnshield = BBpawnshieldLookup[WHITE][Wkingpos];
	BBpawnshield &= (pos->BBwhitepieces & pos->BBpawns);
	openingEval += 30 * __builtin_popcountll(BBpawnshield);
	
	// black pawn shield
	
	int Bkingpos = pos->Bkingpos;
	//BBkingdist1 = BBkingattacks((1ULL << Bkingpos)); // fill 1 square away
	//BBkingdist2 = BBkingattacks((1ULL << Bkingpos >> 8)); // fill 3 squares sw, s, se of king zone
	//BBpawnshield = BBkingdist2 & (pos->BBblackpieces & pos->BBpawns);
	/*
	BBpawnshield = soWeOne(1ULL << Bkingpos) | southOne(1ULL << Bkingpos) | soEaOne(1ULL << Bkingpos);
	BBpawnshield |= southOne(BBpawnshield);
	 */
	BBpawnshield = BBpawnshieldLookup[BLACK][Bkingpos];
	BBpawnshield &= (pos->BBblackpieces & pos->BBpawns);
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
	
	// trapped pieces
	
	// white
	//U64 BBwhitebishops = pos->BBwhitepieces & pos->BBbishops;
	
	/*
	U64 BBwhitematpieces = pos->BBwhitepieces & (pos->BBbishops | pos->BBknights | pos->BBrooks | pos->BBqueens);
	U64 BBmoves;
	while (BBwhitematpieces) {
		int square = __builtin_ctzll(BBwhitematpieces);
		BBwhitematpieces &= BBwhitematpieces - 1;
		char piece = getPiece(pos, square);
		int onedge = 0;
		//if (square == A1 || square == A2 || square == B1 || square == B2
		//	|| square == A8 || square == A7 || square == B8 || square == B7
		//	|| square == H1 || square == H2 || square == G1 || square == G2
		//	|| square == H8 || square == H7 || square == G8 || square == G7) {
		//	onedge = 1;
		//}
		//if ((1ULL << square) & ~BBcentre) onedge = 1;
		//if (getfile(square) == 0 || getfile(square) == 7) onedge = 1;
		if ((1ULL << square) & (BBrank6 | BBrank7 | BBrank8)  & ~BBfileD & ~BBfileE) onedge = 1;
		if (!onedge) continue;

		if (piece != 'N' && piece != 'B') continue;
		//if (pos->tomove == WHITE) {
		//	struct move prevmove = movestack[movestackend - 1];
		//	if (prevmove.cappiece != '0' && pieceval(prevmove.cappiece) > pieceval(piece)) continue;
		//}
		//if (!isAttacked(pos, square, BLACK)) continue;
		//if (piece == 'R') continue;
		if (piece == 'N') {
			BBmoves = BBknightattacks(1ULL << square) & ~(pos->BBwhitepieces);
		}
		if (piece == 'B') {
			BBmoves = Bmagic(square, pos->BBwhitepieces | pos->BBblackpieces) & ~(pos->BBwhitepieces);
		}
		if (piece == 'R') {
			BBmoves = Rmagic(square, pos->BBwhitepieces | pos->BBblackpieces) & ~(pos->BBwhitepieces);
		}
		if (piece == 'Q') {
			BBmoves = (Rmagic(square, pos->BBwhitepieces | pos->BBblackpieces)
						| Bmagic(square, pos->BBwhitepieces | pos->BBblackpieces)) & ~(pos->BBwhitepieces);
		}
		//if (__builtin_popcountll(BBmoves) == 0 || __builtin_popcountll(BBmoves) > 4) continue;
		int noescape = 1;
		while (BBmoves) {
			int movesquare = __builtin_ctzll(BBmoves);
			BBmoves &= BBmoves - 1;
			int SEEvalue = SEEcapture(pos, square, movesquare, WHITE);
			//printf("SEEvalue %d %d\n",movesquare,SEEvalue);
			if (SEEvalue >= 0) {
				noescape = 0;
				break;
			}
		}
		if (noescape) {
			
			//printf("trapped pieces\n");
			//dspBoard(pos);
			// piece is trapped
			
			//printf("trapped piece %d\n", square);
			//dspBoard(pos);
			
			//printf("trapped piece %c %d\n", piece, square);
			//dspBoard(pos);
			openingEval -= 100;
			endgameEval -= 100;
		}
	}

	// black
	U64 BBblackmatpieces = pos->BBblackpieces & (pos->BBbishops | pos->BBknights | pos->BBrooks | pos->BBqueens);
	while (BBblackmatpieces) {
		int square = __builtin_ctzll(BBblackmatpieces);
		BBblackmatpieces &= BBblackmatpieces - 1;
		char piece = getPiece(pos, square);
		int onedge = 0;
		//if (square == A1 || square == A2 || square == B1 || square == B2
		//	|| square == A8 || square == A7 || square == B8 || square == B7
		//	|| square == H1 || square == H2 || square == G1 || square == G2
		//	|| square == H8 || square == H7 || square == G8 || square == G7) {
		//	onedge = 1;
		//}
		//if ((1ULL << square) & ~BBcentre) onedge = 1;
		//if (getfile(square) == 0 || getfile(square) == 7) onedge = 1;
		if ((1ULL << square) & (BBrank1 | BBrank2 | BBrank3) & ~BBfileD & ~BBfileE) onedge = 1;
		if (!onedge) continue;
		if (piece != 'n' && piece != 'b') continue;
		//if (pos->tomove == BLACK) {
		//	struct move prevmove = movestack[movestackend - 1];
		//	if (prevmove.cappiece != '0' && pieceval(prevmove.cappiece) > pieceval(piece)) continue;
		//}
		//if (!isAttacked(pos, square, WHITE)) continue;
		//if (piece == 'r') continue;
		if (piece == 'n') {
			BBmoves = BBknightattacks(1ULL << square) & ~(pos->BBblackpieces);
		}
		if (piece == 'b') {
			BBmoves = Bmagic(square, pos->BBwhitepieces | pos->BBblackpieces) & ~(pos->BBblackpieces);
		}
		if (piece == 'r') {
			BBmoves = Rmagic(square, pos->BBwhitepieces | pos->BBblackpieces) & ~(pos->BBblackpieces);
		}
		if (piece == 'q') {
			BBmoves = (Rmagic(square, pos->BBwhitepieces | pos->BBblackpieces)
						| Bmagic(square, pos->BBwhitepieces | pos->BBblackpieces)) & ~(pos->BBblackpieces);
		}
		//if (__builtin_popcountll(BBmoves) == 0 || __builtin_popcountll(BBmoves) > 4) continue;
		int noescape = 1;
		while (BBmoves) {
			int movesquare = __builtin_ctzll(BBmoves);
			BBmoves &= BBmoves - 1;
			int SEEvalue = SEEcapture(pos, square, movesquare, BLACK);
			if (SEEvalue >= 0) {
				noescape = 0;
				break;
			}
		}
		if (noescape) {
			// piece is trapped
			//printf("trapped piece\n");
			openingEval += 100;
			endgameEval += 100;
		}
	}
	/*
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
	if (num_WB == 1) {
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
			openingEval -= 4 * __builtin_popcountll(BBsamecolpawns);
			endgameEval -= 6 * __builtin_popcountll(BBsamecolpawns);
		}
	}
	
	// black
	
	if (num_BB == 1) {
		U64 BBbishops = pos->BBbishops & pos->BBblackpieces;
		while (BBbishops) {
			int square = __builtin_ctzll(BBbishops);
			BBbishops &= BBbishops - 1;
			int islight = 0;
			if ((1ULL << square) & BBlightsquares) islight = 1;
			else islight = 0;
			U64 BBsamecolpawns;
			if (islight) BBsamecolpawns = pos->BBpawns & pos->BBblackpieces & BBlightsquares;
			else BBsamecolpawns = pos->BBpawns & pos->BBblackpieces & BBdarksquares;
			openingEval += 4 * __builtin_popcountll(BBsamecolpawns);
			endgameEval += 6 * __builtin_popcountll(BBsamecolpawns);
		}
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
	openingEval += 20 * __builtin_popcountll(BBWpiecesincentre);
	endgameEval += 20 * __builtin_popcountll(BBWpiecesincentre);
	
	U64 BBBpiecesincentre = (pos->BBblackpieces & pos->BBpawns & BBcentre);
	openingEval -= 20 * __builtin_popcountll(BBBpiecesincentre);
	endgameEval -= 20 * __builtin_popcountll(BBBpiecesincentre);
	
	// bonus for pawns attacking the centre
	
	U64 BBWattackingcentre = BBpawnattacksW(pos->BBwhitepieces & pos->BBpawns) & BBcentre;
	openingEval += 20 * __builtin_popcountll(BBWattackingcentre);
	endgameEval += 10 * __builtin_popcountll(BBWattackingcentre);
	
	U64 BBBattackingcentre = BBpawnattacksB(pos->BBblackpieces & pos->BBpawns) & BBcentre;
	openingEval -= 20 * __builtin_popcountll(BBBattackingcentre);
	endgameEval -= 10 * __builtin_popcountll(BBBattackingcentre);
	
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
	
	// penalty for pieces being attacked by enemy pawns
	
	// white
	/*
	U64 BBBpawns = pos->BBblackpieces & pos->BBpawns;
	U64 BBBpawnattacks = soEaOne(BBBpawns) | soWeOne(BBBpawns);
	U64 BBWpiecesattacked = (pos->BBwhitepieces & ~pos->BBpawns) & BBBpawnattacks;
	openingEval -= 10 * __builtin_popcountll(BBWpiecesattacked);
	endgameEval -= 10 * __builtin_popcountll(BBWpiecesattacked);
	// black
	
	U64 BBWpawns = pos->BBwhitepieces & pos->BBpawns;
	U64 BBWpawnattacks = noEaOne(BBWpawns) | noWeOne(BBWpawns);
	U64 BBBpiecesattacked = (pos->BBblackpieces & ~pos->BBpawns) & BBWpawnattacks;
	openingEval += 10 * __builtin_popcountll(BBBpiecesattacked);
	endgameEval += 10 * __builtin_popcountll(BBBpiecesattacked);
	*/
	
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
	BBblackpawns = (pos->BBblackpieces & pos->BBpawns);
	
	U64 BBwhiteknights = (pos->BBwhitepieces & pos->BBknights);
	while (BBwhiteknights) {
		int square = __builtin_ctzll(BBwhiteknights);
		//BBwhiteknights &= ~(1ULL << square);
		BBwhiteknights &= BBwhiteknights - 1;
		if ((BBpawnWestAttacksB(1ULL << square) & BBwhitepawns) || (BBpawnEastAttacksB(1ULL << square) & BBwhitepawns)) {
			openingEval += 20;
			endgameEval += 20;
			/*
			int opppawns = 0;
			U64 BBsq = 1ULL << square;
			for (int rank = getrank(square);rank < 8;rank++) {
				U64 BBnewsquare = northOne(BBsq);
				U64 BBnwsq = noWeOne(BBsq);
				U64 BBnesq = noEaOne(BBsq);
				BBsq = BBnewsquare;
				if ((BBnwsq | BBnesq) & BBblackpawns) {
					opppawns = 1;
					break;
				}
			}
			if (!opppawns) {
				// no opposing black pawns, it's an outpost
				openingEval += 30;
				endgameEval += 30;
			}
			 */
		}
	}
	
	// black
	
	U64 BBblackknights = (pos->BBblackpieces & pos->BBknights);
	while (BBblackknights) {
		int square = __builtin_ctzll(BBblackknights);
		//BBblackknights &= ~(1ULL << square);
		BBblackknights &= BBblackknights - 1;
		if ((BBpawnWestAttacksW(1ULL << square) & BBblackpawns) || (BBpawnEastAttacksW(1ULL << square) & BBblackpawns)) {
			openingEval -= 20;
			endgameEval -= 20;
			/*
			int opppawns = 0;
			U64 BBsq = 1ULL << square;
			for (int rank = getrank(square);rank > 0;rank--) {
				U64 BBnewsquare = southOne(BBsq);
				U64 BBswsq = soWeOne(BBsq);
				U64 BBsesq = soEaOne(BBsq);
				BBsq = BBnewsquare;
				if ((BBswsq | BBsesq) & BBwhitepawns) {
					opppawns = 1;
					break;
				}
			}
			if (!opppawns) {
				// no opposing white pawns, it's an outpost
				openingEval -= 30;
				endgameEval -= 30;
			}
			 */
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
	
	// bonus for bishops on long diagonal that attack both centre squares
	/*
	BBwhitebishops = pos->BBwhitepieces & pos->BBbishops;
	BBblackbishops = pos->BBblackpieces & pos->BBbishops;
	while (BBwhitebishops) {
		int square = __builtin_ctzll(BBwhitebishops);
		BBwhitebishops &= BBwhitebishops - 1;
		if ((1ULL << square) & (BBdiagA1H8 | BBdiagA8H1)) {
			// bishop is on long diagonal
			U64 BBmoves = Bmagic(square, pos->BBwhitepieces | pos->BBblackpieces) & ~pos->BBwhitepieces;
			if (__builtin_popcountll(BBmoves & BBcentre) == 2) {
				// bishop can see both centre squares on diagonal
				openingEval += 45;
			}
		}
	}
	while (BBblackbishops) {
		int square = __builtin_ctzll(BBblackbishops);
		BBblackbishops &= BBblackbishops - 1;
		if ((1ULL << square) & (BBdiagA1H8 | BBdiagA8H1)) {
			// bishop is on long diagonal
			U64 BBmoves = Bmagic(square, pos->BBwhitepieces | pos->BBblackpieces) & ~pos->BBblackpieces;
			if (__builtin_popcountll(BBmoves & BBcentre) == 2) {
				// bishop can see both centre squares on diagonal
				openingEval -= 45;
			}
		}
	}
	*/
	
	// colour weaknesses
	/*
	int badpawnpenalty[9] = { 0, 2, 3, 4, 6, 7, 10, 14, 19 };
	
	// white
	
	if ((pos->BBblackpieces & pos->BBbishops & BBlightsquares)
		&& !(pos->BBwhitepieces & pos->BBbishops & BBlightsquares)) {
		// black has a light square bishop
		// white doesn't have a light square bishop
		U64 BBWdarkpawns = pos->BBwhitepieces & pos->BBpawns & BBdarksquares;
		int numbadpawns = __builtin_popcountll(BBWdarkpawns);
		// penalty for each pawn on a dark square
		openingEval -= badpawnpenalty[numbadpawns];
		endgameEval -= badpawnpenalty[numbadpawns];
	}
	if ((pos->BBblackpieces & pos->BBbishops & BBdarksquares) 
		&& !(pos->BBwhitepieces & pos->BBbishops & BBdarksquares)) {
		// black has a dark square bishop
		// white doesn't have a dark square bishop
		U64 BBWlightpawns = pos->BBwhitepieces & pos->BBpawns & BBlightsquares;
		int numbadpawns = __builtin_popcountll(BBWlightpawns);
		// penalty for each pawn on a light square
		openingEval -= badpawnpenalty[numbadpawns];
		endgameEval -= badpawnpenalty[numbadpawns];
	}
	
	// black
	if ((pos->BBwhitepieces & pos->BBbishops & BBlightsquares) 
		&& !(pos->BBblackpieces & pos->BBbishops & BBlightsquares)) {
		// white has a light square bishop
		// black doesn't have a light square bishop
		U64 BBBdarkpawns = pos->BBblackpieces & pos->BBpawns & BBdarksquares;
		int numbadpawns = __builtin_popcountll(BBBdarkpawns);
		// penalty for each pawn on a dark square
		openingEval += badpawnpenalty[numbadpawns];
		endgameEval += badpawnpenalty[numbadpawns];
	}
	if ((pos->BBwhitepieces & pos->BBbishops & BBdarksquares) 
		&& !(pos->BBblackpieces & pos->BBbishops & BBdarksquares)) {
		// white has a dark square bishop
		// black doesn't have a dark square bishop
		U64 BBBlightpawns = pos->BBblackpieces & pos->BBpawns & BBlightsquares;
		int numbadpawns = __builtin_popcountll(BBBlightpawns);
		// penalty for each pawn on a light square
		openingEval += badpawnpenalty[numbadpawns];
		endgameEval += badpawnpenalty[numbadpawns];
	}
	*/
	
	// bishops get stronger if there are pawns on both flanks in bishop vs knight endgames
	/*
	U64 BBABCpawns = (BBfileA | BBfileB | BBfileC) & pos->BBpawns;
	U64 BBFGHpawns = (BBfileF | BBfileG | BBfileH) & pos->BBpawns;
	if (num_BN >= 1 && num_BB == 0 && num_WB >= 1) {
		if (BBABCpawns && BBFGHpawns) {
			// pawns on both wings
			endgameEval += 30 * num_WB;
		}
	}
	if (num_WN >= 1 && num_BB == 0 && num_BB >= 1) {
		if (BBABCpawns && BBFGHpawns) {
			// pawns on both wings
			endgameEval -= 30 * num_BB;
		}
	}
	 */
	// penalty for king on pawnless flank
	
	// white
	
	if (getfile(pos->Wkingpos) != 3 && getfile(pos->Wkingpos) != 4) {
		// king is on a flank
		
		U64 BBflank;
		if (getfile(pos->Wkingpos) <= 3) {
			BBflank = BBfileA | BBfileB | BBfileC;
		}
		else if (getfile(pos->Wkingpos) >= 5) {
			BBflank = BBfileF | BBfileG | BBfileH;
		}
		if (!(BBflank & pos->BBpawns)) {
			openingEval -= 17;
			endgameEval -= 95;
		}
	}
	
	// black
	
	if (getfile(pos->Bkingpos) != 3 && getfile(pos->Bkingpos) != 4) {
		// king is on a flank
		
		U64 BBflank;
		if (getfile(pos->Bkingpos) <= 3) {
			BBflank = BBfileA | BBfileB | BBfileC;
		}
		else if (getfile(pos->Bkingpos) >= 5) {
			BBflank = BBfileF | BBfileG | BBfileH;
		}
		if (!(BBflank & pos->BBpawns)) {
			openingEval += 17;
			endgameEval += 95;
		}
	}
	
	
	// rook on the 7th
	
	// white
	/*
	U64 BBWrookson7th = BBrank7 & pos->BBrooks & pos->BBwhitepieces;
	U64 BBBkingon8th = BBrank8 & pos->BBkings & pos->BBblackpieces;
	if (BBWrookson7th & BBBkingon8th) {
		openingEval += 30 * __builtin_popcountll(BBWrookson7th);
		endgameEval += 30 * __builtin_popcountll(BBWrookson7th);
	}
	
	// black
	
	U64 BBBrookson7th = BBrank2 & pos->BBrooks & pos->BBblackpieces;
	U64 BBWkingon8th = BBrank1 & pos->BBkings & pos->BBwhitepieces;
	if (BBBrookson7th & BBWkingon8th) {
		openingEval -= 30 * __builtin_popcountll(BBBrookson7th);
		endgameEval -= 30 * __builtin_popcountll(BBBrookson7th);
	}
	*/
	
	// penalty for minor pieces and central pawns being unmoved in the opening
	
	// white
	/*
	int penalty = 20;
	
	if (getPiece(pos, B1) == 'N') openingEval -= penalty;
	if (getPiece(pos, G1) == 'N') openingEval -= penalty;
	if (getPiece(pos, C1) == 'B') openingEval -= penalty;
	if (getPiece(pos, F1) == 'B') openingEval -= penalty;
	if (getPiece(pos, D2) == 'P') openingEval -= penalty;
	if (getPiece(pos, E2) == 'P') openingEval -= penalty;
	
	// black
	
	if (getPiece(pos, B8) == 'n') openingEval += penalty;
	if (getPiece(pos, G8) == 'n') openingEval += penalty;
	if (getPiece(pos, C8) == 'b') openingEval += penalty;
	if (getPiece(pos, F8) == 'b') openingEval += penalty;
	if (getPiece(pos, D7) == 'p') openingEval += penalty;
	if (getPiece(pos, E7) == 'p') openingEval += penalty;
	*/
	
	// penalty for moving the king or rook without castling
	
	/*
	// white
	if ((pos->Wkingpos != C1 && pos->Wkingpos != G1)) {
		// king isn't in castled position
		if (getPiece(pos, A1) != 'R') {
			openingEval -= 20;
		}
		if (getPiece(pos, H1) != 'R') {
			openingEval -= 20;
		}
	}
	
	if ((pos->Bkingpos != C8 && pos->Bkingpos != G8)) {
		// king isn't in castled position
		if (getPiece(pos, A8) != 'r') {
			openingEval += 20;
		}
		if (getPiece(pos, H8) != 'r') {
			openingEval += 20;
		}
	}
	*/
	// blocked central pawns in opening
	/*
	if (getPiece(pos,C2) == 'P' && getPiece(pos,C3) != '0') {
		openingEval -= 10;
	}
	if (getPiece(pos,D2) == 'P' && getPiece(pos,D3) != '0') {
		openingEval -= 10;
	}
	if (getPiece(pos,E2) == 'P' && getPiece(pos,E3) != '0') {
		openingEval -= 10;
	}
	if (getPiece(pos,F2) == 'P' && getPiece(pos,F3) != '0') {
		openingEval -= 10;
	}
	
	if (getPiece(pos,C7) == 'p' && getPiece(pos,C6) != '0') {
		openingEval += 10;
	}
	if (getPiece(pos,D7) == 'p' && getPiece(pos,D6) != '0') {
		openingEval += 10;
	}
	if (getPiece(pos,E7) == 'p' && getPiece(pos,E6) != '0') {
		openingEval += 10;
	}
	if (getPiece(pos,F7) == 'p' && getPiece(pos,F6) != '0') {
		openingEval += 10;
	}
	*/
	
	// mobility bonuses
	/*
	int Wmobility = mobility(pos,WHITE);
	int Bmobility = mobility(pos,BLACK);
	
	openingEval += Wmobility * 5;
	endgameEval += Wmobility * 5;
	
	openingEval -= Bmobility * 5;
	endgameEval -= Bmobility * 5;
	 */
	int kingattackers = 0;
	int kingattacks = 0;
	// white
	
	struct mobreturn WNmobility = Nmobility(pos,WHITE);
	openingEval += knightMgMobility[WNmobility.mobility];
	endgameEval += knightEgMobility[WNmobility.mobility];
	openingEval += WNmobility.pstO;
	endgameEval += WNmobility.pstE;
	kingattacks +=  WNmobility.kingattacks;
	kingattackers += WNmobility.kingattackers;
	
	struct mobreturn WBmobility = Bmobility(pos,WHITE);
	openingEval += bishopMgMobility[WBmobility.mobility];
	endgameEval += bishopEgMobility[WBmobility.mobility];
	openingEval += WBmobility.pstO;
	endgameEval += WBmobility.pstE;
	kingattacks += WBmobility.kingattacks;
	kingattackers += WBmobility.kingattackers;
	
	struct mobreturn WRmobility = Rmobility(pos,WHITE);
	openingEval += rookMgMobility[WRmobility.mobility];
	endgameEval += rookEgMobility[WRmobility.mobility];
	openingEval += WRmobility.pstO;
	endgameEval += WRmobility.pstE;
	kingattacks += 2 * WRmobility.kingattacks;
	kingattackers += WRmobility.kingattackers;
	
	struct mobreturn WQmobility = Qmobility(pos,WHITE);
	openingEval += queenMgMobility[WQmobility.mobility];
	endgameEval += queenEgMobility[WQmobility.mobility];
	openingEval += WQmobility.pstO;
	endgameEval += WQmobility.pstE;
	kingattacks += 4 * WQmobility.kingattacks;
	kingattackers += WQmobility.kingattackers;
	
	openingEval += safety_table[kingattackers][kingattacks];
	endgameEval += safety_table[kingattackers][kingattacks];
	
	kingattacks = 0;
	kingattackers = 0;
	// black
	struct mobreturn BNmobility = Nmobility(pos,BLACK);
	openingEval -= knightMgMobility[BNmobility.mobility];
	endgameEval -= knightEgMobility[BNmobility.mobility];
	openingEval += BNmobility.pstO;
	endgameEval += BNmobility.pstE;
	kingattacks += BNmobility.kingattacks;
	kingattackers += BNmobility.kingattackers;
	
	struct mobreturn BBmobility = Bmobility(pos,BLACK);
	openingEval -= bishopMgMobility[BBmobility.mobility];
	endgameEval -= bishopEgMobility[BBmobility.mobility];
	openingEval += BBmobility.pstO;
	endgameEval += BBmobility.pstE;
	kingattacks += BBmobility.kingattacks;
	kingattackers += BBmobility.kingattackers;
	
	struct mobreturn BRmobility = Rmobility(pos,BLACK);
	openingEval -= rookMgMobility[WBmobility.mobility];
	endgameEval -= rookEgMobility[WBmobility.mobility];
	openingEval += BRmobility.pstO;
	endgameEval += BRmobility.pstE;
	kingattacks += 2 * BRmobility.kingattacks;
	kingattackers += BRmobility.kingattackers;
	
	struct mobreturn BQmobility = Qmobility(pos,BLACK);
	openingEval -= queenMgMobility[BQmobility.mobility];
	endgameEval -= queenEgMobility[BQmobility.mobility];
	openingEval += BQmobility.pstO;
	endgameEval += BQmobility.pstE;
	kingattacks += 4 * BQmobility.kingattacks;
	kingattackers += BQmobility.kingattackers;
	
	openingEval -= safety_table[kingattackers][kingattacks];
	endgameEval -= safety_table[kingattackers][kingattacks];
	
	// bonus for knights in closed positions, bishops in open positions
	/*
	U64 BBrammed = northOne(pos->BBpawns & pos->BBwhitepieces) & (pos->BBblackpieces & pos->BBpawns);
	
	BBrammed &= ~BBfileH & ~BBfileA & ~BBfileB & ~BBfileG;
	
	double howClosed = (double)__builtin_popcountll(BBrammed) / 4.0; // between 0 and 1, 0 = open, 6 = closed
	
	//printf("%.2f\n", howClosed);
	
	// white bonus for knights in closed positions
	
	openingEval += num_WN * howClosed * 30;
	endgameEval += num_WN * howClosed * 30;
	
	// black bonus for knights in closed positions
	
	openingEval -= num_BN * howClosed * 30;
	endgameEval -= num_BN * howClosed * 30;
	
	// white bonus for bishops in open positions
	
	openingEval += num_WB * (double)(1 - howClosed) * 30;
	endgameEval += num_WB * (double)(1 - howClosed) * 30;
	
	// black bonus for bishops in open positions
	
	openingEval -= num_BB * (double)(1 - howClosed) * 30;
	endgameEval -= num_BB * (double)(1 - howClosed) * 30;
	*/
	
	// bishops blocked by rammed pawns
	/*
	U64 BBrammedB = northOne(pos->BBpawns & pos->BBwhitepieces) & (pos->BBblackpieces & pos->BBpawns);
	U64 BBrammedW = southOne(pos->BBpawns & pos->BBblackpieces) & (pos->BBwhitepieces & pos->BBpawns);
	
	U64 BBWbishops = pos->BBwhitepieces & pos->BBbishops;
	U64 BBBbishops = pos->BBblackpieces & pos->BBbishops;
	
	while (BBWbishops) {
		int square = __builtin_ctzll(BBWbishops);
		BBWbishops &= BBWbishops - 1;
		U64 BBmatching;
		if (BBlightsquares & (1ULL << square)) BBmatching = BBlightsquares;
		else BBmatching = BBdarksquares;
		int count = __builtin_popcountll(BBrammedW & BBmatching);
		openingEval -= 10 * count;
		endgameEval -= 15 * count;
	}
	while (BBBbishops) {
		int square = __builtin_ctzll(BBBbishops);
		BBBbishops &= BBBbishops - 1;
		U64 BBmatching;
		if (BBlightsquares & (1ULL << square)) BBmatching = BBlightsquares;
		else BBmatching = BBdarksquares;
		int count = __builtin_popcountll(BBrammedB & BBmatching);
		openingEval += 10 * count;
		endgameEval += 15 * count;
	}
	*/

	// knight value decreases as pawns disappear
	
	openingEval -= num_WN * (16 - (num_WP + num_BP)) * 4;
	endgameEval -= num_WN * (16 - (num_WP + num_BP)) * 4;
	openingEval += num_BN * (16 - (num_WP + num_BP)) * 4;
	endgameEval += num_BN * (16 - (num_WP + num_BP)) * 4;
	
	// bonus for minor pieces attacking enemy pieces not defended by pawns
	
	// white
	
	U64 BBblackpieces = pos->BBblackpieces & ~pos->BBpawns;
	U64 BBwhitepieces = pos->BBwhitepieces & ~pos->BBpawns;
	U64 BBoccupancy = pos->BBwhitepieces | pos->BBblackpieces;
	
	U64 BBcopy = BBblackpieces;
	while (BBcopy) {
		int square = __builtin_ctzll(BBcopy);
		BBcopy &= BBcopy - 1;
		
		// check if piece is defended by pawn
		U64 BBdefendingpawns = BBpawnattacksW(1ULL << square) & (pos->BBblackpieces & pos->BBpawns);
		if (BBdefendingpawns) continue; // piece is defended by a pawn
		// check if it's attacked by minor piece
		U64 BBattacksN = BBknightattacks(1ULL << square) & (pos->BBwhitepieces & pos->BBknights);
		U64 BBattacksB = Bmagic(square, BBoccupancy) & (pos->BBwhitepieces & pos->BBbishops);
		if (!BBattacksN && !BBattacksB) continue; // not attacked by minor piece
		int piece = getPiece(pos, square);
		openingEval += minorAttackBonus_mg(piece);
		endgameEval += minorAttackBonus_eg(piece);
	}
	
	BBcopy = BBwhitepieces;
	while (BBcopy) {
		int square = __builtin_ctzll(BBcopy);
		BBcopy &= BBcopy - 1;
		
		// check if piece is defended by pawn
		U64 BBdefendingpawns = BBpawnattacksB(1ULL << square) & (pos->BBwhitepieces & pos->BBpawns);
		if (BBdefendingpawns) continue; // piece is defended by a pawn
		// check if it's attacked by minor piece
		U64 BBattacksN = BBknightattacks(1ULL << square) & (pos->BBblackpieces & pos->BBknights);
		U64 BBattacksB = Bmagic(square, BBoccupancy) & (pos->BBblackpieces & pos->BBbishops);
		if (!BBattacksN && !BBattacksB) continue; // not attacked by minor piece
		int piece = getPiece(pos, square);
		openingEval -= minorAttackBonus_mg(piece);
		endgameEval -= minorAttackBonus_eg(piece);
	}
	
	// penalty for pieces attacking king zone
	/*
	// white
	int numattackers = 0;
	int attacksvalue = 0;
	U64 BBblackpieces = pos->BBblackpieces & (pos->BBqueens | pos->BBrooks | pos->BBbishops | pos->BBknights);
	while (BBblackpieces) {
		int square = __builtin_ctzll(BBblackpieces);
		BBblackpieces &= BBblackpieces - 1;
		char piece = getPiece(pos, square);
		U64 BBmoves;
		int weight = 0;
		if (piece == 'n') {
			BBmoves = BBknightattacks(1ULL << square) & ~(pos->BBblackpieces);
			weight = 20;
		}
		if (piece == 'b') {
			BBmoves = Bmagic(square, pos->BBwhitepieces | pos->BBblackpieces) & ~(pos->BBblackpieces);
			weight = 20;
		}
		if (piece == 'r') {
			BBmoves = Rmagic(square, pos->BBwhitepieces | pos->BBblackpieces) & ~(pos->BBblackpieces);
			weight = 40;
		}
		if (piece == 'q') {
			BBmoves = (Rmagic(square, pos->BBwhitepieces | pos->BBblackpieces)
						| Bmagic(square, pos->BBwhitepieces | pos->BBblackpieces)) & ~(pos->BBblackpieces);
			weight = 80;
		}
		U64 BBkingzone = BBkingattacks(1ULL << pos->Wkingpos) & ~BBblackpieces;
		U64 BBattackedsquares = BBmoves & BBkingzone;
		if (BBattackedsquares) {
			numattackers += 1;
			attacksvalue += weight * __builtin_popcountll(BBattackedsquares);
		}
	}
	int attackweight[8] = { 0, 0, 50, 75, 88, 94, 97, 99 };
	int attackscore = (attacksvalue * attackweight[numattackers]) / 100;
	openingEval -= attackscore * 1.5;
	endgameEval -= attackscore * 1.5;

	numattackers = 0;
	attacksvalue = 0;
	U64 BBwhitepieces = pos->BBwhitepieces & (pos->BBqueens | pos->BBrooks | pos->BBbishops | pos->BBknights);
	while (BBwhitepieces) {
		int square = __builtin_ctzll(BBwhitepieces);
		BBwhitepieces &= BBwhitepieces - 1;
		char piece = getPiece(pos, square);
		U64 BBmoves;
		int weight = 0;
		if (piece == 'N') {
			BBmoves = BBknightattacks(1ULL << square) & ~(pos->BBblackpieces);
			weight = 20;
		}
		if (piece == 'B') {
			BBmoves = Bmagic(square, pos->BBwhitepieces | pos->BBblackpieces) & ~(pos->BBwhitepieces);
			weight = 20;
		}
		if (piece == 'R') {
			BBmoves = Rmagic(square, pos->BBwhitepieces | pos->BBblackpieces) & ~(pos->BBwhitepieces);
			weight = 40;
		}
		if (piece == 'Q') {
			BBmoves = (Rmagic(square, pos->BBwhitepieces | pos->BBblackpieces)
						| Bmagic(square, pos->BBwhitepieces | pos->BBblackpieces)) & ~(pos->BBwhitepieces);
			weight = 80;
		}
		U64 BBkingzone = BBkingattacks(1ULL << pos->Bkingpos) & ~BBwhitepieces;
		U64 BBattackedsquares = BBmoves & BBkingzone;
		if (BBattackedsquares) {
			numattackers += 1;
			attacksvalue += weight * __builtin_popcountll(BBattackedsquares);
		}
	}
	attackscore = (attacksvalue * attackweight[numattackers]) / 100;
	openingEval += attackscore * 1.5;
	endgameEval += attackscore * 1.5;
	*/
	// adjust knight value based on number of our pawns
	/*
	int knight_adj[9] = { -20, -16, -12, -8, -4,  0,  4,  8, 12 };
	
	openingEval += num_WN * knight_adj[num_WP] *2;
	endgameEval += num_WN * knight_adj[num_WP] *2;
	openingEval -= num_BN * knight_adj[num_BP] *2;
	endgameEval -= num_BN * knight_adj[num_BP] *2;
	*/
	
	//bishop value increases as pawns disappear
	//openingEval += num_WB * (16 - (num_WP + num_BP)) * 16;
	//endgameEval += num_WB * (16 - (num_WP + num_BP)) * 16;
	//openingEval -= num_BB * (16 - (num_WP + num_BP)) * 16;
	//endgameEval -= num_BB * (16 - (num_WP + num_BP)) * 16;
	
	// adjust bishop value based on number of our pawns
	
	/*
	int bishop_adj[9] = {12, 8, 4, 0, -4, -8, -12, -16, -20 };
	
	openingEval += num_WB * bishop_adj[num_WP];
	endgameEval += num_WB * bishop_adj[num_WP];
	openingEval -= num_BB * bishop_adj[num_BP];
	endgameEval -= num_BB * bishop_adj[num_BP];
	*/
	
	// adjust rook value based on number of our pawns
	
	/*
	int rook_adj[9] = { 15,  12,   9,  6,  3,  0, -3, -6, -9 };

	openingEval += num_WR * rook_adj[num_WP];
	endgameEval += num_WR * rook_adj[num_WP];
	openingEval -= num_BR * rook_adj[num_BP];
	endgameEval -= num_BR * rook_adj[num_BP];
	*/
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

int isTrappedPiece(struct position *pos, int side) {
	if (side == BLACK) {
		U64 BBblackmatpieces = pos->BBblackpieces & (pos->BBbishops | pos->BBknights | pos->BBrooks | pos->BBqueens);
		while (BBblackmatpieces) {
			int square = __builtin_ctzll(BBblackmatpieces);
			BBblackmatpieces &= BBblackmatpieces - 1;
			char piece = getPiece(pos, square);
			int onedge = 0;
			//if (square == A1 || square == A2 || square == B1 || square == B2
			//	|| square == A8 || square == A7 || square == B8 || square == B7
			//	|| square == H1 || square == H2 || square == G1 || square == G2
			//	|| square == H8 || square == H7 || square == G8 || square == G7) {
			//	onedge = 1;
			//}
			//if ((1ULL << square) & ~BBcentre) onedge = 1;
			//if (getfile(square) == 0 || getfile(square) == 7) onedge = 1;
			if ((1ULL << square) & (BBrank1 | BBrank2 | BBrank3 | BBrank4) & ~BBfileD & ~BBfileE) onedge = 1;
			if (!onedge) continue;
			if (piece != 'n' && piece != 'b' && piece != 'q') continue;
			//if (pos->tomove == BLACK) {
			//	struct move prevmove = movestack[movestackend - 1];
			//	if (prevmove.cappiece != '0' && pieceval(prevmove.cappiece) > pieceval(piece)) continue;
			//}
			//if (!isAttacked(pos, square, WHITE)) continue;
			//if (piece == 'r') continue;
			U64 BBmoves;
			if (piece == 'n') {
				BBmoves = BBknightattacks(1ULL << square) & ~(pos->BBblackpieces);
			}
			if (piece == 'b') {
				BBmoves = Bmagic(square, pos->BBwhitepieces | pos->BBblackpieces) & ~(pos->BBblackpieces);
			}
			if (piece == 'r') {
				BBmoves = Rmagic(square, pos->BBwhitepieces | pos->BBblackpieces) & ~(pos->BBblackpieces);
			}
			if (piece == 'q') {
				BBmoves = (Rmagic(square, pos->BBwhitepieces | pos->BBblackpieces)
							| Bmagic(square, pos->BBwhitepieces | pos->BBblackpieces)) & ~(pos->BBblackpieces);
			}
			//if (__builtin_popcountll(BBmoves) == 0 || __builtin_popcountll(BBmoves) > 4) continue;
			int noescape = 1;
			while (BBmoves) {
				int movesquare = __builtin_ctzll(BBmoves);
				BBmoves &= BBmoves - 1;
				int SEEvalue = SEEcapture(pos, square, movesquare, BLACK);
				if (SEEvalue >= 0) {
					noescape = 0;
					break;
				}
			}
			if (noescape) {
				// piece is trapped
				//printf("trapped piece\n");
				return square;
			}
		}
	}
	if (side == WHITE) {
		U64 BBwhitematpieces = pos->BBwhitepieces & (pos->BBbishops | pos->BBknights | pos->BBrooks | pos->BBqueens);
		U64 BBmoves;
		while (BBwhitematpieces) {
			int square = __builtin_ctzll(BBwhitematpieces);
			BBwhitematpieces &= BBwhitematpieces - 1;
			char piece = getPiece(pos, square);
			int onedge = 0;
			//if (square == A1 || square == A2 || square == B1 || square == B2
			//	|| square == A8 || square == A7 || square == B8 || square == B7
			//	|| square == H1 || square == H2 || square == G1 || square == G2
			//	|| square == H8 || square == H7 || square == G8 || square == G7) {
			//	onedge = 1;
			//}
			//if ((1ULL << square) & ~BBcentre) onedge = 1;
			//if (getfile(square) == 0 || getfile(square) == 7) onedge = 1;
			if ((1ULL << square) & (BBrank5 | BBrank6 | BBrank7 | BBrank8)  & ~BBfileD & ~BBfileE) onedge = 1;
			if (!onedge) continue;

			if (piece != 'N' && piece != 'B' && piece != 'Q') continue;
			//if (pos->tomove == WHITE) {
			//	struct move prevmove = movestack[movestackend - 1];
			//	if (prevmove.cappiece != '0' && pieceval(prevmove.cappiece) > pieceval(piece)) continue;
			//}
			//if (!isAttacked(pos, square, BLACK)) continue;
			//if (piece == 'R') continue;
			if (piece == 'N') {
				BBmoves = BBknightattacks(1ULL << square) & ~(pos->BBwhitepieces);
			}
			if (piece == 'B') {
				BBmoves = Bmagic(square, pos->BBwhitepieces | pos->BBblackpieces) & ~(pos->BBwhitepieces);
			}
			if (piece == 'R') {
				BBmoves = Rmagic(square, pos->BBwhitepieces | pos->BBblackpieces) & ~(pos->BBwhitepieces);
			}
			if (piece == 'Q') {
				BBmoves = (Rmagic(square, pos->BBwhitepieces | pos->BBblackpieces)
							| Bmagic(square, pos->BBwhitepieces | pos->BBblackpieces)) & ~(pos->BBwhitepieces);
			}
			//if (__builtin_popcountll(BBmoves) == 0 || __builtin_popcountll(BBmoves) > 4) continue;
			int noescape = 1;
			while (BBmoves) {
				int movesquare = __builtin_ctzll(BBmoves);
				BBmoves &= BBmoves - 1;
				int SEEvalue = SEEcapture(pos, square, movesquare, WHITE);
				//printf("SEEvalue %d %d\n",movesquare,SEEvalue);
				if (SEEvalue >= 0) {
					noescape = 0;
					break;
				}
			}
			if (noescape) {
				
				//printf("trapped pieces\n");
				//dspBoard(pos);
				// piece is trapped
				
				//printf("trapped piece %d\n", square);
				//dspBoard(pos);
				
				//printf("trapped piece %c %d\n", piece, square);
				//dspBoard(pos);
				return square;
			}
		}

	}
	return -1;
}
struct mobreturn Nmobility(struct position *pos, int side) {
	U64 BBsidepieces;
	U64 BBkingzone;
	U64 BBkingonly;
	if (side == WHITE) {
		BBsidepieces = pos->BBwhitepieces;
		BBkingonly = pos->BBkings & pos->BBblackpieces;
		BBkingzone = BBpawnshieldLookup[BLACK][pos->Bkingpos];
	}
	else {
		BBsidepieces = pos->BBblackpieces;
		BBkingonly = pos->BBkings & pos->BBwhitepieces;
		BBkingzone = BBpawnshieldLookup[WHITE][pos->Wkingpos];
	}
	U64 BBallowed = ~BBsidepieces;
	U64 BBoccupied = pos->BBwhitepieces | pos->BBblackpieces;
	U64 BBmoves = 0;
	U64 BBcopy = 0;
	int from = 0;
	
	int kzattacks = 0;
	int kzattackers = 0;
	int PSTvalO = 0;
	int PSTvalE = 0;
	char piece;
	if (side == WHITE) piece = 'N';
	else piece = 'n';
	// Knights
	BBcopy = pos->BBknights & BBsidepieces;
	while(BBcopy)
	{
		from = __builtin_ctzll(BBcopy);
		//BBmoves |= BBknightattacks(1ULL << from) & BBallowed;
		U64 BBnewmoves = BBknightLookup[from] & BBallowed;
		BBmoves |= BBnewmoves;
		U64 BBkzattacks = BBnewmoves & BBkingzone;
		kzattacks += __builtin_popcountll(BBkzattacks);
		if (BBkzattacks) kzattackers++;
		PSTvalO += PSTval(piece,from,'O');
		PSTvalE += PSTval(piece,from,'E');
		BBcopy &= BBcopy-1;
	}
	struct mobreturn returnstruct;
	returnstruct.kingattacks = kzattacks;
	returnstruct.mobility = __builtin_popcountll(BBmoves);
	returnstruct.kingattackers = kzattackers;
	returnstruct.pstO = PSTvalO;
	returnstruct.pstE = PSTvalE;
	return returnstruct;
}
struct mobreturn Bmobility(struct position *pos, int side) {
	U64 BBsidepieces;
	U64 BBkingzone;
	U64 BBkingonly;
	if (side == WHITE) {
		BBsidepieces = pos->BBwhitepieces;
		BBkingonly = pos->BBkings & pos->BBblackpieces;
		BBkingzone = BBpawnshieldLookup[BLACK][pos->Bkingpos];
	}
	else {
		BBsidepieces = pos->BBblackpieces;
		BBkingonly = pos->BBkings & pos->BBwhitepieces;
		BBkingzone = BBpawnshieldLookup[WHITE][pos->Wkingpos];
	}
	int kzattackers = 0;
	int kzattacks = 0;
	U64 BBallowed = ~BBsidepieces;
	U64 BBoccupied = pos->BBwhitepieces | pos->BBblackpieces;
	U64 BBmoves = 0;
	U64 BBcopy = 0;
	int PSTvalO = 0;
	int PSTvalE = 0;
	int from = 0;
	char piece;
	if (side == WHITE) piece = 'B';
	else piece = 'b';
	BBcopy = pos->BBbishops & BBsidepieces;
	while(BBcopy)
	{
		from = __builtin_ctzll(BBcopy);
		U64 BBnewmoves = Bmagic(from, BBoccupied) & BBallowed;
		BBmoves |= BBnewmoves;
		U64 BBkzattacks = BBnewmoves & BBkingzone;
		kzattacks += __builtin_popcountll(BBkzattacks);
		if (BBkzattacks) kzattackers++;
		PSTvalO += PSTval(piece,from,'O');
		PSTvalE += PSTval(piece,from,'E');
		BBcopy &= BBcopy-1;
	}
	struct mobreturn returnstruct;
	returnstruct.kingattacks = kzattacks;
	returnstruct.mobility = __builtin_popcountll(BBmoves);
	returnstruct.kingattackers = kzattackers;
	returnstruct.pstO = PSTvalO;
	returnstruct.pstE = PSTvalE;
	return returnstruct;
}
struct mobreturn Rmobility(struct position *pos, int side) {
	U64 BBsidepieces;
	U64 BBkingzone;
	U64 BBkingonly;
	if (side == WHITE) {
		BBsidepieces = pos->BBwhitepieces;
		BBkingonly = pos->BBkings & pos->BBblackpieces;
		BBkingzone = BBpawnshieldLookup[BLACK][pos->Bkingpos];
	}
	else {
		BBsidepieces = pos->BBblackpieces;
		BBkingonly = pos->BBkings & pos->BBwhitepieces;
		BBkingzone = BBpawnshieldLookup[WHITE][pos->Wkingpos];
	}
	int kzattackers = 0;
	int kzattacks = 0;
	U64 BBallowed = ~BBsidepieces;
	U64 BBoccupied = pos->BBwhitepieces | pos->BBblackpieces;
	U64 BBmoves = 0;
	U64 BBcopy = 0;
	int PSTvalO = 0;
	int PSTvalE = 0;
	char piece;
	if (side == WHITE) piece = 'R';
	else piece = 'r';
	int from = 0;
	BBcopy = pos->BBrooks & BBsidepieces;
	while(BBcopy)
	{
		from = __builtin_ctzll(BBcopy);
		U64 BBnewmoves = Rmagic(from, BBoccupied) & BBallowed;
		BBmoves |= BBnewmoves;
		U64 BBkzattacks = BBnewmoves & BBkingzone;
		kzattacks += __builtin_popcountll(BBkzattacks);
		if (BBkzattacks) kzattackers++;
		PSTvalO += PSTval(piece,from,'O');
		PSTvalE += PSTval(piece,from,'E');
		BBcopy &= BBcopy-1;
	}
	struct mobreturn returnstruct;
	returnstruct.kingattacks = kzattacks;
	returnstruct.mobility = __builtin_popcountll(BBmoves);
	returnstruct.kingattackers = kzattackers;
	returnstruct.pstO = PSTvalO;
	returnstruct.pstE = PSTvalE;
	return returnstruct;
}
struct mobreturn Qmobility(struct position *pos, int side) {
	U64 BBsidepieces;
	U64 BBkingzone;
	U64 BBkingonly;
	if (side == WHITE) {
		BBsidepieces = pos->BBwhitepieces;
		BBkingonly = pos->BBkings & pos->BBblackpieces;
		BBkingzone = BBpawnshieldLookup[BLACK][pos->Bkingpos];
	}
	else {
		BBsidepieces = pos->BBblackpieces;
		BBkingonly = pos->BBkings & pos->BBwhitepieces;
		BBkingzone = BBpawnshieldLookup[WHITE][pos->Wkingpos];
	}
	int kzattackers = 0;
	int kzattacks = 0;
	U64 BBallowed = ~BBsidepieces;
	U64 BBoccupied = pos->BBwhitepieces | pos->BBblackpieces;
	U64 BBmoves = 0;
	U64 BBcopy = 0;
	int PSTvalO = 0;
	int PSTvalE = 0;
	char piece;
	if (side == WHITE) piece = 'Q';
	else piece = 'q';
	int from = 0;
	BBcopy = pos->BBqueens & BBsidepieces;
	while(BBcopy)
	{
		from = __builtin_ctzll(BBcopy);
		U64 BBnewmoves = (Rmagic(from, BBoccupied) | Bmagic(from, BBoccupied)) & BBallowed;
		BBmoves |= BBnewmoves;
		U64 BBkzattacks = BBnewmoves & BBkingzone;
		kzattacks += __builtin_popcountll(BBkzattacks);
		if (BBkzattacks) kzattackers++;
		PSTvalO += PSTval(piece,from,'O');
		PSTvalE += PSTval(piece,from,'E');
		BBcopy &= BBcopy-1;
	}
	struct mobreturn returnstruct;
	returnstruct.kingattacks = kzattacks;
	returnstruct.mobility = __builtin_popcountll(BBmoves);
	returnstruct.kingattackers = kzattackers;
	returnstruct.pstO = PSTvalO;
	returnstruct.pstE = PSTvalE;
	return returnstruct;
}
/*
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
		BBmoves |= BBknightattacks(1ULL << from) & BBallowed;
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
*/
int isEndgame(struct position *pos) {
	int numpieces = 1;
	U64 BBpieces = pos->BBknights | pos->BBbishops | pos->BBrooks | pos->BBqueens;
	if (pos->tomove == WHITE) BBpieces = BBpieces & pos->BBwhitepieces;
	else BBpieces = BBpieces & pos->BBblackpieces;
	numpieces = __builtin_popcountll(BBpieces);
	if (numpieces > 3) return 0;
	return 1;
	//BBoccupied = pos->BBwhitepieces | pos->BBblackpieces;
	/*
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
	 */
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