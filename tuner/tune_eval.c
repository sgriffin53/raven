#include "../eval.h"
#include "../position.h"
#include "../PST.h"
#include "../attacks.h"
#include "../bitboards.h"
#include "../magicmoves.h"
#include "../globals.h"
#include "../misc.h"
#include <stdio.h>
#include <stdlib.h>
#include "../search.h"
#include "tune_eval.h"

// For tuning

//int tuneParams[5] = { 110, 300, 300, 525, 900 };

int tuneParams[1024];
int tuneParamsInit[1024];
char tuneParamsName[1024][128];

void setParams() {
	strcpy(tuneParamsName[0],"Side to move");
	tuneParams[0] = 20;
	tuneParamsInit[0] = 20;
	
	strcpy(tuneParamsName[1], "Passed rank bonus (rank 2)");
	tuneParams[1] = 10;
	tuneParamsInit[1] = 10;
	
	strcpy(tuneParamsName[2], "Passed rank bonus (rank 3)");
	tuneParams[2] = 10;
	tuneParamsInit[2] = 10;
	
	strcpy(tuneParamsName[3], "Passed rank bonus (rank 4)");
	tuneParams[3] = 15;
	tuneParamsInit[3] = 15;
	
	strcpy(tuneParamsName[4], "Passed rank bonus (rank 5)");
	tuneParams[4] = 25;
	tuneParamsInit[4] = 25;
	
	strcpy(tuneParamsName[5], "Passed rank bonus (rank 6)");
	tuneParams[5] = 80;
	tuneParamsInit[5] = 80;
	
	strcpy(tuneParamsName[6], "Passed rank bonus (rank 7)");
	tuneParams[6] = 120;
	tuneParamsInit[6] = 120;

	strcpy(tuneParamsName[7], "Passed file bonus midgame (A file)");
	tuneParams[7] = 25;
	tuneParamsInit[7] = 25;
	
	strcpy(tuneParamsName[8], "Passed file bonus midgame (B file)");
	tuneParams[8] = 11;
	tuneParamsInit[8] = 11;
	
	strcpy(tuneParamsName[9], "Passed file bonus midgame (C file)");
	tuneParams[9] = -14;
	tuneParamsInit[9] = -14;
	
	strcpy(tuneParamsName[10], "Passed file bonus midgame (D file)");
	tuneParams[10] = -14;
	tuneParamsInit[10] = -14;
	
	strcpy(tuneParamsName[11], "Passed file bonus midgame (E file)");
	tuneParams[11] = -14;
	tuneParamsInit[11] = -14;
	
	strcpy(tuneParamsName[12], "Passed file bonus midgame (F file)");
	tuneParams[12] = 11;
	tuneParamsInit[12] = 11;
	
	strcpy(tuneParamsName[13], "Passed file bonus midgame (G file)");
	tuneParams[13] = 25;
	tuneParamsInit[13] = 25;
	
	strcpy(tuneParamsName[14], "Passed file bonus midgame (H file)");
	tuneParams[14] = 25;
	tuneParamsInit[14] = 25;
	
	strcpy(tuneParamsName[15], "Passed file bonus endgame (A file)");
	tuneParams[15] = 20;
	tuneParamsInit[15] = 20;
	
	strcpy(tuneParamsName[16], "Passed file bonus endgame (B file)");
	tuneParams[16] = 15;
	tuneParamsInit[16] = 15;
	
	strcpy(tuneParamsName[17], "Passed file bonus endgame (C file)");
	tuneParams[17] = 5;
	tuneParamsInit[17] = 5;
	
	strcpy(tuneParamsName[18], "Passed file bonus endgame (D file)");
	tuneParams[18] = -7;
	tuneParamsInit[18] = -7;
	
	strcpy(tuneParamsName[19], "Passed file bonus endgame (E file)");
	tuneParams[19] = -7;
	tuneParamsInit[19] = -7;
	
	strcpy(tuneParamsName[20], "Passed file bonus endgame (F file)");
	tuneParams[20] = 5;
	tuneParamsInit[20] = 5;
	
	strcpy(tuneParamsName[21], "Passed file bonus endgame (G file)");
	tuneParams[21] = 15;
	tuneParamsInit[21] = 15;
	
	strcpy(tuneParamsName[22], "Passed file bonus endgame (H file)");
	tuneParams[22] = 20;
	tuneParamsInit[22] = 20;
	
	strcpy(tuneParamsName[23], "Opposite king proximity");
	tuneParams[23] = 14;
	tuneParamsInit[23] = 14;
	
	strcpy(tuneParamsName[24], "My king proximity");
	tuneParams[24] = -11;
	tuneParamsInit[24] = -11;
	
	strcpy(tuneParamsName[25], "Pawn chain bonus");
	tuneParams[25] = 20;
	tuneParamsInit[25] = 20;
	
	strcpy(tuneParamsName[26], "Doubled pawn penalty");
	tuneParams[26] = 16;
	tuneParamsInit[26] = 16;
	
	strcpy(tuneParamsName[27], "Isolated pawn penalty");
	tuneParams[27] = 6;
	tuneParamsInit[27] = 6;
	
	strcpy(tuneParamsName[28], "Free passer bonus rank 3");
	tuneParams[28] = 10;
	tuneParamsInit[28] = 10;

	strcpy(tuneParamsName[29], "Free passer bonus rank 4");
	tuneParams[29] = 20;
	tuneParamsInit[29] = 20;
	
	strcpy(tuneParamsName[30], "Free passer bonus rank 5");
	tuneParams[30] = 40;
	tuneParamsInit[30] = 40;
	
	strcpy(tuneParamsName[31], "Free passer bonus rank 6");
	tuneParams[31] = 60;
	tuneParamsInit[31] = 60;
	
	strcpy(tuneParamsName[32], "Free passer bonus rank 7");
	tuneParams[32] = 80;
	tuneParamsInit[32] = 80;
	
	strcpy(tuneParamsName[33], "Free passer bonus rank 8");
	tuneParams[33] = 120;
	tuneParamsInit[33] = 120;
	
	strcpy(tuneParamsName[34], "Queens on 7th rank midgame");
	tuneParams[34] = 10;
	tuneParamsInit[34] = 10;
	
	strcpy(tuneParamsName[35], "Queens on 7th rank endgame");
	tuneParams[35] = 20;
	tuneParamsInit[35] = 20;
	
	strcpy(tuneParamsName[36], "Rooks on 7th rank midgame");
	tuneParams[36] = 20;
	tuneParamsInit[36] = 20;
	
	strcpy(tuneParamsName[37], "Rooks on 7th rank endgame");
	tuneParams[37] = 40;
	tuneParamsInit[37] = 40;
	
	strcpy(tuneParamsName[38], "Rooks on open files midgame");
	tuneParams[38] = 48;
	tuneParamsInit[38] = 48;
	
	strcpy(tuneParamsName[39], "Rooks on open files endgame");
	tuneParams[39] = 16;
	tuneParamsInit[39] = 16;
	
	strcpy(tuneParamsName[40], "Rooks on semi-open files midgame");
	tuneParams[40] = 16;
	tuneParamsInit[40] = 16;
	
	strcpy(tuneParamsName[41], "Rooks on semi-open files endgame");
	tuneParams[41] = 6;
	tuneParamsInit[41] = 6;
	
	strcpy(tuneParamsName[42], "Rooks on same file as queen midgame");
	tuneParams[42] = 40;
	tuneParamsInit[42] = 40;
	
	strcpy(tuneParamsName[43], "Rooks on same file as queen endgame");
	tuneParams[43] = 40;
	tuneParamsInit[43] = 40;
	
	strcpy(tuneParamsName[44], "Pawn shield");
	tuneParams[44] = 30;
	tuneParamsInit[44] = 30;
	
	strcpy(tuneParamsName[45], "Bishop pair midgame");
	tuneParams[45] = 60;
	tuneParamsInit[45] = 60;
	
	strcpy(tuneParamsName[46], "Bishop pair endgame");
	tuneParams[46] = 60;
	tuneParamsInit[46] = 60;
	
	strcpy(tuneParamsName[47], "Pawns in centre midgame");
	tuneParams[47] = 20;
	tuneParamsInit[47] = 20;
	
	strcpy(tuneParamsName[48], "Pawns in centre endgame");
	tuneParams[48] = 20;
	tuneParamsInit[48] = 20;
	
	strcpy(tuneParamsName[49], "Pawns attacking centre midgame");
	tuneParams[49] = 20;
	tuneParamsInit[49] = 20;
	
	strcpy(tuneParamsName[50], "Pawns attacking centre endgame");
	tuneParams[50] = 10;
	tuneParamsInit[50] = 10;
	
	strcpy(tuneParamsName[51], "Connected knights midgame");
	tuneParams[51] = 25;
	tuneParamsInit[51] = 25;
	
	strcpy(tuneParamsName[52], "Connected knights endgame");
	tuneParams[52] = 25;
	tuneParamsInit[52] = 25;
	
	strcpy(tuneParamsName[53], "Imbalance midgame");
	tuneParams[53] = 180;
	tuneParamsInit[53] = 180;
	
	strcpy(tuneParamsName[54], "Imbalance endgame");
	tuneParams[54] = 180;
	tuneParamsInit[54] = 180;
	
	strcpy(tuneParamsName[55], "Knights protected by pawn midgame");
	tuneParams[55] = 20;
	tuneParamsInit[55] = 20;
	
	strcpy(tuneParamsName[56], "Knights protected by pawn endgame");
	tuneParams[56] = 20;
	tuneParamsInit[56] = 20;
	
	strcpy(tuneParamsName[57], "Bishops protected by pawn midgame");
	tuneParams[57] = 20;
	tuneParamsInit[57] = 20;
	
	strcpy(tuneParamsName[58], "Bishops protected by pawn endgame");
	tuneParams[58] = 20;
	tuneParamsInit[58] = 20;
	
	strcpy(tuneParamsName[59], "Knights in closed midgame");
	tuneParams[59] = 20;
	tuneParamsInit[59] = 20;
	
	strcpy(tuneParamsName[60], "Knights in closed endgame");
	tuneParams[60] = 20;
	tuneParamsInit[60] = 20;

	strcpy(tuneParamsName[61], "Bishops in open midgame");
	tuneParams[61] = 20;
	tuneParamsInit[61] = 20;
	
	strcpy(tuneParamsName[62], "Bishops in open endgame");
	tuneParams[62] = 20;
	tuneParamsInit[62] = 20;
	
	strcpy(tuneParamsName[63], "Pawn midgame");
	tuneParams[63] = 110;
	tuneParamsInit[63] = 110;
	
	strcpy(tuneParamsName[64], "Knight midgame");
	tuneParams[64] = 300;
	tuneParamsInit[64] = 300;
	
	strcpy(tuneParamsName[65], "Bishop midgame");
	tuneParams[65] = 300;
	tuneParamsInit[65] = 300;
	
	strcpy(tuneParamsName[66], "Rook midgame");
	tuneParams[66] = 525;
	tuneParamsInit[66] = 525;
	
	strcpy(tuneParamsName[67], "Queen midgame");
	tuneParams[67] = 900;
	tuneParamsInit[67] = 900;
	
	strcpy(tuneParamsName[68], "Pawn endgame");
	tuneParams[68] = 110;
	tuneParamsInit[68] = 110;
	
	strcpy(tuneParamsName[69], "Knight endgame");
	tuneParams[69] = 300;
	tuneParamsInit[69] = 300;
	
	strcpy(tuneParamsName[70], "Bishop endgame");
	tuneParams[70] = 300;
	tuneParamsInit[70] = 300;
	
	strcpy(tuneParamsName[71], "Rook endgame");
	tuneParams[71] = 525;
	tuneParamsInit[71] = 525;
	
	strcpy(tuneParamsName[72], "Queen endgame");
	tuneParams[72] = 900;
	tuneParamsInit[72] = 900;
	
	
	
	// for tuning from 0
	
	//for (int i = 0;i < 63;i++) {
	//
	//	tuneParams[i] = 0;
	//	tuneParamsInit[i] = 0;
	//}
}
// Minor piece attack bonus

