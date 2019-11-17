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
	
	U64 BBpawnsandkings = pos->BBpawns | pos->BBkings;
	
	while (BBpawnsandkings != 0) {
		int square = __builtin_ctzll(BBpawnsandkings);
		//BBoccupied &= ~(1ULL << square);
		BBpawnsandkings &= BBpawnsandkings - 1;
		char piece = getPiece(pos,square);
		pstvalO = PSTval(piece,square,'O');
		pstvalE = PSTval(piece,square,'E');
		openingEval += pstvalO;
		endgameEval += pstvalE;
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
	
	
	
	
	// passed pawns
	

	int WpassedRankBonus[8] = {0, 10, 10, 15, 25, 80, 120, 0};
	int BpassedRankBonus[8] = {0, 120, 80, 25, 15, 10, 10, 0};
	
	U64 BBwhitePP = 0ULL;
	U64 BBblackPP = 0ULL;
	
	U64 BBwhitepawns = (pos->BBwhitepieces & pos->BBpawns);
	while (BBwhitepawns) {
		
		// passed pawns
		
		int square = __builtin_ctzll(BBwhitepawns);
		BBwhitepawns &= BBwhitepawns - 1;
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
			
		}
		
		// pawn chain bonus
		U64 BBpawnattacks = BBpawnEastAttacksB(BBpiece) | BBpawnWestAttacksB(BBpiece);
		if ((BBpawnattacks & BBwhitepawns)) {
			openingEval += 20;
			endgameEval += 20;
		}
		
		U64 BBfilemask = BBfileA << getfile(square);
		
		// doubled pawns

		U64 BBWpawnsonfile = BBfilemask & (pos->BBwhitepieces & pos->BBpawns);
		U64 BBisdoubled = BBWpawnsonfile & (BBWpawnsonfile-1);
		if (BBisdoubled) {
			openingEval -= 16;
			endgameEval -= 16;
		}
		
		
		// isolated pawns
		
		U64 BBleftpawns = westOne(BBfilemask) & (pos->BBwhitepieces & pos->BBpawns);
		U64 BBrightpawns = eastOne(BBfilemask) & (pos->BBwhitepieces & pos->BBpawns);
		if (BBleftpawns == 0 && BBrightpawns == 0) {
			openingEval -= 6;
			endgameEval -= 6;
		}

		
	}
	
	U64 BBblackpawns = (pos->BBblackpieces & pos->BBpawns);
	while (BBblackpawns) {
		// passed pawns
		int square = __builtin_ctzll(BBblackpawns);
		//BBblackpawns &= ~(1ULL << square);
		BBblackpawns &= BBblackpawns - 1;
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
			
		}
		
		
		// pawn chain bonus
		U64 BBpawnattacks = BBpawnEastAttacksW(BBpiece) | BBpawnWestAttacksW(BBpiece);
		if ((BBpawnattacks & BBblackpawns)) {
			openingEval -= 20;
			endgameEval -= 20;
		}
		U64 BBfilemask = BBfileA << getfile(square);
		
		// Doubled pawns
		U64 BBBpawnsonfile = BBfilemask & (pos->BBblackpieces & pos->BBpawns);
		
		U64 BBisdoubled = BBBpawnsonfile & (BBBpawnsonfile-1);
		if (BBisdoubled) {
			openingEval += 16;
			endgameEval += 16;
		}
		
		// Isolated pawns
		
		U64 BBleftpawns = westOne(BBfilemask) & (pos->BBblackpieces & pos->BBpawns);
		U64 BBrightpawns = eastOne(BBfilemask) & (pos->BBblackpieces & pos->BBpawns);
		if (BBleftpawns == 0 && BBrightpawns == 0) {
			openingEval += 6;
			endgameEval += 6;
		}
	}
	
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
	 
	// king and rook mate
	
	U64 BBwhitenonrookmaterial = (pos->BBpawns | pos->BBqueens | pos->BBbishops | pos->BBknights) & pos->BBwhitepieces;
	U64 BBblackmaterial = (pos->BBpawns | pos->BBqueens | pos->BBbishops | pos->BBknights | pos->BBrooks) & pos->BBblackpieces;
	if  (!BBwhitenonrookmaterial && !BBblackmaterial && num_WR == 1) {
		// KR vs K endgame, white has the rook
		// give a bonus for the enemy king's centre manhattan distance
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
	if  (!BBblacknonrookmaterial == 1 && !BBwhitematerial && num_BR == 1) {
		// KR vs K endgame, black has the rook
		
		// give a bonus for the enemy king's centre manhattan distance
		
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
		
		U64 BBfilemask = BBfileA << getfile(square);
		
		// rooks on open files
		U64 BBpawnsonfile = BBfilemask & pos->BBpawns;
		U64 BBBpawnsonfile = BBfilemask & pos->BBpawns & pos->BBblackpieces;
		U64 BBWpawnsonfile = BBfilemask & pos->BBpawns & pos->BBwhitepieces;
		// white rook on open file
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
		
		// rook on same file as queen
		
		U64 BBBqueensonfile = BBfilemask & (pos->BBqueens & pos->BBblackpieces);
		if (BBBqueensonfile) {
			openingEval += 40;
			endgameEval += 40;
		}
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
		
		U64 BBfilemask = BBfileA << getfile(square);
		
		// rooks on open files
		U64 BBpawnsonfile = BBfilemask & pos->BBpawns;
		U64 BBBpawnsonfile = BBfilemask & pos->BBpawns & pos->BBblackpieces;
		U64 BBWpawnsonfile = BBfilemask & pos->BBpawns & pos->BBwhitepieces;
		// black rook on open file
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
		// rook on same file as queen
		
		U64 BBWqueensonfile = BBfilemask & (pos->BBqueens & pos->BBwhitepieces);
		if (BBWqueensonfile) {
			openingEval -= 40;
			endgameEval -= 40;
		}
	}
	// loop to check for doubled pawns and rooks on open files
	/*
	for (int i = 0;i < 8;i++) {
		// doubled pawns
		// white pawns
		U64 BBfilemask = BBfileA << i;
		
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
	*/
	// pawn shield
	
	// white pawn shield
	
	int Wkingpos = pos->Wkingpos;
	U64 BBpawnshield = BBpawnshieldLookup[WHITE][Wkingpos];
	BBpawnshield &= (pos->BBwhitepieces & pos->BBpawns);
	openingEval += 30 * __builtin_popcountll(BBpawnshield);
	
	// black pawn shield
	
	int Bkingpos = pos->Bkingpos;
	BBpawnshield = BBpawnshieldLookup[BLACK][Bkingpos];
	BBpawnshield &= (pos->BBblackpieces & pos->BBpawns);
	openingEval -= 30 * __builtin_popcountll(BBpawnshield);
	
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

	if (pos->tomove == BLACK) eval = -eval;
	return eval;
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