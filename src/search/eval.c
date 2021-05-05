#include <stdio.h>
#include <stdlib.h>
#include "eval.h"
#include "../chess/position.h"
#include "PST.h"
#include "../chess/attacks.h"
#include "../chess/bitboards.h"
#include "../chess/magicmoves.h"
#include "../globals.h"
#include "../misc.h"
#include "search.h"

#define Bit(a,b)      ((a) & (1ull << (b)) ? (-1) : (0))

// Evaluation values

int SideToMove_mg = 24;
int SideToMove_eg = 27;
int OppKingProximity = 16;
int MyKingProximity = -10;
int PawnChain_mg = 4;
int PawnChain_eg = 2;
int DoubledPawn_mg = 8;
int DoubledPawn_eg = 7;
int IsolatedPawn_mg = 22;
int IsolatedPawn_eg = 20;
int ZeroOrEightPawns_mg = 10;
int ZeroOrEightPawns_eg = 10;
int PawnShield = 27;
int PawnsInCentre_mg = 14;
int PawnsInCentre_eg = 0;
int PawnsAttackingCentre_mg = 9;
int PawnsAttackingCentre_eg = 8;
int ConnectedKnights_mg = 2;
int ConnectedKnights_eg = 37;
int KnightsProtectedByPawn_mg = 7;
int KnightsProtectedByPawn_eg = 16;
int ClosedKnights_mg = 50;
int ClosedKnights_eg = 50;
int BishopPair_mg = 28;
int BishopPair_eg = 106;
int BishopsProtectedByPawn_mg = 6;
int BishopsProtectedByPawn_eg = 12;
int OpenBishops_mg = 0;
int OpenBishops_eg = 0;
int RookOpenFile_mg = 60;
int RookOpenFile_eg = 2;
int RookSemiOpenFile_mg = 27;
int RookSemiOpenFile_eg = 24;
int RookSameFileQueen_mg = 20;
int RookSameFileQueen_eg = 0;
int RookOn7th_mg = 0;
int RookOn7th_eg = 11;
int QueenOn7th_mg = 0;
int QueenOn7th_eg = 35;
int KingPawnlessFlank_mg = 17;
int KingPawnlessFlank_eg = 95;
int ImbalanceFactor = 180;


int FreePawnRankBonus[8] = {0, 0, 10, 20, 40, 60, 80, 120 };
int PassedRankBonus[8] = { 0, 0, 0, 18, 52, 108, 186, 0 };
int PassedFileBonus_mg[8] = { 25, 11, -14, -14, -14, -14, 11, 25 };
int PassedFileBonus_eg[8] = { 20, 15, 5, -7, -7, 5, 15, 20 };

// Minor piece attack bonus

int minorAttackBonus_mg[6] = {6, 14, 14, 20, 22, 0};
int minorAttackBonus_eg[6] = {10, 18, 18, 28, 30, 0};

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
}
void evalMaterial(struct position *pos, int *openingEval, int *endgameEval) {
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

    *openingEval += white_pieces - black_pieces;
	*endgameEval += white_pieces - black_pieces;
}
void evalKPPST(struct position *pos, int *openingEval, int *endgameEval) {
	
}
void evalPawns(struct position *pos, int *openingEval, int *endgameEval) {
	
	// Evaluate pawns
	
	
}
void evalKnights(struct position *pos, int *openingEval, int *endgameEval) {
	
	
}
void evalBishops(struct position *pos, int *openingEval, int *endgameEval) {
	
}
void evalKRmate(struct position *pos, int *endgameEval) {
}
void evalRooks(struct position *pos, int *openingEval, int *endgameEval) {
	
}

