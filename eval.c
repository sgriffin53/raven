#include "eval.h"
#include "position.h"
#include "PST.h"
#include "attacks.h"
#include "bitboards.h"
#include "magicmoves.h"
#include "globals.h"
#include <stdio.h>
#include <stdlib.h>

int pieceval(const char inpiece) {
	if (inpiece == 'p') return 100;
	if (inpiece == 'n') return 300;
	if (inpiece == 'b') return 300;
	if (inpiece == 'r') return 500;
	if (inpiece == 'q') return 900;
	if (inpiece == 'k') return 9999;
	if (inpiece == 'P') return 100;
	if (inpiece == 'N') return 300;
	if (inpiece == 'B') return 300;
	if (inpiece == 'R') return 500;
	if (inpiece == 'Q') return 900;
	if (inpiece == 'K') return 9999;

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
		BBoccupied &= ~(1ULL << square);
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
		// bonus for N being near king
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
	
	
	U64 BBwhitepawns = (pos->BBwhitepieces & pos->BBpawns);
	while (BBwhitepawns) {
		
		// passed pawns
		
		int square = __builtin_ctzll(BBwhitepawns);
		BBwhitepawns &= ~(1ULL << square);
		U64 BBpiece = (1ULL << square);
		U64 BBmidsquare = BBpiece;
		U64 BBchecksquares = 0ULL;
		int startrank = getrank(square);
		int rank = startrank;
		//printf("\n%d\n",rank);
		while (rank < 6) {
			BBchecksquares |= noWeOne(BBmidsquare);
			BBchecksquares |= northOne(BBmidsquare);
			BBchecksquares |= noEaOne(BBmidsquare);
			BBmidsquare = northOne(BBmidsquare);
			rank++;
		}
		U64 BBenemypawns = (BBchecksquares & (pos->BBblackpieces & pos->BBpawns));
		if (BBenemypawns == 0) {
			int bonus = WpassedRankBonus[startrank];
			openingEval += 0.5 * bonus;
			endgameEval += 1 * bonus;
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
		BBblackpawns &= ~(1ULL << square);
		U64 BBpiece = (1ULL << square);
		U64 BBmidsquare = BBpiece;
		U64 BBchecksquares = 0ULL;
		int startrank = getrank(square);
		int rank = startrank;
		//printf("\n%d\n",rank);
		while (rank > 1) {
			BBchecksquares |= soWeOne(BBmidsquare);
			BBchecksquares |= southOne(BBmidsquare);
			BBchecksquares |= soEaOne(BBmidsquare);
			BBmidsquare = southOne(BBmidsquare);
			rank--;
		}
		U64 BBenemypawns = (BBchecksquares & (pos->BBwhitepieces & pos->BBpawns));
		if (BBenemypawns == 0) {
			int bonus = BpassedRankBonus[startrank];
			openingEval -= 0.5 * bonus;
			endgameEval -= 1 * bonus;
		}
		
		// pawn chain bonus
		
		// pawn chain bonus
		U64 BBpawnattacks = BBpawnEastAttacksW(BBpiece) | BBpawnWestAttacksW(BBpiece);
		if ((BBpawnattacks & BBblackpawns)) {
			openingEval -= 20;
			endgameEval -= 20;
		}
	}

	// loop to check for doubled pawns and rooks on open files
	
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
	BBkingdist1 = BBkingattacks(pos->BBkings & (1ULL << Wkingpos)); // fill 1 square away
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
	BBkingdist1 = BBkingattacks(pos->BBkings & (1ULL << Bkingpos)); // fill 1 square away
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

	if (num_BB >= 2) {
		openingEval -= 60;
		endgameEval -= 60;
	}
	if (num_WB >= 2) {
		openingEval += 60;
		endgameEval += 60;
	}
	// bonus for pieces in centre
	/*
	U64 BBWpiecesincentre = (pos->BBwhitepieces & BBbigcentre);
	while (BBWpiecesincentre) {
		int square = __builtin_ctzll(BBWpiecesincentre);
		BBWpiecesincentre &= ~(1ULL << square);
		int pval = pieceval(getPiece(pos,square));
		openingEval += (pval / 100) * 6;
		endgameEval += (pval / 100) * 6;
	}
	U64 BBBpiecesincentre = (pos->BBwhitepieces & BBbigcentre);
	while (BBBpiecesincentre) {
		int square = __builtin_ctzll(BBBpiecesincentre);
		BBBpiecesincentre &= ~(1ULL << square);
		int pval = pieceval(getPiece(pos,square));
		openingEval -= (pval / 100) * 6;
		endgameEval -= (pval / 100) * 6;
	}
	 */
	 
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
			BBattacks = Rmagic(square,BBoccupancy);
			if (BBattacks & BBrooksstart) {
				openingEval += 30;
				endgameEval += 30;
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
			BBattacks = Rmagic(square,BBoccupancy);
			if (BBattacks & BBrooksstart) {
				openingEval -= 30;
				endgameEval -= 30;
				break;
			}
		}
	}
	 */
	// knight outposts
	
	// white
	
	BBwhitepawns = (pos->BBwhitepieces & pos->BBpawns);
	U64 BBwhiteknights = (pos->BBwhitepieces & pos->BBknights);
	while (BBwhiteknights) {
		int square = __builtin_ctzll(BBwhiteknights);
		BBwhiteknights &= ~(1ULL << square);
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
		BBblackknights &= ~(1ULL << square);
		if ((BBpawnWestAttacksW(1ULL << square) & BBblackpawns) || (BBpawnEastAttacksW(1ULL << square) & BBblackpawns)) {
			openingEval -= 20;
			endgameEval -= 20;
		}
	}
	
	// bishop outposts
	
	// white
	
	U64 BBwhitebishops = (pos->BBwhitepieces & pos->BBbishops);
	while (BBwhitebishops) {
		int square = __builtin_ctzll(BBwhitebishops);
		BBwhitebishops &= ~(1ULL << square);
		if ((BBpawnWestAttacksB(1ULL << square) & BBwhitepawns) || (BBpawnEastAttacksB(1ULL << square) & BBwhitepawns)) {
			openingEval += 20;
			endgameEval += 20;
		}
	}
	
	// black
	
	U64 BBblackbishops = (pos->BBblackpieces & pos->BBbishops);
	while (BBblackbishops) {
		int square = __builtin_ctzll(BBblackbishops);
		BBblackbishops &= ~(1ULL << square);
		if ((BBpawnWestAttacksW(1ULL << square) & BBblackpawns) || (BBpawnEastAttacksW(1ULL << square) & BBblackpawns)) {
			openingEval -= 20;
			endgameEval -= 20;
		}
	}
	
	// mobility bonuses
	/*
	int Wmobility = mobility(pos,WHITE);
	int Bmobility = mobility(pos,BLACK);
	
	openingEval += Wmobility * 5;
	endgameEval += Wmobility * 5;
	
	openingEval -= Bmobility * 5;
	endgameEval -= Bmobility * 5;
	*/

	// knight value decreases as pawns disappear
	/*
	openingEval -= num_WN * (16 - (num_WP + num_BP)) * 16;
	endgameEval -= num_WN * (16 - (num_WP + num_BP)) * 16;
	openingEval += num_BN * (16 - (num_WP + num_BP)) * 16;
	endgameEval += num_BN * (16 - (num_WP + num_BP)) * 16;
	*/
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
	return count;
	
}
int isEndgame(struct position *pos) {
	int numpieces = 1;
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
	if (numpieces <= 3) return 1;
	return 0;
}

int evalBoard(struct position *pos) {
	assert(pos);
	int score = 0;
	int pval;
	U64 BBoccupied = (pos->BBwhitepieces | pos->BBblackpieces);
	while (BBoccupied != 0) {
		int square = __builtin_ctzll(BBoccupied);
		BBoccupied &= ~(1ULL << square);
		char piece = getPiece(pos,square);
		pval = pieceval(piece);
		if ((piece >= 'a') && (piece <= 'z')) {
			pval = -pval;
		}
		int pstscore = PSTval(piece,square,'O');
		score += pval;
		score += pstscore;
		
	}
	if (pos->tomove == BLACK) return -score;
	return score;
}