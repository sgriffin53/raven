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

int piecevals[6] = { 110, 300, 300, 525, 900, 9999 };

int pieceval(const int inpiece) {
	return piecevals[inpiece];
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
	U64 BBoccupied = (pos->colours[WHITE] | pos->colours[BLACK]);
	
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

	while (BBoccupied != 0) {
		int square = __builtin_ctzll(BBoccupied);
		//BBoccupied &= ~(1ULL << square);
		BBoccupied &= BBoccupied - 1;
		int piece = getPiece(pos,square);
		int piececol = getPieceCol(pos, square);

		pstvalO = PSTval(piece, piececol, square,'O');
		pstvalE = PSTval(piece, piececol, square,'E');
		openingEval += pstvalO;
		endgameEval += pstvalE;
	}
	
	//printf("after pst: %d %d\n", openingEval, endgameEval);
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
	//U64 BBkingdist1 = BBkingattacks(pos->pieces[KING] & (1ULL << enemykingpos)); // fill 1 square away
	U64 BBkingdist1 = BBkingLookup[enemykingpos];
	U64 BBattackers = BBkingdist1 & (pos->colours[WHITE] & (pos->pieces[QUEEN] | pos->pieces[ROOK] | pos->pieces[KNIGHT] | pos->pieces[PAWN]));

	openingEval += 20 * __builtin_popcountll(BBattackers);
	endgameEval += 20 * __builtin_popcountll(BBattackers);
	U64 BBkingdist2 = BBkingfillLookup2[enemykingpos]; // fill 2 squares away
	BBkingdist2 = BBkingdist2 & ~(BBkingdist1);
	BBattackers = BBkingdist2 & (pos->colours[WHITE] & (pos->pieces[ROOK] | pos->pieces[QUEEN] | pos->pieces[KNIGHT] | pos->pieces[PAWN]));

	endgameEval += 40 * __builtin_popcountll(BBattackers);
	U64 BBkingdist3 = BBkingfillLookup3[enemykingpos]; // fill 3 squares away
	BBkingdist3 = BBkingdist3 & ~(BBkingdist2);
	BBattackers = BBkingdist3 & (pos->colours[WHITE] & (pos->pieces[ROOK] | pos->pieces[QUEEN] | pos->pieces[KNIGHT] | pos->pieces[PAWN]));

	openingEval += 10 * __builtin_popcountll(BBattackers);
	endgameEval += 10 * __builtin_popcountll(BBattackers);
	U64 BBkingdist4 = BBkingfillLookup4[enemykingpos]; // fill 4 squares away
	BBkingdist4 = BBkingdist4 & ~(BBkingdist3);
	BBattackers = BBkingdist4 & (pos->colours[WHITE] & (pos->pieces[ROOK] | pos->pieces[QUEEN] | pos->pieces[KNIGHT] | pos->pieces[PAWN]));


	openingEval += 10 * __builtin_popcountll(BBattackers);
	endgameEval += 10 * __builtin_popcountll(BBattackers);
	// black pieces attacking white king
	enemykingpos = pos->Wkingpos;
	//BBkingdist1 = BBkingattacks(pos->pieces[KING] & (1ULL << enemykingpos)); // fill 1 square away
	BBkingdist1 = BBkingLookup[enemykingpos];
	BBattackers = BBkingdist1 & (pos->colours[BLACK] & (pos->pieces[QUEEN] | pos->pieces[ROOK] | pos->pieces[KNIGHT] | pos->pieces[PAWN]));

	openingEval -= 20 * __builtin_popcountll(BBattackers);
	endgameEval -= 20 * __builtin_popcountll(BBattackers);
	
	BBkingdist2 = BBkingfillLookup2[enemykingpos]; // fill 2 squares away
	BBkingdist2 = BBkingdist2 & ~(BBkingdist1);
	BBattackers = BBkingdist2 & (pos->colours[BLACK] & (pos->pieces[ROOK] | pos->pieces[QUEEN] | pos->pieces[KNIGHT] | pos->pieces[PAWN]));

	endgameEval -= 40 * __builtin_popcountll(BBattackers);
	BBkingdist3 = BBkingfillLookup3[enemykingpos]; // fill 3 squares away
	BBkingdist3 = BBkingdist3 & ~(BBkingdist2);
	BBattackers = BBkingdist3 & (pos->colours[BLACK] & (pos->pieces[ROOK] | pos->pieces[QUEEN] | pos->pieces[KNIGHT] | pos->pieces[PAWN]));

	openingEval -= 10 * __builtin_popcountll(BBattackers);
	endgameEval -= 10 * __builtin_popcountll(BBattackers);
	
	BBkingdist4 = BBkingfillLookup4[enemykingpos]; // fill 4 squares away
	BBkingdist4 = BBkingdist4 & ~(BBkingdist3);
	BBattackers = BBkingdist4 & (pos->colours[BLACK] & (pos->pieces[ROOK] | pos->pieces[QUEEN] | pos->pieces[KNIGHT] | pos->pieces[PAWN]));

	openingEval -= 10 * __builtin_popcountll(BBattackers);
	endgameEval -= 10 * __builtin_popcountll(BBattackers);
	
	
	//printf("after king attacks: %d %d\n", openingEval, endgameEval);
	
	
	// passed pawns
	
	//int WpassedRankBonus[8] = {0,10,40,50,60,80,100,0};
	//int BpassedRankBonus[8] = {0,100,80,60,50,40,10,0};
	

	int WpassedRankBonus[8] = {0, 10, 10, 15, 25, 80, 120, 0};
	int BpassedRankBonus[8] = {0, 120, 80, 25, 15, 10, 10, 0};
	
	U64 BBwhitePP = 0ULL;
	U64 BBblackPP = 0ULL;
	
	U64 BBwhitepawns = (pos->colours[WHITE] & pos->pieces[PAWN]);
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
			BBenemypawns = (BBchecksquares & (pos->colours[BLACK] & pos->pieces[PAWN]));
			if (BBenemypawns) break;
			rank++;
		}
		 */
		U64 BBpiece = (1ULL << square);
		int startrank = getrank(square);
		U64 BBenemypawns = BBpasserLookup[WHITE][square] & (pos->colours[BLACK] & pos->pieces[PAWN]);
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
			
			U64 BBwhiterooks = pos->colours[WHITE] & pos->pieces[ROOK];
			U64 BBblackrooks = pos->colours[BLACK] & pos->pieces[ROOK];
			
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
			if (BBislight) BBsamecolbishops = pos->pieces[BISHOP] & pos->colours[WHITE] & BBlightsquares;
			else BBsamecolbishops = pos->pieces[BISHOP] & pos->colours[WHITE] & BBdarksquares;
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
	
	U64 BBblackpawns = (pos->colours[BLACK] & pos->pieces[PAWN]);
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
			BBenemypawns = (BBchecksquares & (pos->colours[WHITE] & pos->pieces[PAWN]));
			if (BBenemypawns) break;
			rank--;
		}
		 */
		U64 BBpiece = (1ULL << square);
		int startrank = getrank(square);
		U64 BBenemypawns = (BBpasserLookup[BLACK][square] & (pos->colours[WHITE] & pos->pieces[PAWN]));
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
			
			U64 BBwhiterooks = pos->colours[WHITE] & pos->pieces[ROOK];
			U64 BBblackrooks = pos->colours[BLACK] & pos->pieces[ROOK];
			
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
			if (islight) BBsamecolbishops = pos->pieces[BISHOP] & pos->colours[BLACK] & BBlightsquares;
			else BBsamecolbishops = pos->pieces[BISHOP] & pos->colours[BLACK] & BBdarksquares;
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
	
	//printf("after passers: %d %d\n", openingEval, endgameEval);
	// bonus for pawns attacking enemy pieces
	
	// white
	
	/*
	 
	BBwhitepawns = pos->pieces[PAWN] & pos->colours[WHITE];
	U64 BBpawnattacks = BBpawnattacksW(BBwhitepawns);
	U64 BBBpieces = pos->colours[BLACK] & (pos->pieces[KNIGHT] | pos->pieces[BISHOP] | pos->pieces[ROOK] | pos->pieces[QUEEN]);
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
	
	BBblackpawns = pos->pieces[PAWN] & pos->colours[BLACK];
	BBpawnattacks = BBpawnattacksB(BBblackpawns);
	U64 BBWpieces = pos->colours[WHITE] & (pos->pieces[KNIGHT] | pos->pieces[BISHOP] | pos->pieces[ROOK] | pos->pieces[QUEEN]);
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
	
	//printf("after free passers: %d %d\n", openingEval, endgameEval);
	
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
	
	// white
	/*
	BBwhitepawns = pos->colours[WHITE] & pos->pieces[PAWN];
	BBblackpawns = pos->colours[BLACK] & pos->pieces[PAWN];
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
	BBwhitepawns = pos->colours[WHITE] & pos->pieces[PAWN];
	while (BBwhitepawns) {
		int square = __builtin_ctzll(BBwhitepawns);
		BBwhitepawns &= ~(1ULL << square);
		int file = getfile(square);
		U64 BBfilemask = BBfileA << file;
		U64 BBBpawnsonfile = (pos->colours[BLACK] & pos->pieces[PAWN] & BBfilemask);
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
				if (pos->colours[BLACK] & pos->pieces[PAWN] & BBnw) sentries++;
				if (pos->colours[BLACK] & pos->pieces[PAWN] & BBne) sentries++;
				if (pos->colours[WHITE] & pos->pieces[PAWN] & BBnw) helpers++;
				if (pos->colours[WHITE] & pos->pieces[PAWN] & BBne) helpers++;
			}
			if (helpers > sentries) {
				openingEval += 10;
				endgameEval += 30;
			}
		}
	}
	
	// black
	
	BBblackpawns = pos->colours[BLACK] & pos->pieces[PAWN];
	while (BBblackpawns) {
		int square = __builtin_ctzll(BBblackpawns);
		BBblackpawns &= ~(1ULL << square);
		int file = getfile(square);
		U64 BBfilemask = BBfileA << file;
		U64 BBWpawnsonfile = (pos->colours[WHITE] & pos->pieces[PAWN] & BBfilemask);
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
				if (pos->colours[WHITE] & pos->pieces[PAWN] & BBnw) sentries++;
				if (pos->colours[WHITE] & pos->pieces[PAWN] & BBne) sentries++;
				if (pos->colours[BLACK] & pos->pieces[PAWN] & BBnw) helpers++;
				if (pos->colours[BLACK] & pos->pieces[PAWN] & BBne) helpers++;
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
	openingEval -= 10 * __builtin_popcountll((BBwhitePP<<8) & (pos->colours[WHITE] | pos->colours[BLACK]));
	endgameEval -= 20 * __builtin_popcountll((BBwhitePP<<8) & (pos->colours[WHITE] | pos->colours[BLACK]));
	
	// black
	openingEval += 10 * __builtin_popcountll((BBblackPP>>8) & (pos->colours[WHITE] | pos->colours[BLACK]));
	endgameEval += 20 * __builtin_popcountll((BBblackPP>>8) & (pos->colours[WHITE] | pos->colours[BLACK]));
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
	U64 BBwhitenonBNmaterial = (pos->pieces[PAWN] | pos->pieces[QUEEN] | pos->pieces[ROOK]) & pos->colours[WHITE];
	U64 BBblackmaterial = (pos->pieces[PAWN] | pos->pieces[QUEEN] | pos->pieces[BISHOP] | pos->pieces[KNIGHT] | pos->pieces[ROOK]) & pos->colours[BLACK];
	
	if (!BBwhitenonBNmaterial && !BBblackmaterial && num_WB == 1 && num_WN == 1) {
		// KBN vs K endgame, white has the B+N
		// penalty for king going into wrong corner, bonus for king going into right corner
		int islight = pos->pieces[BISHOP] & pos->colours[WHITE] & BBlightsquares;
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
	U64 BBblacknonBNmaterial = (pos->pieces[PAWN] | pos->pieces[QUEEN] | pos->pieces[ROOK]) & pos->colours[BLACK];
	U64 BBwhitematerial = (pos->pieces[PAWN] | pos->pieces[QUEEN] | pos->pieces[BISHOP] | pos->pieces[KNIGHT] | pos->pieces[ROOK]) & pos->colours[WHITE];
	if (!BBblacknonBNmaterial && !BBwhitematerial && num_BB == 1 && num_BN == 1) {
		// KBN vs K endgame, white has the B+N
		// penalty for king going into wrong corner, bonus for king going into right corner
		int islight = pos->pieces[BISHOP] & pos->colours[BLACK] & BBlightsquares;
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
	
	U64 BBwhitenonrookmaterial = (pos->pieces[PAWN] | pos->pieces[QUEEN] | pos->pieces[BISHOP] | pos->pieces[KNIGHT]) & pos->colours[WHITE];
	U64 BBblackmaterial = (pos->pieces[PAWN] | pos->pieces[QUEEN] | pos->pieces[BISHOP] | pos->pieces[KNIGHT] | pos->pieces[ROOK]) & pos->colours[BLACK];
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
	
	//printf("after mop up: %d %d\n", openingEval, endgameEval);
	
	// bonus for rooks trapping kings on the edge
	/*
	int kingonedgebonus = 15;
	if (pos->tomove == WHITE) {
		int enemykingpos = pos->Bkingpos;
		int rank = getrank(enemykingpos);
		int file = getfile(enemykingpos);
		if (rank == 7) { // 8th rank
			if (!(pos->pieces[PAWN] & BBrank7) && (pos->pieces[ROOK] & pos->colours[WHITE] & BBrank7)) { // no pawns on 7th rank, white rook(s) on 7th rank
				endgameEval += kingonedgebonus;
			}
		}
		else if (rank == 0) { // 1st rank
			if (!(pos->pieces[PAWN] & BBrank1) && (pos->pieces[ROOK] & pos->colours[WHITE] & BBrank1)) { // no pawns on 1st rank, white rook(s) on 1st rank
				endgameEval += kingonedgebonus;
			}
		}
		if (file == 0) { // A file
			if ((!pos->pieces[PAWN] & BBfileB) && (pos->pieces[ROOK] & pos->colours[WHITE] & BBfileB)) {
				endgameEval += kingonedgebonus;
			}
		}
		else if (file == 7) { // H file
			if ((!pos->pieces[PAWN] & BBfileG) && (pos->pieces[ROOK] & pos->colours[WHITE] & BBfileG)) {
				endgameEval += kingonedgebonus;
			}
		}
	}
	 
	else if (pos->tomove == BLACK) {
		int enemykingpos = pos->Wkingpos;
		int rank = getrank(enemykingpos);
		int file = getfile(enemykingpos);
		if (rank == 7) { // 8th rank
			if (!(pos->pieces[PAWN] & BBrank7) && (pos->pieces[ROOK] & pos->colours[BLACK] & BBrank7)) { // no pawns on 7th rank, black rook(s) on 7th rank
				endgameEval -= kingonedgebonus;
			}
		}
		else if (rank == 0) { // 1st rank
			if (!(pos->pieces[PAWN] & BBrank1) && (pos->pieces[ROOK] & pos->colours[BLACK] & BBrank1)) { // no pawns on 1st rank, black rook(s) on 1st rank
				endgameEval -= kingonedgebonus;
			}
		}
		if (file == 0) { // A file
			if ((!pos->pieces[PAWN] & BBfileB) && (pos->pieces[ROOK] & pos->colours[BLACK] & BBfileB)) {
				endgameEval -= kingonedgebonus;
			}
		}
		else if (file == 7) { // H file
			if ((!pos->pieces[PAWN] & BBfileG) && (pos->pieces[ROOK] & pos->colours[BLACK] & BBfileG)) {
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
		
		//U64 BBallpawnsonfile = BBfilemask & pos->pieces[PAWN];
		//U64 BBallrooksonfile = BBfilemask & pos->pieces[ROOK];
		//if (!BBallpawnsonfile && !BBallrooksonfile) continue;
		
		U64 BBWpawnsonfile = BBfilemask & (pos->colours[WHITE] & pos->pieces[PAWN]);
		
		U64 BBisdoubled = BBWpawnsonfile & (BBWpawnsonfile-1);
		if (BBisdoubled) {
			openingEval -= 16;
			endgameEval -= 16;
		}
		// black pawns
		U64 BBBpawnsonfile = BBfilemask & (pos->colours[BLACK] & pos->pieces[PAWN]);
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
			U64 BBleftpawns = westOne(BBfilemask) & (pos->colours[WHITE] & pos->pieces[PAWN]);
			U64 BBrightpawns = eastOne(BBfilemask) & (pos->colours[WHITE] & pos->pieces[PAWN]);
			if (BBleftpawns == 0 && BBrightpawns == 0) {
				openingEval -= 6;
				endgameEval -= 6;
			}
		}
		if (BBBpawnsonfile) {
			U64 BBleftpawns = westOne(BBfilemask) & (pos->colours[BLACK] & pos->pieces[PAWN]);
			U64 BBrightpawns = eastOne(BBfilemask) & (pos->colours[BLACK] & pos->pieces[PAWN]);
			if (BBleftpawns == 0 && BBrightpawns == 0) {
				openingEval += 6;
				endgameEval += 6;
			}
		}
		//if (!BBallrooksonfile) continue;
		// rooks on open files
		U64 BBpawnsonfile = BBfilemask & pos->pieces[PAWN];
		// white rook on open file
		U64 BBWrooksonfile = BBfilemask & (pos->pieces[ROOK] & pos->colours[WHITE]);
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
		U64 BBBrooksonfile = BBfilemask & (pos->pieces[ROOK] & pos->colours[BLACK]);
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
		U64 BBWqueensonfile = BBfilemask & (pos->pieces[QUEEN] & pos->colours[WHITE]);
		U64 BBBqueensonfile = BBfilemask & (pos->pieces[QUEEN] & pos->colours[BLACK]);
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
	//printf("after pawn rook stuff: %d %d\n", openingEval, endgameEval);
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
	BBpawnshield &= (pos->colours[WHITE] & pos->pieces[PAWN]);
	openingEval += 30 * __builtin_popcountll(BBpawnshield);
	
	// black pawn shield
	
	int Bkingpos = pos->Bkingpos;
	//BBkingdist1 = BBkingattacks((1ULL << Bkingpos)); // fill 1 square away
	//BBkingdist2 = BBkingattacks((1ULL << Bkingpos >> 8)); // fill 3 squares sw, s, se of king zone
	//BBpawnshield = BBkingdist2 & (pos->colours[BLACK] & pos->pieces[PAWN]);
	/*
	BBpawnshield = soWeOne(1ULL << Bkingpos) | southOne(1ULL << Bkingpos) | soEaOne(1ULL << Bkingpos);
	BBpawnshield |= southOne(BBpawnshield);
	 */
	BBpawnshield = BBpawnshieldLookup[BLACK][Bkingpos];
	BBpawnshield &= (pos->colours[BLACK] & pos->pieces[PAWN]);
	openingEval -= 30 * __builtin_popcountll(BBpawnshield);
	
	//printf("after pawn shield: %d %d\n", openingEval, endgameEval);
	
	// king safety - bonus for friendly pieces around the king
	/*
	// white
	
	BBkingdist1 = BBkingattacks(1ULL << pos->Wkingpos); // fill 1 square away
	U64 BBfriendlypieces = (BBkingdist1 & (pos->colours[WHITE] & ~pos->pieces[PAWN]));
	openingEval += 5 * __builtin_popcountll(BBfriendlypieces);
	
	// black
	
	BBkingdist1 = BBkingattacks(1ULL << pos->Bkingpos); // fill 1 square away
	BBfriendlypieces = (BBkingdist1 & (pos->colours[BLACK] & ~pos->pieces[PAWN]));
	openingEval -= 5 * __builtin_popcountll(BBfriendlypieces);
	*/
	
	// trapped pieces
	
	// white
	//U64 BBwhitebishops = pos->colours[WHITE] & pos->pieces[BISHOP];
	
	/*
	U64 BBwhitematpieces = pos->colours[WHITE] & (pos->pieces[BISHOP] | pos->pieces[KNIGHT] | pos->pieces[ROOK] | pos->pieces[QUEEN]);
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
			BBmoves = BBknightattacks(1ULL << square) & ~(pos->colours[WHITE]);
		}
		if (piece == 'B') {
			BBmoves = Bmagic(square, pos->colours[WHITE] | pos->colours[BLACK]) & ~(pos->colours[WHITE]);
		}
		if (piece == 'R') {
			BBmoves = Rmagic(square, pos->colours[WHITE] | pos->colours[BLACK]) & ~(pos->colours[WHITE]);
		}
		if (piece == 'Q') {
			BBmoves = (Rmagic(square, pos->colours[WHITE] | pos->colours[BLACK])
						| Bmagic(square, pos->colours[WHITE] | pos->colours[BLACK])) & ~(pos->colours[WHITE]);
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
	U64 BBblackmatpieces = pos->colours[BLACK] & (pos->pieces[BISHOP] | pos->pieces[KNIGHT] | pos->pieces[ROOK] | pos->pieces[QUEEN]);
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
			BBmoves = BBknightattacks(1ULL << square) & ~(pos->colours[BLACK]);
		}
		if (piece == 'b') {
			BBmoves = Bmagic(square, pos->colours[WHITE] | pos->colours[BLACK]) & ~(pos->colours[BLACK]);
		}
		if (piece == 'r') {
			BBmoves = Rmagic(square, pos->colours[WHITE] | pos->colours[BLACK]) & ~(pos->colours[BLACK]);
		}
		if (piece == 'q') {
			BBmoves = (Rmagic(square, pos->colours[WHITE] | pos->colours[BLACK])
						| Bmagic(square, pos->colours[WHITE] | pos->colours[BLACK])) & ~(pos->colours[BLACK]);
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
		U64 BBbishops = pos->pieces[BISHOP] & pos->colours[WHITE];
		while (BBbishops) {
			int square = __builtin_ctzll(BBbishops);
			BBbishops &= BBbishops - 1;
			int islight = 0;
			if ((1ULL << square) & BBlightsquares) islight = 1;
			else islight = 0;
			U64 BBsamecolpawns;
			if (islight) BBsamecolpawns = pos->pieces[PAWN] & pos->colours[WHITE] & BBlightsquares;
			else BBsamecolpawns = pos->pieces[PAWN] & pos->colours[WHITE] & BBdarksquares;
			openingEval -= 4 * __builtin_popcountll(BBsamecolpawns);
			endgameEval -= 6 * __builtin_popcountll(BBsamecolpawns);
		}
	}
	
	// black
	
	if (num_BB == 1) {
		U64 BBbishops = pos->pieces[BISHOP] & pos->colours[BLACK];
		while (BBbishops) {
			int square = __builtin_ctzll(BBbishops);
			BBbishops &= BBbishops - 1;
			int islight = 0;
			if ((1ULL << square) & BBlightsquares) islight = 1;
			else islight = 0;
			U64 BBsamecolpawns;
			if (islight) BBsamecolpawns = pos->pieces[PAWN] & pos->colours[BLACK] & BBlightsquares;
			else BBsamecolpawns = pos->pieces[PAWN] & pos->colours[BLACK] & BBdarksquares;
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
	
	U64 BBWpiecesincentre = (pos->colours[WHITE] & pos->pieces[PAWN] & BBcentre);
	openingEval += 20 * __builtin_popcountll(BBWpiecesincentre);
	endgameEval += 20 * __builtin_popcountll(BBWpiecesincentre);
	
	U64 BBBpiecesincentre = (pos->colours[BLACK] & pos->pieces[PAWN] & BBcentre);
	openingEval -= 20 * __builtin_popcountll(BBBpiecesincentre);
	endgameEval -= 20 * __builtin_popcountll(BBBpiecesincentre);
	
	// bonus for connected knights
	// white
	if (num_WN >= 2) {
		U64 BBWknights = (pos->colours[WHITE] & pos->pieces[KNIGHT]);
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
		U64 BBBknights = (pos->colours[BLACK] & pos->pieces[KNIGHT]);
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
	
	// penalty for pieces being attacked by enemy pawns
	
	// white
	/*
	U64 BBBpawns = pos->colours[BLACK] & pos->pieces[PAWN];
	U64 BBBpawnattacks = soEaOne(BBBpawns) | soWeOne(BBBpawns);
	U64 BBWpiecesattacked = (pos->colours[WHITE] & ~pos->pieces[PAWN]) & BBBpawnattacks;
	openingEval -= 10 * __builtin_popcountll(BBWpiecesattacked);
	endgameEval -= 10 * __builtin_popcountll(BBWpiecesattacked);
	// black
	
	U64 BBWpawns = pos->colours[WHITE] & pos->pieces[PAWN];
	U64 BBWpawnattacks = noEaOne(BBWpawns) | noWeOne(BBWpawns);
	U64 BBBpiecesattacked = (pos->colours[BLACK] & ~pos->pieces[PAWN]) & BBWpawnattacks;
	openingEval += 10 * __builtin_popcountll(BBBpiecesattacked);
	endgameEval += 10 * __builtin_popcountll(BBBpiecesattacked);
	*/
	
	// bonus for connected rooks
	/*
	// white
	if (num_WR >= 2) {
		U64 BBrooks = (pos->pieces[ROOK] & pos->colours[WHITE]);
		U64 BBrooksstart = BBrooks;
		U64 BBoccupancy = (pos->colours[WHITE] | pos->colours[BLACK]);
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
		U64 BBrooks = (pos->pieces[ROOK] & pos->colours[BLACK]);
		U64 BBrooksstart = BBrooks;
		U64 BBoccupancy = (pos->colours[WHITE] | pos->colours[BLACK]);
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
	
	U64 BBblackknights = (pos->colours[BLACK] & pos->pieces[KNIGHT]);
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
	
	// bonus for bishops on long diagonal that attack both centre squares
	/*
	BBwhitebishops = pos->colours[WHITE] & pos->pieces[BISHOP];
	BBblackbishops = pos->colours[BLACK] & pos->pieces[BISHOP];
	while (BBwhitebishops) {
		int square = __builtin_ctzll(BBwhitebishops);
		BBwhitebishops &= BBwhitebishops - 1;
		if ((1ULL << square) & (BBdiagA1H8 | BBdiagA8H1)) {
			// bishop is on long diagonal
			U64 BBmoves = Bmagic(square, pos->colours[WHITE] | pos->colours[BLACK]) & ~pos->colours[WHITE];
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
			U64 BBmoves = Bmagic(square, pos->colours[WHITE] | pos->colours[BLACK]) & ~pos->colours[BLACK];
			if (__builtin_popcountll(BBmoves & BBcentre) == 2) {
				// bishop can see both centre squares on diagonal
				openingEval -= 45;
			}
		}
	}
	*/
	
	// colour weaknesses
	
	// white
	/*
	if ((pos->colours[BLACK] & pos->pieces[BISHOP] & BBlightsquares)
		&& !(pos->colours[WHITE] & pos->pieces[BISHOP] & BBlightsquares)) {
		// black has a light square bishop
		// white doesn't have a light square bishop
		U64 BBWdarkpawns = pos->colours[WHITE] & pos->pieces[PAWN] & BBdarksquares;
		int numbadpawns = __builtin_popcountll(BBWdarkpawns);
		// penalty for each pawn on a dark square
		openingEval -= numbadpawns * 3;
		endgameEval -= numbadpawns * 6;
	}
	if ((pos->colours[BLACK] & pos->pieces[BISHOP] & BBdarksquares) 
		&& !(pos->colours[WHITE] & pos->pieces[BISHOP] & BBdarksquares)) {
		// black has a dark square bishop
		// white doesn't have a dark square bishop
		U64 BBWlightpawns = pos->colours[WHITE] & pos->pieces[PAWN] & BBlightsquares;
		int numbadpawns = __builtin_popcountll(BBWlightpawns);
		// penalty for each pawn on a light square
		openingEval -= numbadpawns * 3;
		endgameEval -= numbadpawns * 6;
	}
	
	// black
	if ((pos->colours[WHITE] & pos->pieces[BISHOP] & BBlightsquares) 
		&& !(pos->colours[BLACK] & pos->pieces[BISHOP] & BBlightsquares)) {
		// white has a light square bishop
		// black doesn't have a light square bishop
		U64 BBBdarkpawns = pos->colours[BLACK] & pos->pieces[PAWN] & BBdarksquares;
		int numbadpawns = __builtin_popcountll(BBBdarkpawns);
		// penalty for each pawn on a dark square
		openingEval += numbadpawns * 3;
		endgameEval += numbadpawns * 6;
	}
	if ((pos->colours[WHITE] & pos->pieces[BISHOP] & BBdarksquares) 
		&& !(pos->colours[BLACK] & pos->pieces[BISHOP] & BBdarksquares)) {
		// white has a dark square bishop
		// black doesn't have a dark square bishop
		U64 BBBlightpawns = pos->colours[BLACK] & pos->pieces[PAWN] & BBlightsquares;
		int numbadpawns = __builtin_popcountll(BBBlightpawns);
		// penalty for each pawn on a light square
		openingEval += numbadpawns * 3;
		endgameEval += numbadpawns * 6;
	}
	*/
	
	// bishops get stronger if there are pawns on both flanks in bishop vs knight endgames
	/*
	U64 BBABCpawns = (BBfileA | BBfileB | BBfileC) & pos->pieces[PAWN];
	U64 BBFGHpawns = (BBfileF | BBfileG | BBfileH) & pos->pieces[PAWN];
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
	
	
	// rook on the 7th
	
	// white
	/*
	U64 BBWrookson7th = BBrank7 & pos->pieces[ROOK] & pos->colours[WHITE];
	U64 BBBkingon8th = BBrank8 & pos->pieces[KING] & pos->colours[BLACK];
	if (BBWrookson7th & BBBkingon8th) {
		openingEval += 30 * __builtin_popcountll(BBWrookson7th);
		endgameEval += 30 * __builtin_popcountll(BBWrookson7th);
	}
	
	// black
	
	U64 BBBrookson7th = BBrank2 & pos->pieces[ROOK] & pos->colours[BLACK];
	U64 BBWkingon8th = BBrank1 & pos->pieces[KING] & pos->colours[WHITE];
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
	 
	// white
	
	int WNmobility = Nmobility(pos,WHITE);
	openingEval += knightMgMobility[WNmobility];
	endgameEval += knightEgMobility[WNmobility];
	int WBmobility = Bmobility(pos,WHITE);
	openingEval += bishopMgMobility[WBmobility];
	endgameEval += bishopEgMobility[WBmobility];
	int WRmobility = Rmobility(pos,WHITE);
	openingEval += rookMgMobility[WRmobility];
	endgameEval += rookEgMobility[WRmobility];
	int WQmobility = Qmobility(pos,WHITE);
	openingEval += queenMgMobility[WQmobility];
	endgameEval += queenEgMobility[WQmobility];	
	
	// black
	int BNmobility = Nmobility(pos,BLACK);
	openingEval -= knightMgMobility[BNmobility];
	endgameEval -= knightEgMobility[BNmobility];
	int BBmobility = Bmobility(pos,BLACK);
	openingEval -= bishopMgMobility[BBmobility];
	endgameEval -= bishopEgMobility[BBmobility];
	int BRmobility = Rmobility(pos,BLACK);
	openingEval -= rookMgMobility[WBmobility];
	endgameEval -= rookEgMobility[WBmobility];
	int BQmobility = Qmobility(pos,BLACK);
	openingEval -= queenMgMobility[BQmobility];
	endgameEval -= queenEgMobility[BQmobility];	
	
	// bonus for knights in closed positions, bishops in open positions
	/*
	U64 BBrammed = northOne(pos->pieces[PAWN] & pos->colours[WHITE]) & (pos->colours[BLACK] & pos->pieces[PAWN]);
	
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
	U64 BBrammedB = northOne(pos->pieces[PAWN] & pos->colours[WHITE]) & (pos->colours[BLACK] & pos->pieces[PAWN]);
	U64 BBrammedW = southOne(pos->pieces[PAWN] & pos->colours[BLACK]) & (pos->colours[WHITE] & pos->pieces[PAWN]);
	
	U64 BBWbishops = pos->colours[WHITE] & pos->pieces[BISHOP];
	U64 BBBbishops = pos->colours[BLACK] & pos->pieces[BISHOP];
	
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

	// penalty for pieces attacking king zone
	/*
	// white
	int numattackers = 0;
	int attacksvalue = 0;
	U64 BBblackpieces = pos->colours[BLACK] & (pos->pieces[QUEEN] | pos->pieces[ROOK] | pos->pieces[BISHOP] | pos->pieces[KNIGHT]);
	while (BBblackpieces) {
		int square = __builtin_ctzll(BBblackpieces);
		BBblackpieces &= BBblackpieces - 1;
		char piece = getPiece(pos, square);
		U64 BBmoves;
		int weight = 0;
		if (piece == 'n') {
			BBmoves = BBknightattacks(1ULL << square) & ~(pos->colours[BLACK]);
			weight = 20;
		}
		if (piece == 'b') {
			BBmoves = Bmagic(square, pos->colours[WHITE] | pos->colours[BLACK]) & ~(pos->colours[BLACK]);
			weight = 20;
		}
		if (piece == 'r') {
			BBmoves = Rmagic(square, pos->colours[WHITE] | pos->colours[BLACK]) & ~(pos->colours[BLACK]);
			weight = 40;
		}
		if (piece == 'q') {
			BBmoves = (Rmagic(square, pos->colours[WHITE] | pos->colours[BLACK])
						| Bmagic(square, pos->colours[WHITE] | pos->colours[BLACK])) & ~(pos->colours[BLACK]);
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
	U64 BBwhitepieces = pos->colours[WHITE] & (pos->pieces[QUEEN] | pos->pieces[ROOK] | pos->pieces[BISHOP] | pos->pieces[KNIGHT]);
	while (BBwhitepieces) {
		int square = __builtin_ctzll(BBwhitepieces);
		BBwhitepieces &= BBwhitepieces - 1;
		char piece = getPiece(pos, square);
		U64 BBmoves;
		int weight = 0;
		if (piece == 'N') {
			BBmoves = BBknightattacks(1ULL << square) & ~(pos->colours[BLACK]);
			weight = 20;
		}
		if (piece == 'B') {
			BBmoves = Bmagic(square, pos->colours[WHITE] | pos->colours[BLACK]) & ~(pos->colours[WHITE]);
			weight = 20;
		}
		if (piece == 'R') {
			BBmoves = Rmagic(square, pos->colours[WHITE] | pos->colours[BLACK]) & ~(pos->colours[WHITE]);
			weight = 40;
		}
		if (piece == 'Q') {
			BBmoves = (Rmagic(square, pos->colours[WHITE] | pos->colours[BLACK])
						| Bmagic(square, pos->colours[WHITE] | pos->colours[BLACK])) & ~(pos->colours[WHITE]);
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
	openingEval += __builtin_popcountll(pos->colours[WHITE] & BBbigcentre) * 24;
	endgameEval += __builtin_popcountll(pos->colours[WHITE] & BBbigcentre) * 8;
	
	openingEval -= __builtin_popcountll(pos->colours[BLACK] & BBbigcentre) * 24;
	endgameEval -= __builtin_popcountll(pos->colours[BLACK] & BBbigcentre) * 8;
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
/*
int isTrappedPiece(struct position *pos, int side) {
	if (side == BLACK) {
		U64 BBblackmatpieces = pos->colours[BLACK] & (pos->pieces[BISHOP] | pos->pieces[KNIGHT] | pos->pieces[ROOK] | pos->pieces[QUEEN]);
		while (BBblackmatpieces) {
			int square = __builtin_ctzll(BBblackmatpieces);
			BBblackmatpieces &= BBblackmatpieces - 1;
			int piece = getPiece(pos, square);
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
				BBmoves = BBknightattacks(1ULL << square) & ~(pos->colours[BLACK]);
			}
			if (piece == 'b') {
				BBmoves = Bmagic(square, pos->colours[WHITE] | pos->colours[BLACK]) & ~(pos->colours[BLACK]);
			}
			if (piece == 'r') {
				BBmoves = Rmagic(square, pos->colours[WHITE] | pos->colours[BLACK]) & ~(pos->colours[BLACK]);
			}
			if (piece == 'q') {
				BBmoves = (Rmagic(square, pos->colours[WHITE] | pos->colours[BLACK])
							| Bmagic(square, pos->colours[WHITE] | pos->colours[BLACK])) & ~(pos->colours[BLACK]);
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
		U64 BBwhitematpieces = pos->colours[WHITE] & (pos->pieces[BISHOP] | pos->pieces[KNIGHT] | pos->pieces[ROOK] | pos->pieces[QUEEN]);
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
				BBmoves = BBknightattacks(1ULL << square) & ~(pos->colours[WHITE]);
			}
			if (piece == 'B') {
				BBmoves = Bmagic(square, pos->colours[WHITE] | pos->colours[BLACK]) & ~(pos->colours[WHITE]);
			}
			if (piece == 'R') {
				BBmoves = Rmagic(square, pos->colours[WHITE] | pos->colours[BLACK]) & ~(pos->colours[WHITE]);
			}
			if (piece == 'Q') {
				BBmoves = (Rmagic(square, pos->colours[WHITE] | pos->colours[BLACK])
							| Bmagic(square, pos->colours[WHITE] | pos->colours[BLACK])) & ~(pos->colours[WHITE]);
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
 */