void evalQueens(struct position *pos, int *openingEval, int *endgameEval) {
}
void evalKings(struct position *pos, int *openingEval, int *endgameEval) {
	
}
void evalMobility(struct position *pos, int *openingEval, int *endgameEval) {
}
void evalMinorAttacks(struct position *pos, int *openingEval, int *endgameEval) {
		// bonus for minor pieces attacking enemy pieces not defended by pawns
	
}
void evalMaterialImbalance(struct position *pos, int *openingEval, int *endgameEval) {
}
int evalEndgame(struct position *pos, int endgameEval) {
	return endgameEval;
}
int finalEval(struct position *pos, int *openingEval, int *endgameEval) {
	
	// get number of pieces for calculating phase for final eval
	
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
	
	int pawnPhase = 0;
	int knightPhase = 1;
	int bishopPhase = 1;
	int rookPhase = 2;
	int queenPhase = 4;
	
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
	
	int eval = ((*openingEval * (256 - phase)) + (*endgameEval * phase)) / 256;

	if (pos->tomove == BLACK) eval = -eval;
	return eval;
}
int taperedEval(struct position *pos) {
	assert(pos);
	
	int openingEval = 0;
	int endgameEval = 0;
	
	
	
	evalMaterial(pos, &openingEval, &endgameEval);
	
	// Get king and pawn PST values
	// Other piece PSTs get calculated with mobility
	
	evalKPPST(pos, &openingEval, &endgameEval);
	
	// evaluate pieces
	
	evalPawns(pos, &openingEval, &endgameEval);
	evalKnights(pos, &openingEval, &endgameEval);
	evalBishops(pos, &openingEval, &endgameEval);
	evalRooks(pos, &openingEval, &endgameEval);
	evalQueens(pos, &openingEval, &endgameEval);
	evalKings(pos, &openingEval, &endgameEval);
	
	// mop up evaluation for basic checkmate
	
	evalKRmate(pos, &endgameEval);
	
	// mobility
	
	evalMobility(pos, &openingEval, &endgameEval);
	
	// minor attacks
	
	evalMinorAttacks(pos, &openingEval, &endgameEval);
	
	// bonus for trading when ahead in material
	
	evalMaterialImbalance(pos, &openingEval, &endgameEval);
	
	// endgame eval
	
	endgameEval = evalEndgame(pos, endgameEval);
	
	// combine opening and endgame eval into final evaluation
	
	return finalEval(pos, &openingEval, &endgameEval);
}

struct mobreturn Nmobility(struct position *pos, int side) {
	U64 BBsidepieces;
	U64 BBkingzone;
	U64 BBattackedbypawns;
	if (side == WHITE) {
		BBsidepieces = pos->colours[WHITE];
		BBkingzone = BBpawnshieldLookup[BLACK][pos->Bkingpos];
		BBattackedbypawns = BBpawnattacksB(pos->pieces[PAWN] & pos->colours[BLACK]);
	}
	else {
		BBsidepieces = pos->colours[BLACK];
		BBkingzone = BBpawnshieldLookup[WHITE][pos->Wkingpos];
		BBattackedbypawns = BBpawnattacksW(pos->pieces[PAWN] & pos->colours[WHITE]);
	}
	U64 BBallowed = ~BBsidepieces;
	U64 BBmoves = 0;
	U64 BBcopy = 0;
	int from = 0;
	
