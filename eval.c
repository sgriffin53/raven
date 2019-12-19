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

// Minor piece attack bonus

int minorAttackBonus_mg[6] = {6, 14, 14, 20, 22, 0};
int minorAttackBonus_eg[6] = {10, 18, 18, 28, 30, 0};
/*
int minorAttackBonus_mg(char piece) {
	switch (piece) {
		case PAWN: return 6;
		case KNIGHT: return 14;
		case BISHOP: return 14;
		case ROOK: return 20;
		case QUEEN: return 22;
		case KING: return 0;
	}
}
int minorAttackBonus_eg(char piece) {
	switch (piece) {
		case PAWN: return 10;
		case KNIGHT: return 18;
		case BISHOP: return 18;
		case ROOK: return 28;
		case QUEEN: return 30;
		case KING: return 0;
	}
}
*/
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
int piecevalues[7] = { 110, 300, 300, 525, 900, 9999, 0 };
int pieceval(const char inpiece) {
	return piecevalues[inpiece];
	/*
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
	 */
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
	
	int num_BP = __builtin_popcountll(pos->colours[BLACK] & pos->pieces[PAWN]);
	int num_BN = __builtin_popcountll(pos->colours[BLACK] & pos->pieces[KNIGHT]);
	int num_BB = __builtin_popcountll(pos->colours[BLACK] & pos->pieces[BISHOP]);
	int num_BR = __builtin_popcountll(pos->colours[BLACK] & pos->pieces[ROOK]);
	int num_BQ = __builtin_popcountll(pos->colours[BLACK] & pos->pieces[QUEEN]);
	int num_WP = __builtin_popcountll(pos->colours[WHITE] & pos->pieces[PAWN]);
	int num_WN = __builtin_popcountll(pos->colours[WHITE] & pos->pieces[KNIGHT]);
	int num_WB = __builtin_popcountll(pos->colours[WHITE] & pos->pieces[BISHOP]);
	int num_WR = __builtin_popcountll(pos->colours[WHITE] & pos->pieces[ROOK]);
	int num_WQ = __builtin_popcountll(pos->colours[WHITE] & pos->pieces[QUEEN]);
	
	// Piece values
    int white_pieces = num_WP*pieceval(PAWN)   +
                       num_WN*pieceval(KNIGHT) +
                       num_WB*pieceval(BISHOP) +
                       num_WR*pieceval(ROOK)   +
                       num_WQ*pieceval(QUEEN);

    int black_pieces = num_BP*pieceval(PAWN)   +
                       num_BN*pieceval(KNIGHT) +
                       num_BB*pieceval(BISHOP) +
                       num_BR*pieceval(ROOK)   +
                       num_BQ*pieceval(QUEEN);

    openingEval += white_pieces - black_pieces;
	endgameEval += white_pieces - black_pieces;
	material += white_pieces - black_pieces;
	
	//printf("after material: %d %d\n", openingEval, endgameEval);
	
	U64 BBpawnsandkings = pos->pieces[PAWN] | pos->pieces[KING];
	
	while (BBpawnsandkings != 0) {
		int square = __builtin_ctzll(BBpawnsandkings);
		//BBoccupied &= ~(1ULL << square);
		BBpawnsandkings &= BBpawnsandkings - 1;
		char piece = getPiece(pos,square);
		int col = getColour(pos, square);
		pstvalO = PSTval(col, piece,square,'O');
		pstvalE = PSTval(col, piece,square,'E');
		//printf("square: %d pst vals: %d %d\n", square, pstvalO, pstvalE);
		openingEval += pstvalO;
		endgameEval += pstvalE;
	}
	
	//printf("after first pst: %d %d\n", openingEval, endgameEval);
		
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
	
	int passedFileBonus_mg[8] = { 25, 11, -14, -14, -14, -14, 11, 25 };
	int passedFileBonus_eg[8] = { 20, 15, 5, -7, -7, 5, 15, 20 };
	
	U64 BBwhitePP = 0ULL;
	U64 BBblackPP = 0ULL;
	
	U64 BBwhitepawns = (pos->colours[WHITE] & pos->pieces[PAWN]);
	while (BBwhitepawns) {
		
		// passed pawns
		
		int square = __builtin_ctzll(BBwhitepawns);
		BBwhitepawns &= BBwhitepawns - 1;
		U64 BBpiece = (1ULL << square);
		int startrank = getrank(square);
		U64 BBenemypawns = BBpasserLookup[WHITE][square] & (pos->colours[BLACK] & pos->pieces[PAWN]);
		if (BBenemypawns == 0) {
			// pawn is passed
			
			BBwhitePP |= square; // add square to bb of white passed pawns
			int bonus = WpassedRankBonus[startrank];
			openingEval += 0.5 * bonus;
			endgameEval += 1 * bonus;
			
			openingEval += passedFileBonus_mg[getfile(square)];
			endgameEval += passedFileBonus_eg[getfile(square)];
			
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

		U64 BBWpawnsonfile = BBfilemask & (pos->colours[WHITE] & pos->pieces[PAWN]);
		U64 BBisdoubled = BBWpawnsonfile & (BBWpawnsonfile-1);
		if (BBisdoubled) {
			openingEval -= 16;
			endgameEval -= 16;
		}
		
		
		// isolated pawns
		
		U64 BBleftpawns = westOne(BBfilemask) & (pos->colours[WHITE] & pos->pieces[PAWN]);
		U64 BBrightpawns = eastOne(BBfilemask) & (pos->colours[WHITE] & pos->pieces[PAWN]);
		if (BBleftpawns == 0 && BBrightpawns == 0) {
			openingEval -= 6;
			endgameEval -= 6;
		}

		
	}
	
	U64 BBblackpawns = (pos->colours[BLACK] & pos->pieces[PAWN]);
	while (BBblackpawns) {
		// passed pawns
		int square = __builtin_ctzll(BBblackpawns);
		//BBblackpawns &= ~(1ULL << square);
		BBblackpawns &= BBblackpawns - 1;
		U64 BBpiece = (1ULL << square);
		int startrank = getrank(square);
		U64 BBenemypawns = (BBpasserLookup[BLACK][square] & (pos->colours[WHITE] & pos->pieces[PAWN]));
		if (BBenemypawns == 0) {
			BBblackPP |= square;
			int bonus = BpassedRankBonus[startrank];
			openingEval -= 0.5 * bonus;
			endgameEval -= 1 * bonus;
			
			openingEval -= passedFileBonus_mg[getfile(square)];
			endgameEval -= passedFileBonus_eg[getfile(square)];
			
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
		U64 BBpawnsonfile = BBfilemask & (pos->colours[BLACK] & pos->pieces[PAWN]);
		
		U64 BBisdoubled = BBpawnsonfile & (BBpawnsonfile-1);
		if (BBisdoubled) {
			openingEval += 16;
			endgameEval += 16;
		}
		
		// Isolated pawns
		
		U64 BBleftpawns = westOne(BBfilemask) & (pos->colours[BLACK] & pos->pieces[PAWN]);
		U64 BBrightpawns = eastOne(BBfilemask) & (pos->colours[BLACK] & pos->pieces[PAWN]);
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
	
	U64 BBwhitenonrookmaterial = (pos->pieces[PAWN] | pos->pieces[QUEEN] | pos->pieces[BISHOP] | pos->pieces[KNIGHT]) & pos->colours[WHITE];
	U64 BBblackmaterial = (pos->pieces[PAWN] | pos->pieces[QUEEN] | pos->pieces[BISHOP] | pos->pieces[KNIGHT] | pos->pieces[ROOK]) & pos->colours[BLACK];
	if  (!BBwhitenonrookmaterial && !BBblackmaterial && num_WR == 1) {
		// KR vs K endgame, white has the rook
		// give a bonus for the enemy king's centre manhattan distance
		int xdist = abs(getfile(pos->Wkingpos) - getfile(pos->Bkingpos));
		int ydist = abs(getrank(pos->Wkingpos) - getrank(pos->Bkingpos));
		
		if (xdist == 2 && ydist == 0) {
			// king is opposing enemy king two files away
			endgameEval += 200;
			// check if enemy king is on same file as rook
			int square = __builtin_ctzll(pos->colours[WHITE] & pos->pieces[ROOK]);
			if (getfile(pos->Bkingpos) == getfile(square)) {
				endgameEval += 300;
			}
		}
		if (ydist == 2 && xdist == 0) {
			// king is opposing enemy king two ranks away
			endgameEval += 200;
			// check if enemy king is on same rank as rook
			int square = __builtin_ctzll(pos->colours[WHITE] & pos->pieces[ROOK]);
			if (getrank(pos->Bkingpos) == getrank(square)) {
				endgameEval += 300;
			}
		}
	} 
	U64 BBblacknonrookmaterial = (pos->pieces[PAWN] | pos->pieces[QUEEN] | pos->pieces[BISHOP] | pos->pieces[KNIGHT]) & pos->colours[BLACK];
	U64 BBwhitematerial = (pos->pieces[PAWN] | pos->pieces[QUEEN] | pos->pieces[BISHOP] | pos->pieces[KNIGHT] | pos->pieces[ROOK]) & pos->colours[WHITE];
	if  (!BBblacknonrookmaterial == 1 && !BBwhitematerial && num_BR == 1) {
		// KR vs K endgame, black has the rook
		
		// give a bonus for the enemy king's centre manhattan distance
		
		int xdist = abs(getfile(pos->Wkingpos) - getfile(pos->Bkingpos));
		int ydist = abs(getrank(pos->Wkingpos) - getrank(pos->Bkingpos));
		
		if (xdist == 2 && ydist == 0) {
			// king is opposing enemy king two files away
			endgameEval -= 200;
			// check if enemy king is on same file as rook
			int square = __builtin_ctzll(pos->colours[BLACK] & pos->pieces[ROOK]);
			if (getfile(pos->Wkingpos) == getfile(square)) {
				endgameEval -= 300;
			}
		}
		if (ydist == 2 && xdist == 0) {
			// king is opposing enemy king two ranks away
			endgameEval -= 200;
			// check if enemy king is on same rank as rook
			int square = __builtin_ctzll(pos->colours[BLACK] & pos->pieces[ROOK]);
			if (getrank(pos->Wkingpos) == getrank(square)) {
				endgameEval -= 300;
			}
		}
	} 
	
	// bonus for rooks/queens being on 7th rank
	
	U64 BBwhitequeens = pos->colours[WHITE] & pos->pieces[QUEEN];
	U64 BBblackqueens = pos->colours[BLACK] & pos->pieces[QUEEN];
	
	U64 BBwhiterooks = pos->colours[WHITE] & pos->pieces[ROOK];
	U64 BBblackrooks = pos->colours[BLACK] & pos->pieces[ROOK];
	
	while (BBwhitequeens) {
		int square = __builtin_ctzll(BBwhitequeens);
		BBwhitequeens &= BBwhitequeens - 1;
		if (getrank(square) != 6) continue;
		// queen on 7th rank
		U64 BBhostilepawns = BBrank7 & pos->colours[BLACK] & pos->pieces[PAWN];
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
		U64 BBhostilepawns = BBrank2 & pos->colours[WHITE] & pos->pieces[PAWN];
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
		U64 BBhostilepawns = BBrank7 & pos->colours[BLACK] & pos->pieces[PAWN];
		if (!BBhostilepawns && getrank(pos->Bkingpos) != 7) continue;
		// either hostile pawns on 7th rank or king is on 8th rank
		
		openingEval += 20;
		endgameEval += 40;
		
		U64 BBfilemask = BBfileA << getfile(square);
		
		// rooks on open files
		U64 BBpawnsonfile = BBfilemask & pos->pieces[PAWN];
		U64 BBBpawnsonfile = BBfilemask & pos->pieces[PAWN] & pos->colours[BLACK];
		U64 BBWpawnsonfile = BBfilemask & pos->pieces[PAWN] & pos->colours[WHITE];
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
		
		U64 BBBqueensonfile = BBfilemask & (pos->pieces[QUEEN] & pos->colours[BLACK]);
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
		U64 BBhostilepawns = BBrank2 & pos->colours[WHITE] & pos->pieces[PAWN];
		if (!BBhostilepawns && getrank(pos->Wkingpos) != 0) continue;
		// either hostile pawns on 7th rank or king is on 8th rank
		
		openingEval -= 20;
		endgameEval -= 40;
		
		U64 BBfilemask = BBfileA << getfile(square);
		
		// rooks on open files
		U64 BBpawnsonfile = BBfilemask & pos->pieces[PAWN];
		U64 BBBpawnsonfile = BBfilemask & pos->pieces[PAWN] & pos->colours[BLACK];
		U64 BBWpawnsonfile = BBfilemask & pos->pieces[PAWN] & pos->colours[WHITE];
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
		
		U64 BBWqueensonfile = BBfilemask & (pos->pieces[QUEEN] & pos->colours[WHITE]);
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
		
		U64 BBWpawnsonfile = BBfilemask & (pos->colours[WHITE] & pos->pieces[PAWN]);
		
		U64 BBisdoubled = BBWpawnsonfile & (BBWpawnsonfile-1);
		if (BBisdoubled) {
			openingEval -= 16;
			endgameEval -= 16;
		}
		// black pawns
		U64 Bpieces[PAWN]onfile = BBfilemask & (pos->colours[BLACK] & pos->pieces[PAWN]);
		BBisdoubled = Bpieces[PAWN]onfile & (Bpieces[PAWN]onfile-1);
		if (BBisdoubled) {
			openingEval += 16;
			endgameEval += 16;
		}
		
		
		// isolated pawns

		if (BBWpawnsonfile) {
			U64 BBleftpawns = westOne(BBfilemask) & (pos->colours[WHITE] & pos->pieces[PAWN]);
			U64 BBrightpawns = eastOne(BBfilemask) & (pos->colours[WHITE] & pos->pieces[PAWN]);
			if (BBleftpawns == 0 && BBrightpawns == 0) {
				openingEval -= 6;
				endgameEval -= 6;
			}
		}
		if (Bpieces[PAWN]onfile) {
			U64 BBleftpawns = westOne(BBfilemask) & (pos->colours[BLACK] & pos->pieces[PAWN]);
			U64 BBrightpawns = eastOne(BBfilemask) & (pos->colours[BLACK] & pos->pieces[PAWN]);
			if (BBleftpawns == 0 && BBrightpawns == 0) {
				openingEval += 6;
				endgameEval += 6;
			}
		}
		
		// rooks on open files
		U64 pieces[PAWN]onfile = BBfilemask & pos->pieces[PAWN];
		// white rook on open file
		U64 BBWrooksonfile = BBfilemask & (pos->pieces[ROOK] & pos->colours[WHITE]);
		if (BBWrooksonfile) {
			if (pieces[PAWN]onfile == 0) {
				// white rook on open file
				openingEval += 48;
				endgameEval += 16;
			}
			if ((BBWpawnsonfile == 0) && (Bpieces[PAWN]onfile)) {
				// white rook on semi-open file with black pawns
				openingEval += 6;
				endgameEval += 6;
			}
		}
		// black rooks on open file
		U64 Bpieces[ROOK]onfile = BBfilemask & (pos->pieces[ROOK] & pos->colours[BLACK]);
		if (Bpieces[ROOK]onfile) {
			if (pieces[PAWN]onfile == 0) {
				// black rook on open file
				openingEval -= 48;
				endgameEval -= 16;
			}
			if ((Bpieces[PAWN]onfile == 0) && (BBWpawnsonfile)) {
				// black rook on semi-open file with white pawns
				openingEval -= 6;
				endgameEval -= 6;
			}
		}

		// rooks on same file as queen
		U64 BBWqueensonfile = BBfilemask & (pos->pieces[QUEEN] & pos->colours[WHITE]);
		U64 Bpieces[QUEEN]onfile = BBfilemask & (pos->pieces[QUEEN] & pos->colours[BLACK]);
		if (BBWrooksonfile) {
			if (Bpieces[QUEEN]onfile) {
				openingEval += 40;
				endgameEval += 40;
			}
		}
		if (Bpieces[ROOK]onfile) {
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
	BBpawnshield &= (pos->colours[WHITE] & pos->pieces[PAWN]);
	openingEval += 30 * __builtin_popcountll(BBpawnshield);
	
	// black pawn shield
	
	int Bkingpos = pos->Bkingpos;
	BBpawnshield = BBpawnshieldLookup[BLACK][Bkingpos];
	BBpawnshield &= (pos->colours[BLACK] & pos->pieces[PAWN]);
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
	
	U64 BBWpiecesincentre = (pos->colours[WHITE] & pos->pieces[PAWN] & BBcentre);
	openingEval += 20 * __builtin_popcountll(BBWpiecesincentre);
	endgameEval += 20 * __builtin_popcountll(BBWpiecesincentre);
	
	U64 BBBpiecesincentre = (pos->colours[BLACK] & pos->pieces[PAWN] & BBcentre);
	openingEval -= 20 * __builtin_popcountll(BBBpiecesincentre);
	endgameEval -= 20 * __builtin_popcountll(BBBpiecesincentre);
	
	// bonus for pawns attacking the centre
	
	U64 BBWattackingcentre = BBpawnattacksW(pos->colours[WHITE] & pos->pieces[PAWN]) & BBcentre;
	openingEval += 20 * __builtin_popcountll(BBWattackingcentre);
	endgameEval += 10 * __builtin_popcountll(BBWattackingcentre);
	
	U64 BBBattackingcentre = BBpawnattacksB(pos->colours[BLACK] & pos->pieces[PAWN]) & BBcentre;
	openingEval -= 20 * __builtin_popcountll(BBBattackingcentre);
	endgameEval -= 10 * __builtin_popcountll(BBBattackingcentre);
	
	// bonus for connected knights
	// white
	if (num_WN >= 2) {
		U64 BBWknights = (pos->colours[WHITE] & pos->pieces[KNIGHT]);
		U64 BBattacks = BBknightattacks(BBWknights);
		U64 BBconnectedknights = BBattacks & BBWknights;
		if (BBconnectedknights) {
			openingEval += 25;
			endgameEval += 25;
		}
	}
	// black
	if (num_BN >= 2) {
		U64 BBBknights = (pos->colours[BLACK] & pos->pieces[KNIGHT]);
		U64 BBattacks = BBknightattacks(BBBknights);
		U64 BBconnectedknights = BBattacks & BBBknights;
		if (BBconnectedknights) {
			openingEval -= 25;
			endgameEval -= 25;
		}
	}
	
	// bonus for trading when ahead in material
	
	int whitematval = num_WN * pieceval(KNIGHT) + num_WB * pieceval(BISHOP) + num_WR * pieceval(ROOK) + num_WQ * pieceval(QUEEN);
	int blackmatval = num_BN * pieceval(KNIGHT) + num_BB * pieceval(BISHOP) + num_BR * pieceval(ROOK) + num_BQ * pieceval(QUEEN);
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
	
	BBwhitepawns = (pos->colours[WHITE] & pos->pieces[PAWN]);
	BBblackpawns = (pos->colours[BLACK] & pos->pieces[PAWN]);
	
	U64 BBwhiteknights = (pos->colours[WHITE] & pos->pieces[KNIGHT]);
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
	
	U64 BBblackknights = (pos->colours[BLACK] & pos->pieces[KNIGHT]);
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
	
	U64 BBwhitebishops = (pos->colours[WHITE] & pos->pieces[BISHOP]);
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
	
	U64 BBblackbishops = (pos->colours[BLACK] & pos->pieces[BISHOP]);
	while (BBblackbishops) {
		int square = __builtin_ctzll(BBblackbishops);
		//BBblackbishops &= ~(1ULL << square);
		BBblackbishops &= BBblackbishops - 1;
		if ((BBpawnWestAttacksW(1ULL << square) & BBblackpawns) || (BBpawnEastAttacksW(1ULL << square) & BBblackpawns)) {
			openingEval -= 20;
			endgameEval -= 20;
		}
	}
	
	
	// bonus for knights in closed positions
	// bonus for bishops in open positions
	
	U64 BBWrammed = southOne(pos->pieces[PAWN] & pos->colours[BLACK]) & (pos->pieces[PAWN] & pos->colours[WHITE]);
	U64 BBBrammed = northOne(pos->pieces[PAWN] & pos->colours[WHITE]) & (pos->pieces[PAWN] & pos->colours[BLACK]);
	
	int rammedpairs = __builtin_popcountll(BBWrammed);
	int nonrammedpawns = __builtin_popcountll(pos->pieces[PAWN] & ~BBWrammed & ~BBBrammed);
	
	// closedness is a number from -8 to 8, -8 being completely open, 8 being completely closed
		
	double closedness = rammedpairs * 2 + nonrammedpawns / 2 - 8;
	
	// white knight bonus
	
	if (closedness > 0) {
		openingEval += num_WN * (closedness / 8.0) * 20;
		endgameEval += num_WN * (closedness / 8.0) * 20;
	}
	
	// white bishop bonus
	
	if (closedness < 0) {
		openingEval += num_WB * (-closedness / 8.0) * 20;
		endgameEval += num_WB * (-closedness / 8.0) * 20;
	}
	
	// black knight bonus
	
	if (closedness > 0) {
		openingEval -= num_BN * (closedness / 8.0) * 20;
		endgameEval -= num_BN * (closedness / 8.0) * 20;
	}
	
	// black bishop bonus
	
	if (closedness < 0) {
		openingEval -= num_BB * (-closedness / 8.0) * 20;
		endgameEval -= num_BB * (-closedness / 8.0) * 20;
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
		if (!(BBflank & pos->pieces[PAWN])) {
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
		if (!(BBflank & pos->pieces[PAWN])) {
			openingEval += 17;
			endgameEval += 95;
		}
	}
	
	int kingattackers = 0;
	int kingattacks = 0;
	// white
	
	int idx = 0;
	
	struct mobreturn WNmobility = Nmobility(pos,WHITE);
	idx = max(0, WNmobility.mobility - WNmobility.unsafe * 2);
	openingEval += knightMgMobility[idx];
	endgameEval += knightEgMobility[idx];
	openingEval += WNmobility.pstO;
	endgameEval += WNmobility.pstE;
	kingattacks +=  WNmobility.kingattacks;
	kingattackers += WNmobility.kingattackers;
	
	struct mobreturn WBmobility = Bmobility(pos,WHITE);
	idx = max(0, WBmobility.mobility - WBmobility.unsafe * 2);
	openingEval += bishopMgMobility[idx];
	endgameEval += bishopEgMobility[idx];
	openingEval += WBmobility.pstO;
	endgameEval += WBmobility.pstE;
	kingattacks += WBmobility.kingattacks;
	kingattackers += WBmobility.kingattackers;
	
	struct mobreturn WRmobility = Rmobility(pos,WHITE);
	idx = max(0, WRmobility.mobility - WRmobility.unsafe * 2);
	openingEval += rookMgMobility[idx];
	endgameEval += rookEgMobility[idx];
	openingEval += WRmobility.pstO;
	endgameEval += WRmobility.pstE;
	kingattacks += 2 * WRmobility.kingattacks;
	kingattackers += WRmobility.kingattackers;
	
	struct mobreturn WQmobility = Qmobility(pos,WHITE);
	idx = max(0, WQmobility.mobility - WQmobility.unsafe * 2);
	openingEval += queenMgMobility[idx];
	endgameEval += queenEgMobility[idx];
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
	idx = max(0, BNmobility.mobility - BNmobility.unsafe * 2);
	openingEval -= knightMgMobility[idx];
	endgameEval -= knightEgMobility[idx];
	openingEval += BNmobility.pstO;
	endgameEval += BNmobility.pstE;
	kingattacks += BNmobility.kingattacks;
	kingattackers += BNmobility.kingattackers;
	
	struct mobreturn BBmobility = Bmobility(pos,BLACK);
	idx = max(0, BBmobility.mobility - BBmobility.unsafe * 2);
	openingEval -= bishopMgMobility[idx];
	endgameEval -= bishopEgMobility[idx];
	openingEval += BBmobility.pstO;
	endgameEval += BBmobility.pstE;
	kingattacks += BBmobility.kingattacks;
	kingattackers += BBmobility.kingattackers;
	
	struct mobreturn BRmobility = Rmobility(pos,BLACK);
	idx = max(0, BRmobility.mobility - BRmobility.unsafe * 2);
	openingEval -= rookMgMobility[idx];
	endgameEval -= rookEgMobility[idx];
	openingEval += BRmobility.pstO;
	endgameEval += BRmobility.pstE;
	kingattacks += 2 * BRmobility.kingattacks;
	kingattackers += BRmobility.kingattackers;
	
	struct mobreturn BQmobility = Qmobility(pos,BLACK);
	idx = max(0, BQmobility.mobility - BQmobility.unsafe * 2);
	openingEval -= queenMgMobility[idx];
	endgameEval -= queenEgMobility[idx];
	openingEval += BQmobility.pstO;
	endgameEval += BQmobility.pstE;
	kingattacks += 4 * BQmobility.kingattacks;
	kingattackers += BQmobility.kingattackers;
	
	openingEval -= safety_table[kingattackers][kingattacks];
	endgameEval -= safety_table[kingattackers][kingattacks];

	// knight value decreases as pawns disappear
	
	/*
	openingEval -= num_WN * (16 - (num_WP + num_BP)) * 4;
	endgameEval -= num_WN * (16 - (num_WP + num_BP)) * 4;
	openingEval += num_BN * (16 - (num_WP + num_BP)) * 4;
	endgameEval += num_BN * (16 - (num_WP + num_BP)) * 4;
	*/
	
	// bonus for minor pieces attacking enemy pieces not defended by pawns
	
	// white
	
	U64 BBblackpieces = pos->colours[BLACK] & ~pos->pieces[PAWN];
	U64 BBwhitepieces = pos->colours[WHITE] & ~pos->pieces[PAWN];
	U64 BBoccupancy = pos->colours[WHITE] | pos->colours[BLACK];
	
	U64 BBcopy = BBblackpieces;
	while (BBcopy) {
		int square = __builtin_ctzll(BBcopy);
		BBcopy &= BBcopy - 1;
		
		// check if piece is defended by pawn
		U64 BBdefendingpawns = BBpawnattacksW(1ULL << square) & (pos->colours[BLACK] & pos->pieces[PAWN]);
		if (BBdefendingpawns) continue; // piece is defended by a pawn
		// check if it's attacked by minor piece
		U64 BBattacksN = BBknightLookup[square] & (pos->colours[WHITE] & pos->pieces[KNIGHT]);
		U64 BBattacksB = Bmagic(square, BBoccupancy) & (pos->colours[WHITE] & pos->pieces[BISHOP]);
		if (!BBattacksN && !BBattacksB) continue; // not attacked by minor piece
		int piece = getPiece(pos, square);
		openingEval += minorAttackBonus_mg[piece];
		endgameEval += minorAttackBonus_eg[piece];
	}
	
	BBcopy = BBwhitepieces;
	while (BBcopy) {
		int square = __builtin_ctzll(BBcopy);
		BBcopy &= BBcopy - 1;
		
		// check if piece is defended by pawn
		U64 BBdefendingpawns = BBpawnattacksB(1ULL << square) & (pos->colours[WHITE] & pos->pieces[PAWN]);
		if (BBdefendingpawns) continue; // piece is defended by a pawn
		// check if it's attacked by minor piece
		U64 BBattacksN = BBknightLookup[square] & (pos->colours[BLACK] & pos->pieces[KNIGHT]);
		U64 BBattacksB = Bmagic(square, BBoccupancy) & (pos->colours[BLACK] & pos->pieces[BISHOP]);
		if (!BBattacksN && !BBattacksB) continue; // not attacked by minor piece
		int piece = getPiece(pos, square);
		openingEval -= minorAttackBonus_mg[piece];
		endgameEval -= minorAttackBonus_eg[piece];
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
	U64 BBattackedbypawns;
	if (side == WHITE) {
		BBsidepieces = pos->colours[WHITE];
		BBkingonly = pos->pieces[KING] & pos->colours[BLACK];
		BBkingzone = BBpawnshieldLookup[BLACK][pos->Bkingpos];
		BBattackedbypawns = BBpawnattacksB(pos->pieces[PAWN] & pos->colours[BLACK]);
	}
	else {
		BBsidepieces = pos->colours[BLACK];
		BBkingonly = pos->pieces[KING] & pos->colours[WHITE];
		BBkingzone = BBpawnshieldLookup[WHITE][pos->Wkingpos];
		BBattackedbypawns = BBpawnattacksW(pos->pieces[PAWN] & pos->colours[WHITE]);
	}
	U64 BBallowed = ~BBsidepieces;
	U64 BBoccupied = pos->colours[WHITE] | pos->colours[BLACK];
	U64 BBmoves = 0;
	U64 BBcopy = 0;
	int from = 0;
	
	int kzattacks = 0;
	int kzattackers = 0;
	int PSTvalO = 0;
	int PSTvalE = 0;
	char piece = KNIGHT;
	// Knights
	BBcopy = pos->pieces[KNIGHT] & BBsidepieces;
	while(BBcopy)
	{
		from = __builtin_ctzll(BBcopy);
		//BBmoves |= BBknightattacks(1ULL << from) & BBallowed;
		U64 BBnewmoves = BBknightLookup[from] & BBallowed;
		BBmoves |= BBnewmoves;
		U64 BBkzattacks = BBnewmoves & BBkingzone;
		kzattacks += __builtin_popcountll(BBkzattacks);
		if (BBkzattacks) kzattackers++;
		PSTvalO += PSTval(side, piece,from,'O');
		PSTvalE += PSTval(side, piece,from,'E');
		BBcopy &= BBcopy-1;
	}
	struct mobreturn returnstruct;
	returnstruct.kingattacks = kzattacks;
	returnstruct.mobility = __builtin_popcountll(BBmoves);
	returnstruct.kingattackers = kzattackers;
	returnstruct.pstO = PSTvalO;
	returnstruct.pstE = PSTvalE;
	returnstruct.unsafe = __builtin_popcountll(BBmoves & BBattackedbypawns);
	return returnstruct;
}
struct mobreturn Bmobility(struct position *pos, int side) {
	U64 BBsidepieces;
	U64 BBkingzone;
	U64 BBkingonly;
	U64 BBattackedbypawns;
	if (side == WHITE) {
		BBsidepieces = pos->colours[WHITE];
		BBkingonly = pos->pieces[KING] & pos->colours[BLACK];
		BBkingzone = BBpawnshieldLookup[BLACK][pos->Bkingpos];
		BBattackedbypawns = BBpawnattacksB(pos->pieces[PAWN] & pos->colours[BLACK]);
	}
	else {
		BBsidepieces = pos->colours[BLACK];
		BBkingonly = pos->pieces[KING] & pos->colours[WHITE];
		BBkingzone = BBpawnshieldLookup[WHITE][pos->Wkingpos];
		BBattackedbypawns = BBpawnattacksW(pos->pieces[PAWN] & pos->colours[WHITE]);
	}
	int kzattackers = 0;
	int kzattacks = 0;
	U64 BBallowed = ~BBsidepieces;
	U64 BBoccupied = pos->colours[WHITE] | pos->colours[BLACK];
	U64 BBmoves = 0;
	U64 BBcopy = 0;
	int PSTvalO = 0;
	int PSTvalE = 0;
	int from = 0;
	char piece = BISHOP;
	BBcopy = pos->pieces[BISHOP] & BBsidepieces;
	while(BBcopy)
	{
		from = __builtin_ctzll(BBcopy);
		U64 BBnewmoves = Bmagic(from, BBoccupied) & BBallowed;
		BBmoves |= BBnewmoves;
		U64 BBkzattacks = BBnewmoves & BBkingzone;
		kzattacks += __builtin_popcountll(BBkzattacks);
		if (BBkzattacks) kzattackers++;
		PSTvalO += PSTval(side, piece,from,'O');
		PSTvalE += PSTval(side, piece,from,'E');
		BBcopy &= BBcopy-1;
	}
	struct mobreturn returnstruct;
	returnstruct.kingattacks = kzattacks;
	returnstruct.mobility = __builtin_popcountll(BBmoves);
	returnstruct.kingattackers = kzattackers;
	returnstruct.pstO = PSTvalO;
	returnstruct.pstE = PSTvalE;
	returnstruct.unsafe = __builtin_popcountll(BBmoves & BBattackedbypawns);
	return returnstruct;
}
struct mobreturn Rmobility(struct position *pos, int side) {
	U64 BBsidepieces;
	U64 BBkingzone;
	U64 BBkingonly;
	U64 BBattackedbypawns;
	if (side == WHITE) {
		BBsidepieces = pos->colours[WHITE];
		BBkingonly = pos->pieces[KING] & pos->colours[BLACK];
		BBkingzone = BBpawnshieldLookup[BLACK][pos->Bkingpos];
		BBattackedbypawns = BBpawnattacksB(pos->pieces[PAWN] & pos->colours[BLACK]);
	}
	else {
		BBsidepieces = pos->colours[BLACK];
		BBkingonly = pos->pieces[KING] & pos->colours[WHITE];
		BBkingzone = BBpawnshieldLookup[WHITE][pos->Wkingpos];
		BBattackedbypawns = BBpawnattacksW(pos->pieces[PAWN] & pos->colours[WHITE]);
	}
	int kzattackers = 0;
	int kzattacks = 0;
	U64 BBallowed = ~BBsidepieces;
	U64 BBoccupied = pos->colours[WHITE] | pos->colours[BLACK];
	U64 BBmoves = 0;
	U64 BBcopy = 0;
	int PSTvalO = 0;
	int PSTvalE = 0;
	char piece = ROOK;
	int from = 0;
	BBcopy = pos->pieces[ROOK] & BBsidepieces;
	while(BBcopy)
	{
		from = __builtin_ctzll(BBcopy);
		U64 BBnewmoves = Rmagic(from, BBoccupied) & BBallowed;
		BBmoves |= BBnewmoves;
		U64 BBkzattacks = BBnewmoves & BBkingzone;
		kzattacks += __builtin_popcountll(BBkzattacks);
		if (BBkzattacks) kzattackers++;
		PSTvalO += PSTval(side, piece,from,'O');
		PSTvalE += PSTval(side, piece,from,'E');
		BBcopy &= BBcopy-1;
	}
	struct mobreturn returnstruct;
	returnstruct.kingattacks = kzattacks;
	returnstruct.mobility = __builtin_popcountll(BBmoves);
	returnstruct.kingattackers = kzattackers;
	returnstruct.pstO = PSTvalO;
	returnstruct.pstE = PSTvalE;
	returnstruct.unsafe = __builtin_popcountll(BBmoves & BBattackedbypawns);
	return returnstruct;
}
struct mobreturn Qmobility(struct position *pos, int side) {
	U64 BBsidepieces;
	U64 BBkingzone;
	U64 BBkingonly;
	U64 BBattackedbypawns;
	if (side == WHITE) {
		BBsidepieces = pos->colours[WHITE];
		BBkingonly = pos->pieces[KING] & pos->colours[BLACK];
		BBkingzone = BBpawnshieldLookup[BLACK][pos->Bkingpos];
		BBattackedbypawns = BBpawnattacksB(pos->pieces[PAWN] & pos->colours[BLACK]);
	}
	else {
		BBsidepieces = pos->colours[BLACK];
		BBkingonly = pos->pieces[KING] & pos->colours[WHITE];
		BBkingzone = BBpawnshieldLookup[WHITE][pos->Wkingpos];
		BBattackedbypawns = BBpawnattacksW(pos->pieces[PAWN] & pos->colours[WHITE]);
	}
	int kzattackers = 0;
	int kzattacks = 0;
	U64 BBallowed = ~BBsidepieces;
	U64 BBoccupied = pos->colours[WHITE] | pos->colours[BLACK];
	U64 BBmoves = 0;
	U64 BBcopy = 0;
	int PSTvalO = 0;
	int PSTvalE = 0;
	char piece = QUEEN;
	int from = 0;
	BBcopy = pos->pieces[QUEEN] & BBsidepieces;
	while(BBcopy)
	{
		from = __builtin_ctzll(BBcopy);
		U64 BBnewmoves = (Rmagic(from, BBoccupied) | Bmagic(from, BBoccupied)) & BBallowed;
		BBmoves |= BBnewmoves;
		U64 BBkzattacks = BBnewmoves & BBkingzone;
		kzattacks += __builtin_popcountll(BBkzattacks);
		if (BBkzattacks) kzattackers++;
		PSTvalO += PSTval(side, piece,from,'O');
		PSTvalE += PSTval(side, piece,from,'E');
		BBcopy &= BBcopy-1;
	}
	struct mobreturn returnstruct;
	returnstruct.kingattacks = kzattacks;
	returnstruct.mobility = __builtin_popcountll(BBmoves);
	returnstruct.kingattackers = kzattackers;
	returnstruct.pstO = PSTvalO;
	returnstruct.pstE = PSTvalE;
	returnstruct.unsafe = __builtin_popcountll(BBmoves & BBattackedbypawns);
	return returnstruct;
}

int isEndgame(struct position *pos) {
	int numpieces = 1;
	U64 BBpieces = pos->pieces[KNIGHT] | pos->pieces[BISHOP] | pos->pieces[ROOK] | pos->pieces[QUEEN];
	if (pos->tomove == WHITE) BBpieces = BBpieces & pos->colours[WHITE];
	else BBpieces = BBpieces & pos->colours[BLACK];
	numpieces = __builtin_popcountll(BBpieces);
	if (numpieces > 3) return 0;
	return 1;
	//BBoccupied = pos->colours[WHITE] | pos->colours[BLACK];
	/*
	U64 BBoccupied = pos->colours[WHITE];
	if (pos->tomove == BLACK) BBoccupied = pos->colours[BLACK];
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
	
	int num_BP = __builtin_popcountll(pos->colours[BLACK] & pos->pieces[PAWN]);
	int num_BN = __builtin_popcountll(pos->colours[BLACK] & pos->pieces[KNIGHT]);
	int num_BB = __builtin_popcountll(pos->colours[BLACK] & pos->pieces[BISHOP]);
	int num_BR = __builtin_popcountll(pos->colours[BLACK] & pos->pieces[ROOK]);
	int num_BQ = __builtin_popcountll(pos->colours[BLACK] & pos->pieces[QUEEN]);
	int num_WP = __builtin_popcountll(pos->colours[WHITE] & pos->pieces[PAWN]);
	int num_WN = __builtin_popcountll(pos->colours[WHITE] & pos->pieces[KNIGHT]);
	int num_WB = __builtin_popcountll(pos->colours[WHITE] & pos->pieces[BISHOP]);
	int num_WR = __builtin_popcountll(pos->colours[WHITE] & pos->pieces[ROOK]);
	int num_WQ = __builtin_popcountll(pos->colours[WHITE] & pos->pieces[QUEEN]);
	
	// Piece values
    int white_pieces = num_WP*pieceval(PAWN)   +
                       num_WN*pieceval(KNIGHT) +
                       num_WB*pieceval(BISHOP) +
                       num_WR*pieceval(ROOK)   +
                       num_WQ*pieceval(QUEEN);

    int black_pieces = num_BP*pieceval(PAWN)   +
                       num_BN*pieceval(KNIGHT) +
                       num_BB*pieceval(BISHOP) +
                       num_BR*pieceval(ROOK)   +
                       num_BQ*pieceval(QUEEN);

	int material = white_pieces - black_pieces;
	int score = material;
	/*
	U64 BBoccupied = (pos->colours[WHITE] | pos->colours[BLACK]);
	while (BBoccupied != 0) {
		int square = __builtin_ctzll(BBoccupied);
		char piece = getPiece(pos,square);
		int col = getColour(pos, square);
		//BBoccupied &= ~(1ULL << square);
		BBoccupied &= BBoccupied - 1;
		int pstscoreO = PSTval(col, piece,square, 'O');
		int pstscoreE = PSTval(col, piece,square,'E');
		int pval = (pstscoreO + pstscoreE) / 2;
		//if ((piece >= 'a') && (piece <= 'z')) {
		//	pval = -pval;
		//}
		score += pval;
	}
	 */
	if (pos->tomove == BLACK) return -score;
	else return score;
	
}