int Nmobility(struct position *pos, int side) {
	U64 BBsidepieces;
	if (side == WHITE) BBsidepieces = pos->colours[WHITE];
	else BBsidepieces = pos->colours[BLACK];
	U64 BBallowed = ~BBsidepieces;
	U64 BBoccupied = pos->colours[WHITE] | pos->colours[BLACK];
	U64 BBmoves = 0;
	U64 BBcopy = 0;
	int from = 0;
	
	// Knights
	BBcopy = pos->pieces[KNIGHT] & BBsidepieces;
	while(BBcopy)
	{
		from = __builtin_ctzll(BBcopy);
		//BBmoves |= BBknightattacks(1ULL << from) & BBallowed;
		BBmoves |= BBknightLookup[from] & BBallowed;
		BBcopy &= BBcopy-1;
	}
	
	return __builtin_popcountll(BBmoves);
}
int Bmobility(struct position *pos, int side) {
	U64 BBsidepieces;
	if (side == WHITE) BBsidepieces = pos->colours[WHITE];
	else BBsidepieces = pos->colours[BLACK];
	U64 BBallowed = ~BBsidepieces;
	U64 BBoccupied = pos->colours[WHITE] | pos->colours[BLACK];
	U64 BBmoves = 0;
	U64 BBcopy = 0;
	int from = 0;
	BBcopy = pos->pieces[BISHOP] & BBsidepieces;
	while(BBcopy)
	{
		from = __builtin_ctzll(BBcopy);
		BBmoves |= Bmagic(from, BBoccupied) & BBallowed;
		BBcopy &= BBcopy-1;
	}
	return __builtin_popcountll(BBmoves);
}
int Rmobility(struct position *pos, int side) {
	U64 BBsidepieces;
	if (side == WHITE) BBsidepieces = pos->colours[WHITE];
	else BBsidepieces = pos->colours[BLACK];
	U64 BBallowed = ~BBsidepieces;
	U64 BBoccupied = pos->colours[WHITE] | pos->colours[BLACK];
	U64 BBmoves = 0;
	U64 BBcopy = 0;
	int from = 0;
	BBcopy = pos->pieces[ROOK] & BBsidepieces;
	while(BBcopy)
	{
		from = __builtin_ctzll(BBcopy);
		BBmoves |= Rmagic(from, BBoccupied) & BBallowed;
		BBcopy &= BBcopy-1;
	}
	return __builtin_popcountll(BBmoves);
}
int Qmobility(struct position *pos, int side) {
	U64 BBsidepieces;
	if (side == WHITE) BBsidepieces = pos->colours[WHITE];
	else BBsidepieces = pos->colours[BLACK];
	U64 BBallowed = ~BBsidepieces;
	U64 BBoccupied = pos->colours[WHITE] | pos->colours[BLACK];
	U64 BBmoves = 0;
	U64 BBcopy = 0;
	int from = 0;
	BBcopy = pos->pieces[QUEEN] & BBsidepieces;
	while(BBcopy)
	{
		from = __builtin_ctzll(BBcopy);
		BBmoves |= (Rmagic(from, BBoccupied) | Bmagic(from, BBoccupied)) & BBallowed;
		BBcopy &= BBcopy-1;
	}
	return __builtin_popcountll(BBmoves);
}
/*
int mobility(struct position *pos, int side) {
	U64 BBsidepieces;
	if (side == WHITE) BBsidepieces = pos->colours[WHITE];
	else BBsidepieces = pos->colours[BLACK];
	U64 BBallowed = ~BBsidepieces;
	U64 BBoccupied = pos->colours[WHITE] | pos->colours[BLACK];
	U64 BBmoves = 0;
	U64 BBcopy = 0;
	int from = 0;
	
	// Knights
	BBcopy = pos->pieces[KNIGHT] & BBsidepieces;
	while(BBcopy)
	{
		from = __builtin_ctzll(BBcopy);
		BBmoves |= BBknightattacks(1ULL << from) & BBallowed;
		BBcopy &= BBcopy-1;
	}

	// Bishops and Queens
	BBcopy = (pos->pieces[BISHOP] | pos->pieces[QUEEN]) & BBsidepieces;
	while(BBcopy)
	{
		from = __builtin_ctzll(BBcopy);
		BBmoves |= Bmagic(from, BBoccupied) & BBallowed;
		BBcopy &= BBcopy-1;
	}
	
	// Rooks and Queens
	BBcopy = (pos->pieces[ROOK] | pos->pieces[QUEEN]) & BBsidepieces;
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
/*
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
	U64 BBoccupied = (pos->colours[WHITE] | pos->colours[BLACK]);
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
 */
/*
int evalBoard(struct position *pos) {
	assert(pos);
	int score = 0;
	int pval;
	U64 BBoccupied = (pos->colours[WHITE] | pos->colours[BLACK]);
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