int tune_minorAttackBonus_mg[6] = {6, 14, 14, 20, 22, 0};
int tune_minorAttackBonus_eg[6] = {10, 18, 18, 28, 30, 0};

// Piece mobility

int tune_knightMgMobility[9] = {-15, -5, -1, 2, 5, 7, 9, 11, 13};

// (10 * x Pow 0.5) - 15};

int tune_knightEgMobility[9] = {-30, -10, -2, 4, 10, 14, 18, 22, 26};

// (20 * x Pow 0.5) - 30};

int tune_bishopMgMobility[14] = {-25, -11, -6, -1, 3, 6, 9, 12, 14, 17, 19, 21, 23, 25};

// (14 * x Pow 0.5) - 25};

int tune_bishopEgMobility[14] = {-50, -22, -11, -2, 6, 12, 18, 24, 29, 34, 38, 42, 46, 50};

// (28 * x Pow 0.5) - 50};

int tune_rookMgMobility[15] = {-10, -4, -2, 0, 2, 3, 4, 5, 6, 8, 8, 9, 10, 11, 12};

// (6 * x Pow 0.5) - 10};

int tune_rookEgMobility[15] = {-50, -22, -11, -2, 6, 12, 18, 24, 29, 34, 38, 42, 46, 50, 54};

// (28 * x Pow 0.5) - 50};