	int kzattacks = 0;
	int kzattackers = 0;
	int PSTvalO = 0;
	int PSTvalE = 0;
	char piece = KNIGHT;
	int centre = 0;
	// Knights
	BBcopy = pos->pieces[KNIGHT] & BBsidepieces;
	while(BBcopy)
	{
		from = __builtin_ctzll(BBcopy);
		//BBmoves |= BBknightattacks(1ULL << from) & BBallowed;
		U64 BBnewmoves = BBknightLookup[from] & BBallowed;
		BBmoves |= BBnewmoves;
		U64 BBkzattacks = BBnewmoves & BBkingzone;
		centre += __builtin_popcountll(BBcentre & BBnewmoves);
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
	returnstruct.centre = centre;
	U64 BBhomerows = BBrank7 | BBrank8;
	if (pos->tomove == BLACK) BBhomerows = BBrank1 | BBrank2;
	returnstruct.homerowsattacks = __builtin_popcountll(BBmoves & BBhomerows);
	return returnstruct;
}
struct mobreturn Bmobility(struct position *pos, int side) {
	U64 BBsidepieces;
	U64 BBkingzone;
	U64 BBattackedbypawns;
	if (side == WHITE) {
		BBsidepieces = pos->colours[WHITE];
		BBkingzone = BBpawnshieldLookup[BLACK][pos->Bkingpos];
		BBattackedbypawns = BBpawnattacksB(pos->pieces[PAWN] & pos->colours[BLACK]);
	}
	else {
		BBsidepieces = pos->colours[BLACK];
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
	int centre = 0;
	char piece = BISHOP;
	BBcopy = pos->pieces[BISHOP] & BBsidepieces;
	while(BBcopy)
	{
		from = __builtin_ctzll(BBcopy);
		U64 BBnewmoves = Bmagic(from, BBoccupied) & BBallowed;
		BBmoves |= BBnewmoves;
		U64 BBkzattacks = BBnewmoves & BBkingzone;
		kzattacks += __builtin_popcountll(BBkzattacks);
		centre += __builtin_popcountll(BBcentre & BBnewmoves);
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
	returnstruct.centre = centre;
	U64 BBhomerows = BBrank7 | BBrank8;
	if (pos->tomove == BLACK) BBhomerows = BBrank1 | BBrank2;
	returnstruct.homerowsattacks = __builtin_popcountll(BBmoves & BBhomerows);
	return returnstruct;
}
struct mobreturn Rmobility(struct position *pos, int side) {
	U64 BBsidepieces;
	U64 BBkingzone;
	U64 BBattackedbypawns;
	if (side == WHITE) {
		BBsidepieces = pos->colours[WHITE];
		BBkingzone = BBpawnshieldLookup[BLACK][pos->Bkingpos];
		BBattackedbypawns = BBpawnattacksB(pos->pieces[PAWN] & pos->colours[BLACK]);
	}
	else {
		BBsidepieces = pos->colours[BLACK];
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
	int centre = 0;
	BBcopy = pos->pieces[ROOK] & BBsidepieces;
	while(BBcopy)
	{
		from = __builtin_ctzll(BBcopy);
		U64 BBnewmoves = Rmagic(from, BBoccupied) & BBallowed;
		BBmoves |= BBnewmoves;
		U64 BBkzattacks = BBnewmoves & BBkingzone;
		kzattacks += __builtin_popcountll(BBkzattacks);
		centre += __builtin_popcountll(BBcentre & BBnewmoves);
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
	returnstruct.centre = centre;
	U64 BBhomerows = BBrank7 | BBrank8;
	if (pos->tomove == BLACK) BBhomerows = BBrank1 | BBrank2;
	returnstruct.homerowsattacks = __builtin_popcountll(BBmoves & BBhomerows);
	return returnstruct;
}
struct mobreturn Qmobility(struct position *pos, int side) {
	U64 BBsidepieces;
	U64 BBkingzone;
	U64 BBattackedbypawns;
	if (side == WHITE) {
		BBsidepieces = pos->colours[WHITE];
		BBkingzone = BBpawnshieldLookup[BLACK][pos->Bkingpos];
		BBattackedbypawns = BBpawnattacksB(pos->pieces[PAWN] & pos->colours[BLACK]);
	}
	else {
		BBsidepieces = pos->colours[BLACK];
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
	int centre = 0;
	BBcopy = pos->pieces[QUEEN] & BBsidepieces;
	while(BBcopy)
	{
		from = __builtin_ctzll(BBcopy);
		U64 BBnewmoves = (Rmagic(from, BBoccupied) | Bmagic(from, BBoccupied)) & BBallowed;
		BBmoves |= BBnewmoves;
		U64 BBkzattacks = BBnewmoves & BBkingzone;
		kzattacks += __builtin_popcountll(BBkzattacks);
		centre += __builtin_popcountll(BBcentre & BBnewmoves);
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
	returnstruct.centre = centre;
	U64 BBhomerows = BBrank7 | BBrank8;
	if (pos->tomove == BLACK) BBhomerows = BBrank1 | BBrank2;
	returnstruct.homerowsattacks = __builtin_popcountll(BBmoves & BBhomerows);
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
	if (numpieces <= 3) return 1;
	return 0;
}