int tune_queenMgMobility[28] = {-10, -6, -5, -4, -2, -2, -1, 0, 1, 2, 2, 3, 3, 4, 4, 5, 6, 6, 6,

7, 7, 8, 8, 9, 9, 10, 10, 10};

// (4 * x Pow 0.5) - 10};

int tune_queenEgMobility[28] = {-50, -30, -22, -16, -10, -6, -2, 2, 6, 10, 13, 16, 19, 22, 24,

27, 30, 32, 34, 37, 39, 41, 43, 45, 47, 50, 51, 53}; 

// (20 * x Pow 0.5) - 50};

const int tune_arrCenterManhattanDistance[64] = { // char is sufficient as well, also unsigned
  6, 5, 4, 3, 3, 4, 5, 6,
  5, 4, 3, 2, 2, 3, 4, 5,
  4, 3, 2, 1, 1, 2, 3, 4,
  3, 2, 1, 0, 0, 1, 2, 3,
  3, 2, 1, 0, 0, 1, 2, 3,
  4, 3, 2, 1, 1, 2, 3, 4,
  5, 4, 3, 2, 2, 3, 4, 5,
  6, 5, 4, 3, 3, 4, 5, 6
};

//int tune_piecevalues[7] = { 110, 300, 300, 525, 900, 9999, 0 };
int tune_pieceval(int inpiece) {
	//return tune_piecevalues[inpiece];
	
	return tune_pieceval_eg(inpiece);
}
int tune_pieceval_mg(int inpiece) {
	int tune_piecevalues_mg[7];
	tune_piecevalues_mg[0] = tuneParams[63];
	tune_piecevalues_mg[1] = tuneParams[64];
	tune_piecevalues_mg[2] = tuneParams[65];
	tune_piecevalues_mg[3] = tuneParams[66];
	tune_piecevalues_mg[4] = tuneParams[67];
	return tune_piecevalues_mg[inpiece];
}
int tune_pieceval_eg(int inpiece) {
	int tune_piecevalues_eg[7];
	tune_piecevalues_eg[0] = tuneParams[68];
	tune_piecevalues_eg[1] = tuneParams[69];
	tune_piecevalues_eg[2] = tuneParams[70];
	tune_piecevalues_eg[3] = tuneParams[71];
	tune_piecevalues_eg[4] = tuneParams[72];
	return tune_piecevalues_eg[inpiece];
}

int tune_eval(struct position *pos) {
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
    int white_pieces_mg = num_WP*tune_pieceval_mg(PAWN)   +
                       num_WN*tune_pieceval_mg(KNIGHT) +
                       num_WB*tune_pieceval_mg(BISHOP) +
                       num_WR*tune_pieceval_mg(ROOK)   +
                       num_WQ*tune_pieceval_mg(QUEEN);
					   
    int white_pieces_eg = num_WP*tune_pieceval_eg(PAWN)   +
                       num_WN*tune_pieceval_eg(KNIGHT) +
                       num_WB*tune_pieceval_eg(BISHOP) +
                       num_WR*tune_pieceval_eg(ROOK)   +
                       num_WQ*tune_pieceval_eg(QUEEN);

    int black_pieces_mg = num_BP*tune_pieceval_mg(PAWN)   +
                       num_BN*tune_pieceval_mg(KNIGHT) +
                       num_BB*tune_pieceval_mg(BISHOP) +
                       num_BR*tune_pieceval_mg(ROOK)   +
                       num_BQ*tune_pieceval_mg(QUEEN);
					   
    int black_pieces_eg = num_BP*tune_pieceval_eg(PAWN)   +
                       num_BN*tune_pieceval_eg(KNIGHT) +
                       num_BB*tune_pieceval_eg(BISHOP) +
                       num_BR*tune_pieceval_eg(ROOK)   +
                       num_BQ*tune_pieceval_eg(QUEEN);

    openingEval += white_pieces_mg - black_pieces_mg;
	endgameEval += white_pieces_eg - black_pieces_eg;
	//material += white_pieces - black_pieces;
	
	
	U64 BBpawnsandkings = pos->pieces[PAWN] | pos->pieces[KING];
	
	while (BBpawnsandkings != 0) {
		int square = __builtin_ctzll(BBpawnsandkings);

		BBpawnsandkings &= BBpawnsandkings - 1;
		char piece = getPiece(pos,square);
		int col = getColour(pos, square);
		pstvalO = PSTval(col, piece,square,'O');
		pstvalE = PSTval(col, piece,square,'E');
		openingEval += pstvalO;
		endgameEval += pstvalE;
	}
	
		
	// side to move bonus
	
	if (pos->tomove == WHITE) {
		openingEval += tuneParams[0];
		endgameEval += tuneParams[0];
	}
	else {
		openingEval -= tuneParams[0];
		endgameEval -= tuneParams[0];
	}
	
	
	
	
	// passed pawns
	

	//int passedRankBonus[8] = {0, 10, 10, 15, 25, 80, 120, 0};
	//int BpassedRankBonus[8] = {0, 120, 80, 25, 15, 10, 10, 0};
	int passedRankBonus[8];
	passedRankBonus[0] = 0;
	passedRankBonus[1] = tuneParams[1];
	passedRankBonus[2] = tuneParams[2];
	//passedRankBonus[1] = -10;
	passedRankBonus[3] = tuneParams[3];
	passedRankBonus[4] = tuneParams[4];
	passedRankBonus[5] = tuneParams[5];
	passedRankBonus[6] = tuneParams[6];
	passedRankBonus[7] = 0;
	
	int passedFileBonus_mg[8] = { 25, 11, -14, -14, -14, -14, 11, 25 };
	int passedFileBonus_eg[8] = { 20, 15, 5, -7, -7, 5, 15, 20 };
	/*
	int passedFileBonus_mg[8];
	int passedFileBonus_eg[9];
	
	passedFileBonus_mg[0] = tuneParams[7];
	passedFileBonus_mg[1] = tuneParams[8];
	passedFileBonus_mg[2] = tuneParams[9];
	passedFileBonus_mg[3] = tuneParams[10];
	passedFileBonus_mg[4] = tuneParams[11];
	passedFileBonus_mg[5] = tuneParams[12];
	passedFileBonus_mg[6] = tuneParams[13];
	passedFileBonus_mg[7] = tuneParams[14];
	
	passedFileBonus_mg[0] = tuneParams[15];
	passedFileBonus_mg[1] = tuneParams[16];
	passedFileBonus_mg[2] = tuneParams[17];
	passedFileBonus_mg[3] = tuneParams[18];
	passedFileBonus_mg[4] = tuneParams[19];
	passedFileBonus_mg[5] = tuneParams[20];
	passedFileBonus_mg[6] = tuneParams[21];
	passedFileBonus_mg[7] = tuneParams[22];
	*/
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
			int bonus = passedRankBonus[startrank];
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
			
			endgameEval += tuneParams[23] * oppkingdist;
			endgameEval += tuneParams[24] * mykingdist;
			
		}
		
		// pawn chain bonus

		U64 BBpawnattacks = BBpawnEastAttacksB(BBpiece) | BBpawnWestAttacksB(BBpiece);
		if ((BBpawnattacks & BBwhitepawns)) {
			openingEval += tuneParams[25];
			endgameEval += tuneParams[25];
		}
		
		U64 BBfilemask = BBfileA << getfile(square);
		
		// doubled pawns

		U64 BBWpawnsonfile = BBfilemask & (pos->colours[WHITE] & pos->pieces[PAWN]);
		U64 BBisdoubled = BBWpawnsonfile & (BBWpawnsonfile-1);
		if (BBisdoubled) {
			openingEval -= tuneParams[26];
			endgameEval -= tuneParams[26];
		}
		
		
		// isolated pawns
		
		U64 BBleftpawns = westOne(BBfilemask) & (pos->colours[WHITE] & pos->pieces[PAWN]);
		U64 BBrightpawns = eastOne(BBfilemask) & (pos->colours[WHITE] & pos->pieces[PAWN]);
		if (BBleftpawns == 0 && BBrightpawns == 0) {
			openingEval -= tuneParams[27];
			endgameEval -= tuneParams[27];
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
			int bonus = passedRankBonus[7 - startrank];
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
			
			endgameEval -= tuneParams[23] * oppkingdist;
			endgameEval -= tuneParams[24] * mykingdist;
			
		}
		
		
		// pawn chain bonus
		U64 BBpawnattacks = BBpawnEastAttacksW(BBpiece) | BBpawnWestAttacksW(BBpiece);
		if ((BBpawnattacks & BBblackpawns)) {
			openingEval -= tuneParams[25];
			endgameEval -= tuneParams[25];
		}
		U64 BBfilemask = BBfileA << getfile(square);
		
		// Doubled pawns
		U64 BBpawnsonfile = BBfilemask & (pos->colours[BLACK] & pos->pieces[PAWN]);
		
		U64 BBisdoubled = BBpawnsonfile & (BBpawnsonfile-1);
		if (BBisdoubled) {
			openingEval += tuneParams[26];
			endgameEval += tuneParams[26];
		}
		
		// Isolated pawns
		
		U64 BBleftpawns = westOne(BBfilemask) & (pos->colours[BLACK] & pos->pieces[PAWN]);
		U64 BBrightpawns = eastOne(BBfilemask) & (pos->colours[BLACK] & pos->pieces[PAWN]);
		if (BBleftpawns == 0 && BBrightpawns == 0) {
			openingEval += tuneParams[27];
			endgameEval += tuneParams[27];
		}
	}
	
	// give a bonus for free passed pawns
	// pawns on the 6th or 7th rank that can advance without losing material
	
	//int freepawnrankbonus[8] = {0, 0, 10, 20, 40, 60, 80, 120 };
	int freepawnrankbonus[8];
	
	freepawnrankbonus[0] = 0;
	freepawnrankbonus[1] = tuneParams[28];
	freepawnrankbonus[2] = tuneParams[29];
	freepawnrankbonus[3] = tuneParams[30];
	freepawnrankbonus[4] = tuneParams[31];
	freepawnrankbonus[5] = tuneParams[32];
	freepawnrankbonus[6] = tuneParams[33];
	freepawnrankbonus[7] = tuneParams[34];
	
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
	if (tune_isEndgame(pos)) {
		if (material > 0) {
			winningside = WHITE;
		}
		else if (material < 0) winningside = BLACK;
		if (winningside == WHITE) {
			endgameEval += tune_arrCenterManhattanDistance[pos->Bkingpos] * 10;
			int xdist = abs(getfile(pos->Wkingpos) - getfile(pos->Bkingpos));
			int ydist = abs(getrank(pos->Wkingpos) - getrank(pos->Bkingpos));
			int dist = max(xdist, ydist);
			endgameEval += (6 - dist) * 10;
		}
		else if (winningside == BLACK) {
			endgameEval -= tune_arrCenterManhattanDistance[pos->Wkingpos] * 10;
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
		
		openingEval += tuneParams[34];
		endgameEval += tuneParams[35];
	}
	
	while (BBblackqueens) {
		int square = __builtin_ctzll(BBblackqueens);
		BBblackqueens &= BBblackqueens - 1;
		if (getrank(square) != 1) continue;
		// queen on 7th rank
		U64 BBhostilepawns = BBrank2 & pos->colours[WHITE] & pos->pieces[PAWN];
		if (!BBhostilepawns && getrank(pos->Wkingpos) != 0) continue;
		// either hostile pawns on 7th rank or king is on 8th rank
		
		openingEval -= tuneParams[34];
		endgameEval -= tuneParams[35];
	}
	
	while (BBwhiterooks) {
		int square = __builtin_ctzll(BBwhiterooks);
		BBwhiterooks &= BBwhiterooks - 1;
		if (getrank(square) != 6) continue;
		// rook on 7th rank
		U64 BBhostilepawns = BBrank7 & pos->colours[BLACK] & pos->pieces[PAWN];
		if (!BBhostilepawns && getrank(pos->Bkingpos) != 7) continue;
		// either hostile pawns on 7th rank or king is on 8th rank
		
		openingEval += tuneParams[36];
		endgameEval += tuneParams[37];
		
		U64 BBfilemask = BBfileA << getfile(square);
		
		// rooks on open files
		U64 BBpawnsonfile = BBfilemask & pos->pieces[PAWN];
		U64 BBBpawnsonfile = BBfilemask & pos->pieces[PAWN] & pos->colours[BLACK];
		U64 BBWpawnsonfile = BBfilemask & pos->pieces[PAWN] & pos->colours[WHITE];
		// white rook on open file
		if (BBpawnsonfile == 0) {
			// white rook on open file
			openingEval += tuneParams[38];
			endgameEval += tuneParams[39];
		}
		if ((BBWpawnsonfile == 0) && (BBBpawnsonfile)) {
			// white rook on semi-open file with black pawns
			openingEval += tuneParams[40];
			endgameEval += tuneParams[41];
		}
		
		// rook on same file as queen
		
		U64 BBBqueensonfile = BBfilemask & (pos->pieces[QUEEN] & pos->colours[BLACK]);
		if (BBBqueensonfile) {
			openingEval += tuneParams[42];
			endgameEval += tuneParams[43];
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
		
		openingEval -= tuneParams[36];
		endgameEval -= tuneParams[37];
		
		U64 BBfilemask = BBfileA << getfile(square);
		
		// rooks on open files
		U64 BBpawnsonfile = BBfilemask & pos->pieces[PAWN];
		U64 BBBpawnsonfile = BBfilemask & pos->pieces[PAWN] & pos->colours[BLACK];
		U64 BBWpawnsonfile = BBfilemask & pos->pieces[PAWN] & pos->colours[WHITE];
		// black rook on open file
		if (BBpawnsonfile == 0) {
			// black rook on open file
			openingEval -= tuneParams[38];
			endgameEval -= tuneParams[39];
		}
		if ((BBBpawnsonfile == 0) && (BBWpawnsonfile)) {
			// black rook on semi-open file with white pawns
			openingEval -= tuneParams[40];
			endgameEval -= tuneParams[41];
		}
		// rook on same file as queen
		
		U64 BBWqueensonfile = BBfilemask & (pos->pieces[QUEEN] & pos->colours[WHITE]);
		if (BBWqueensonfile) {
			openingEval -= tuneParams[42];
			endgameEval -= tuneParams[43];
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
	openingEval += tuneParams[44] * __builtin_popcountll(BBpawnshield);
	
	// black pawn shield
	
	int Bkingpos = pos->Bkingpos;
	BBpawnshield = BBpawnshieldLookup[BLACK][Bkingpos];
	BBpawnshield &= (pos->colours[BLACK] & pos->pieces[PAWN]);
	openingEval -= tuneParams[44] * __builtin_popcountll(BBpawnshield);
	
	// bishop pair bonus
	
	if (num_BB >= 2) {
		openingEval -= tuneParams[45];
		endgameEval -= tuneParams[46];
	}
	if (num_WB >= 2) {
		openingEval += tuneParams[45];
		endgameEval += tuneParams[46];
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
	openingEval += tuneParams[47] * __builtin_popcountll(BBWpiecesincentre);
	endgameEval += tuneParams[48] * __builtin_popcountll(BBWpiecesincentre);
	
	U64 BBBpiecesincentre = (pos->colours[BLACK] & pos->pieces[PAWN] & BBcentre);
	openingEval -= tuneParams[47] * __builtin_popcountll(BBBpiecesincentre);
	endgameEval -= tuneParams[48] * __builtin_popcountll(BBBpiecesincentre);
	
	// bonus for pawns attacking the centre
	
	U64 BBWattackingcentre = BBpawnattacksW(pos->colours[WHITE] & pos->pieces[PAWN]) & BBcentre;
	openingEval += tuneParams[49] * __builtin_popcountll(BBWattackingcentre);
	endgameEval += tuneParams[50] * __builtin_popcountll(BBWattackingcentre);
	
	U64 BBBattackingcentre = BBpawnattacksB(pos->colours[BLACK] & pos->pieces[PAWN]) & BBcentre;
	openingEval -= tuneParams[49] * __builtin_popcountll(BBBattackingcentre);
	endgameEval -= tuneParams[50] * __builtin_popcountll(BBBattackingcentre);
	
	// bonus for connected knights
	// white
	if (num_WN >= 2) {
		U64 BBWknights = (pos->colours[WHITE] & pos->pieces[KNIGHT]);
		U64 BBattacks = BBknightattacks(BBWknights);
		U64 BBconnectedknights = BBattacks & BBWknights;
		if (BBconnectedknights) {
			openingEval += tuneParams[51];
			endgameEval += tuneParams[52];
		}
	}
	// black
	if (num_BN >= 2) {
		U64 BBBknights = (pos->colours[BLACK] & pos->pieces[KNIGHT]);
		U64 BBattacks = BBknightattacks(BBBknights);
		U64 BBconnectedknights = BBattacks & BBBknights;
		if (BBconnectedknights) {
			openingEval -= tuneParams[51];
			endgameEval -= tuneParams[52];
		}
	}
	
	
	// bonus for trading when ahead in material
	
	int whitematval = num_WN * tune_pieceval(KNIGHT) + num_WB * tune_pieceval(BISHOP) + num_WR * tune_pieceval(ROOK) + num_WQ * tune_pieceval(QUEEN);
	int blackmatval = num_BN * tune_pieceval(KNIGHT) + num_BB * tune_pieceval(BISHOP) + num_BR * tune_pieceval(ROOK) + num_BQ * tune_pieceval(QUEEN);
	
	if (whitematval > blackmatval) {
		double matimb = 1.0 - (blackmatval / whitematval);
		openingEval += matimb * tuneParams[53];
		endgameEval += matimb * tuneParams[54];
	}
	
	else if (blackmatval > whitematval) {
		double matimb = 1.0 - (whitematval / blackmatval);
		openingEval -= matimb * tuneParams[53];
		endgameEval -= matimb * tuneParams[54];
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
			openingEval += tuneParams[55];
			endgameEval += tuneParams[56];
		}
	}
	
	// black
	
	U64 BBblackknights = (pos->colours[BLACK] & pos->pieces[KNIGHT]);
	while (BBblackknights) {
		int square = __builtin_ctzll(BBblackknights);
		//BBblackknights &= ~(1ULL << square);
		BBblackknights &= BBblackknights - 1;
		if ((BBpawnWestAttacksW(1ULL << square) & BBblackpawns) || (BBpawnEastAttacksW(1ULL << square) & BBblackpawns)) {
			openingEval -= tuneParams[55];
			endgameEval -= tuneParams[56];
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
			openingEval += tuneParams[57];
			endgameEval += tuneParams[58];
		}
	}
	
	// black
	
	U64 BBblackbishops = (pos->colours[BLACK] & pos->pieces[BISHOP]);
	while (BBblackbishops) {
		int square = __builtin_ctzll(BBblackbishops);
		//BBblackbishops &= ~(1ULL << square);
		BBblackbishops &= BBblackbishops - 1;
		if ((BBpawnWestAttacksW(1ULL << square) & BBblackpawns) || (BBpawnEastAttacksW(1ULL << square) & BBblackpawns)) {
			openingEval -= tuneParams[57];
			endgameEval -= tuneParams[58];
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
		openingEval += num_WN * (closedness / 8.0) * tuneParams[59];
		endgameEval += num_WN * (closedness / 8.0) * tuneParams[60];
	}
	
	// white bishop bonus
	
	if (closedness < 0) {
		openingEval += num_WB * (-closedness / 8.0) * tuneParams[61];
		endgameEval += num_WB * (-closedness / 8.0) * tuneParams[62];
	}
	
	// black knight bonus
	
	if (closedness > 0) {
		openingEval -= num_BN * (closedness / 8.0) * tuneParams[59];
		endgameEval -= num_BN * (closedness / 8.0) * tuneParams[60];
	}
	
	// black bishop bonus
	
	if (closedness < 0) {
		openingEval -= num_BB * (-closedness / 8.0) * tuneParams[61];
		endgameEval -= num_BB * (-closedness / 8.0) * tuneParams[62];
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
	
	struct mobreturn WNmobility = tune_Nmobility(pos,WHITE);
	idx = max(0, WNmobility.mobility - WNmobility.unsafe * 2);
	openingEval += tune_knightMgMobility[idx];
	endgameEval += tune_knightEgMobility[idx];
	openingEval += WNmobility.pstO;
	endgameEval += WNmobility.pstE;
	kingattacks +=  WNmobility.kingattacks;
	kingattackers += WNmobility.kingattackers;
	
	struct mobreturn WBmobility = tune_Bmobility(pos,WHITE);
	idx = max(0, WBmobility.mobility - WBmobility.unsafe * 2);
	openingEval += tune_bishopMgMobility[idx];
	endgameEval += tune_bishopEgMobility[idx];
	openingEval += WBmobility.pstO;
	endgameEval += WBmobility.pstE;
	kingattacks += WBmobility.kingattacks;
	kingattackers += WBmobility.kingattackers;
	
	struct mobreturn WRmobility = tune_Rmobility(pos,WHITE);
	idx = max(0, WRmobility.mobility - WRmobility.unsafe * 2);
	openingEval += tune_rookMgMobility[idx];
	endgameEval += tune_rookEgMobility[idx];
	openingEval += WRmobility.pstO;
	endgameEval += WRmobility.pstE;
	kingattacks += 2 * WRmobility.kingattacks;
	kingattackers += WRmobility.kingattackers;
	
	struct mobreturn WQmobility = tune_Qmobility(pos,WHITE);
	idx = max(0, WQmobility.mobility - WQmobility.unsafe * 2);
	openingEval += tune_queenMgMobility[idx];
	endgameEval += tune_queenEgMobility[idx];
	openingEval += WQmobility.pstO;
	endgameEval += WQmobility.pstE;
	kingattacks += 4 * WQmobility.kingattacks;
	kingattackers += WQmobility.kingattackers;
	
	openingEval += safety_table[kingattackers][kingattacks];
	endgameEval += safety_table[kingattackers][kingattacks];
	
	kingattacks = 0;
	kingattackers = 0;
	// black
	struct mobreturn BNmobility = tune_Nmobility(pos,BLACK);
	idx = max(0, BNmobility.mobility - BNmobility.unsafe * 2);
	openingEval -= tune_knightMgMobility[idx];
	endgameEval -= tune_knightEgMobility[idx];
	openingEval += BNmobility.pstO;
	endgameEval += BNmobility.pstE;
	kingattacks += BNmobility.kingattacks;
	kingattackers += BNmobility.kingattackers;
	
	struct mobreturn BBmobility = tune_Bmobility(pos,BLACK);
	idx = max(0, BBmobility.mobility - BBmobility.unsafe * 2);
	openingEval -= tune_bishopMgMobility[idx];
	endgameEval -= tune_bishopEgMobility[idx];
	openingEval += BBmobility.pstO;
	endgameEval += BBmobility.pstE;
	kingattacks += BBmobility.kingattacks;
	kingattackers += BBmobility.kingattackers;
	
	struct mobreturn BRmobility = tune_Rmobility(pos,BLACK);
	idx = max(0, BRmobility.mobility - BRmobility.unsafe * 2);
	openingEval -= tune_rookMgMobility[idx];
	endgameEval -= tune_rookEgMobility[idx];
	openingEval += BRmobility.pstO;
	endgameEval += BRmobility.pstE;
	kingattacks += 2 * BRmobility.kingattacks;
	kingattackers += BRmobility.kingattackers;
	
	struct mobreturn BQmobility = tune_Qmobility(pos,BLACK);
	idx = max(0, BQmobility.mobility - BQmobility.unsafe * 2);
	openingEval -= tune_queenMgMobility[idx];
	endgameEval -= tune_queenEgMobility[idx];
	openingEval += BQmobility.pstO;
	endgameEval += BQmobility.pstE;
	kingattacks += 4 * BQmobility.kingattacks;
	kingattackers += BQmobility.kingattackers;
	
	openingEval -= safety_table[kingattackers][kingattacks];
	endgameEval -= safety_table[kingattackers][kingattacks];


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
		openingEval += tune_minorAttackBonus_mg[piece];
		endgameEval += tune_minorAttackBonus_eg[piece];
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
		openingEval -= tune_minorAttackBonus_mg[piece];
		endgameEval -= tune_minorAttackBonus_eg[piece];
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

struct mobreturn tune_Nmobility(struct position *pos, int side) {
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
struct mobreturn tune_Bmobility(struct position *pos, int side) {
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
struct mobreturn tune_Rmobility(struct position *pos, int side) {
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
struct mobreturn tune_Qmobility(struct position *pos, int side) {
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

int tune_isEndgame(struct position *pos) {
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