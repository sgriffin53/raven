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
	
	U64 BBpawnsandkings = pos->pieces[PAWN] | pos->pieces[KING];
	
	while (BBpawnsandkings != 0) {
		int square = __builtin_ctzll(BBpawnsandkings);
		//BBoccupied &= ~(1ULL << square);
		BBpawnsandkings &= BBpawnsandkings - 1;
		char piece = getPiece(pos,square);
		int col = getColour(pos, square);
		*openingEval += PSTval(col, piece,square,'O');
		*endgameEval += PSTval(col, piece,square,'E');
	}
	
}
void evalPawns(struct position *pos, int *openingEval, int *endgameEval) {
	
	// Evaluate pawns
	
	U64 BBwhitePP = 0ULL;
	U64 BBblackPP = 0ULL;
	
	U64 BBwhitepawns = (pos->colours[WHITE] & pos->pieces[PAWN]);
	int num_WP = __builtin_popcountll(BBwhitepawns);
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
			
			int bonus = PassedRankBonus[startrank];
			
			// blockage of stop square
			
			int stopsquare = fileranktosquareidx(getfile(square), startrank + 1);
			if ((1ULL << stopsquare) & pos->colours[BLACK] & (pos->pieces[BISHOP] | pos->pieces[KNIGHT] | pos->pieces[KING])) {
				bonus = PassedRankBonus[startrank - 1];
			}
			
			*openingEval += (int)(0.5 * bonus);
			*endgameEval += 1 * bonus;
			
			*openingEval += PassedFileBonus_mg[getfile(square)];
			*endgameEval += PassedFileBonus_eg[getfile(square)];
			
			// give a bonus/penalty for opponent/friendly king distances to the passed pawn
			
			
			int oppkingdistx = abs(getfile(pos->Bkingpos) - getfile(square));
			int oppkingdisty = abs(getrank(pos->Bkingpos) - getrank(square));
			int oppkingdist = max(oppkingdistx, oppkingdisty);
			
			int mykingdistx = abs(getfile(pos->Wkingpos) - getfile(square));
			int mykingdisty = abs(getrank(pos->Wkingpos) - getrank(square));
			int mykingdist = max(mykingdistx, mykingdisty);
			
			*endgameEval += OppKingProximity * oppkingdist;
			*endgameEval += MyKingProximity * mykingdist;
			
		}
		
		// pawn chain bonus
		U64 BBpawnattacks = BBpawnEastAttacksB(BBpiece) | BBpawnWestAttacksB(BBpiece);
		if ((BBpawnattacks & pos->colours[WHITE] & pos->pieces[PAWN])) {
			*openingEval += PawnChain_mg;
			*endgameEval += PawnChain_eg;
		}
		
		U64 BBfilemask = BBfileA << getfile(square);
		
		// doubled pawns

		U64 BBWpawnsonfile = BBfilemask & (pos->colours[WHITE] & pos->pieces[PAWN]);
		U64 BBisdoubled = BBWpawnsonfile & (BBWpawnsonfile-1);
		if (BBisdoubled) {
			*openingEval -= DoubledPawn_mg;
			*endgameEval -= DoubledPawn_eg;
		}
		
		
		// isolated pawns
		
		U64 BBleftpawns = westOne(BBfilemask) & (pos->colours[WHITE] & pos->pieces[PAWN]);
		U64 BBrightpawns = eastOne(BBfilemask) & (pos->colours[WHITE] & pos->pieces[PAWN]);
		if (BBleftpawns == 0 && BBrightpawns == 0) {
			*openingEval -= IsolatedPawn_mg;
			*endgameEval -= IsolatedPawn_eg;
		}

		
	}
	
	U64 BBblackpawns = (pos->colours[BLACK] & pos->pieces[PAWN]);
	int num_BP = __builtin_popcountll(BBblackpawns);
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
			
			int bonus = PassedRankBonus[7 - startrank];
			
			// blockage of stop square
			
			int stopsquare = fileranktosquareidx(getfile(square), startrank - 1);
			if ((1ULL << stopsquare) & pos->colours[WHITE] & (pos->pieces[BISHOP] | pos->pieces[KNIGHT] | pos->pieces[KING])) {
				bonus = PassedRankBonus[7 - startrank - 1];
			}
			
			*openingEval -= (int)(0.5 * bonus);
			*endgameEval -= 1 * bonus;
			
			*openingEval -= PassedFileBonus_mg[getfile(square)];
			*endgameEval -= PassedFileBonus_eg[getfile(square)];
			
			// give a bonus/penalty for opponent/friendly king distances to the passed pawn
			
			
			int oppkingdistx = abs(getfile(pos->Wkingpos) - getfile(square));
			int oppkingdisty = abs(getrank(pos->Wkingpos) - getrank(square));
			int oppkingdist = max(oppkingdistx, oppkingdisty);
			
			int mykingdistx = abs(getfile(pos->Bkingpos) - getfile(square));
			int mykingdisty = abs(getrank(pos->Bkingpos) - getrank(square));
			int mykingdist = max(mykingdistx, mykingdisty);
			
			*endgameEval -= OppKingProximity * oppkingdist;
			*endgameEval -= MyKingProximity * mykingdist;
			
		}
		
		
		// pawn chain bonus
		U64 BBpawnattacks = BBpawnEastAttacksW(BBpiece) | BBpawnWestAttacksW(BBpiece);
		if ((BBpawnattacks & pos->colours[BLACK] & pos->pieces[PAWN])) {
			*openingEval -= PawnChain_mg;
			*endgameEval -= PawnChain_eg;
		}
		U64 BBfilemask = BBfileA << getfile(square);
		
		// Doubled pawns
		U64 BBpawnsonfile = BBfilemask & (pos->colours[BLACK] & pos->pieces[PAWN]);
		
		U64 BBisdoubled = BBpawnsonfile & (BBpawnsonfile-1);
		if (BBisdoubled) {
			*openingEval += DoubledPawn_mg;
			*endgameEval += DoubledPawn_eg;
		}
		
		// Isolated pawns
		
		U64 BBleftpawns = westOne(BBfilemask) & (pos->colours[BLACK] & pos->pieces[PAWN]);
		U64 BBrightpawns = eastOne(BBfilemask) & (pos->colours[BLACK] & pos->pieces[PAWN]);
		if (BBleftpawns == 0 && BBrightpawns == 0) {
			*openingEval += IsolatedPawn_mg;
			*endgameEval += IsolatedPawn_eg;
		}
	}
	
	// give a bonus for free passed pawns
	// pawns on the 6th or 7th rank that can advance without losing material
	
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
			*openingEval += FreePawnRankBonus[getrank(square)];
			*endgameEval += FreePawnRankBonus[getrank(square)];
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
			*openingEval -= FreePawnRankBonus[7 - getrank(square)];
			*endgameEval -= FreePawnRankBonus[7 - getrank(square)];
		}
	}
	
	// penalties for 8 or 0 pawns
	
	if (num_WP == 0 || num_WP == 8) {
		*openingEval -= ZeroOrEightPawns_mg;
		*endgameEval -= ZeroOrEightPawns_eg;
	}
	if (num_BP == 0 || num_BP == 8) {
		*openingEval += ZeroOrEightPawns_mg;
		*endgameEval += ZeroOrEightPawns_eg;
	}
	
	// pawn shield
	
	// white pawn shield
	
	int Wkingpos = pos->Wkingpos;
	U64 BBpawnshield = BBpawnshieldLookup[WHITE][Wkingpos];
	BBpawnshield &= (pos->colours[WHITE] & pos->pieces[PAWN]);
	*openingEval += PawnShield * __builtin_popcountll(BBpawnshield);
	
	// black pawn shield
	
	int Bkingpos = pos->Bkingpos;
	BBpawnshield = BBpawnshieldLookup[BLACK][Bkingpos];
	BBpawnshield &= (pos->colours[BLACK] & pos->pieces[PAWN]);
	*openingEval -= PawnShield * __builtin_popcountll(BBpawnshield);
	
	// bonus for pawns in centre
	
	U64 BBWpiecesincentre = (pos->colours[WHITE] & pos->pieces[PAWN] & BBcentre);
	*openingEval += PawnsInCentre_mg * __builtin_popcountll(BBWpiecesincentre);
	*endgameEval += PawnsInCentre_eg * __builtin_popcountll(BBWpiecesincentre);
	
	U64 BBBpiecesincentre = (pos->colours[BLACK] & pos->pieces[PAWN] & BBcentre);
	*openingEval -= PawnsInCentre_mg * __builtin_popcountll(BBBpiecesincentre);
	*endgameEval -= PawnsInCentre_eg * __builtin_popcountll(BBBpiecesincentre);
	
	// bonus for pawns attacking the centre
	
	U64 BBWattackingcentre = BBpawnattacksW(pos->colours[WHITE] & pos->pieces[PAWN]) & BBcentre;
	*openingEval += PawnsAttackingCentre_mg * __builtin_popcountll(BBWattackingcentre);
	*endgameEval += PawnsAttackingCentre_eg * __builtin_popcountll(BBWattackingcentre);
	
	U64 BBBattackingcentre = BBpawnattacksB(pos->colours[BLACK] & pos->pieces[PAWN]) & BBcentre;
	*openingEval -= PawnsAttackingCentre_mg * __builtin_popcountll(BBBattackingcentre);
	*endgameEval -= PawnsAttackingCentre_eg * __builtin_popcountll(BBBattackingcentre);
	
}
void evalKnights(struct position *pos, int *openingEval, int *endgameEval) {
	
	int num_BN = __builtin_popcountll(pos->colours[BLACK] & pos->pieces[KNIGHT]);
	int num_WN = __builtin_popcountll(pos->colours[WHITE] & pos->pieces[KNIGHT]);
	
	// bonus for connected knights
	// white
	if (num_WN >= 2) {
		U64 BBWknights = (pos->colours[WHITE] & pos->pieces[KNIGHT]);
		U64 BBattacks = BBknightattacks(BBWknights);
		U64 BBconnectedknights = BBattacks & BBWknights;
		if (BBconnectedknights) {
			*openingEval += ConnectedKnights_mg;
			*endgameEval += ConnectedKnights_eg;
		}
	}
	// black
	if (num_BN >= 2) {
		U64 BBBknights = (pos->colours[BLACK] & pos->pieces[KNIGHT]);
		U64 BBattacks = BBknightattacks(BBBknights);
		U64 BBconnectedknights = BBattacks & BBBknights;
		if (BBconnectedknights) {
			*openingEval -= ConnectedKnights_mg;
			*endgameEval -= ConnectedKnights_eg;
		}
	}
	
	
	// knights protected by pawns
	
	// white
	
	U64 BBwhitepawns = (pos->colours[WHITE] & pos->pieces[PAWN]);
	U64 BBblackpawns = (pos->colours[BLACK] & pos->pieces[PAWN]);
	
	U64 BBwhiteknights = (pos->colours[WHITE] & pos->pieces[KNIGHT]);
	while (BBwhiteknights) {
		int square = __builtin_ctzll(BBwhiteknights);
		//BBwhiteknights &= ~(1ULL << square);
		BBwhiteknights &= BBwhiteknights - 1;
		if ((BBpawnWestAttacksB(1ULL << square) & BBwhitepawns) || (BBpawnEastAttacksB(1ULL << square) & BBwhitepawns)) {
			*openingEval += KnightsProtectedByPawn_mg;
			*endgameEval += KnightsProtectedByPawn_eg;
		}
	}
	
	// black
	
	U64 BBblackknights = (pos->colours[BLACK] & pos->pieces[KNIGHT]);
	while (BBblackknights) {
		int square = __builtin_ctzll(BBblackknights);
		//BBblackknights &= ~(1ULL << square);
		BBblackknights &= BBblackknights - 1;
		if ((BBpawnWestAttacksW(1ULL << square) & BBblackpawns) || (BBpawnEastAttacksW(1ULL << square) & BBblackpawns)) {
			*openingEval -= KnightsProtectedByPawn_mg;
			*endgameEval -= KnightsProtectedByPawn_eg;
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
		*openingEval += num_WN * (int)(closedness / 8.0) * ClosedKnights_mg;
		*endgameEval += num_WN * (int)(closedness / 8.0) * ClosedKnights_eg;
	}
	
	// black knight bonus
	
	if (closedness > 0) {
		*openingEval -= num_BN * (int)(closedness / 8.0) * ClosedKnights_mg;
		*endgameEval -= num_BN * (int)(closedness / 8.0) * ClosedKnights_eg;
	}
}
void evalBishops(struct position *pos, int *openingEval, int *endgameEval) {
	
	U64 BBwhitepawns = (pos->colours[WHITE] & pos->pieces[PAWN]);
	U64 BBblackpawns = (pos->colours[BLACK] & pos->pieces[PAWN]);
	
		
	int num_BB = __builtin_popcountll(pos->colours[BLACK] & pos->pieces[BISHOP]);
	int num_WB = __builtin_popcountll(pos->colours[WHITE] & pos->pieces[BISHOP]);
	
	// bishop pair bonus
	
	if (num_BB >= 2) {
		*openingEval -= BishopPair_mg;
		*endgameEval -= BishopPair_eg;
	}
	if (num_WB >= 2) {
		*openingEval += BishopPair_mg;
		*endgameEval += BishopPair_eg;
	}
	

	// bishops protected by pawns
	
	// white
	
	U64 BBwhitebishops = (pos->colours[WHITE] & pos->pieces[BISHOP]);
	while (BBwhitebishops) {
		int square = __builtin_ctzll(BBwhitebishops);
		//BBwhitebishops &= ~(1ULL << square);
		BBwhitebishops &= BBwhitebishops - 1;
		if ((BBpawnWestAttacksB(1ULL << square) & BBwhitepawns) || (BBpawnEastAttacksB(1ULL << square) & BBwhitepawns)) {
			*openingEval += BishopsProtectedByPawn_mg;
			*endgameEval += BishopsProtectedByPawn_eg;
		}
	}
	
	// black
	
	U64 BBblackbishops = (pos->colours[BLACK] & pos->pieces[BISHOP]);
	while (BBblackbishops) {
		int square = __builtin_ctzll(BBblackbishops);
		//BBblackbishops &= ~(1ULL << square);
		BBblackbishops &= BBblackbishops - 1;
		if ((BBpawnWestAttacksW(1ULL << square) & BBblackpawns) || (BBpawnEastAttacksW(1ULL << square) & BBblackpawns)) {
			*openingEval -= BishopsProtectedByPawn_mg;
			*endgameEval -= BishopsProtectedByPawn_eg;
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
	
	// white bishop bonus
	
	if (closedness < 0) {
		*openingEval += num_WB * (int)(-closedness / 8.0) * OpenBishops_mg;
		*endgameEval += num_WB * (int)(-closedness / 8.0) * OpenBishops_eg;
	}
	
	// black bishop bonus
	
	if (closedness < 0) {
		*openingEval -= num_BB * (int)(-closedness / 8.0) * OpenBishops_mg;
		*endgameEval -= num_BB * (int)(-closedness / 8.0) * OpenBishops_eg;
	}
	
}
void evalKRmate(struct position *pos, int *openingEval, int *endgameEval) {
	
	int num_BR = __builtin_popcountll(pos->colours[BLACK] & pos->pieces[ROOK]);
	int num_WR = __builtin_popcountll(pos->colours[WHITE] & pos->pieces[ROOK]);
	
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
			*endgameEval += 200;
			// check if enemy king is on same file as rook
			int square = __builtin_ctzll(pos->colours[WHITE] & pos->pieces[ROOK]);
			if (getfile(pos->Bkingpos) == getfile(square)) {
				*endgameEval += 300;
			}
		}
		if (ydist == 2 && xdist == 0) {
			// king is opposing enemy king two ranks away
			*endgameEval += 200;
			// check if enemy king is on same rank as rook
			int square = __builtin_ctzll(pos->colours[WHITE] & pos->pieces[ROOK]);
			if (getrank(pos->Bkingpos) == getrank(square)) {
				*endgameEval += 300;
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
			*endgameEval -= 200;
			// check if enemy king is on same file as rook
			int square = __builtin_ctzll(pos->colours[BLACK] & pos->pieces[ROOK]);
			if (getfile(pos->Wkingpos) == getfile(square)) {
				*endgameEval -= 300;
			}
		}
		if (ydist == 2 && xdist == 0) {
			// king is opposing enemy king two ranks away
			*endgameEval -= 200;
			// check if enemy king is on same rank as rook
			int square = __builtin_ctzll(pos->colours[BLACK] & pos->pieces[ROOK]);
			if (getrank(pos->Wkingpos) == getrank(square)) {
				*endgameEval -= 300;
			}
		}
	} 
}
void evalRooks(struct position *pos, int *openingEval, int *endgameEval) {
	
	// bonus for rooks being on 7th rank or (semi-)open files
	
	U64 BBwhiterooks = pos->colours[WHITE] & pos->pieces[ROOK];
	U64 BBblackrooks = pos->colours[BLACK] & pos->pieces[ROOK];
	
	while (BBwhiterooks) {
		int square = __builtin_ctzll(BBwhiterooks);
		BBwhiterooks &= BBwhiterooks - 1;
		
		U64 BBfilemask = BBfileA << getfile(square);
		
		
		// rooks on open files
		U64 BBpawnsonfile = BBfilemask & pos->pieces[PAWN];
		U64 BBBpawnsonfile = BBfilemask & pos->pieces[PAWN] & pos->colours[BLACK];
		U64 BBWpawnsonfile = BBfilemask & pos->pieces[PAWN] & pos->colours[WHITE];
		// white rook on open file
		if (BBpawnsonfile == 0) {
			// white rook on open file
			*openingEval += RookOpenFile_mg;
			*endgameEval += RookOpenFile_eg;
		}
		if ((BBWpawnsonfile == 0) && (BBBpawnsonfile)) {
			// white rook on semi-open file with black pawns
			*openingEval += RookSemiOpenFile_mg;
			*endgameEval += RookSemiOpenFile_eg;
		}
		
		// rook on same file as queen
		
		U64 BBBqueensonfile = BBfilemask & (pos->pieces[QUEEN] & pos->colours[BLACK]);
		if (BBBqueensonfile) {
			*openingEval += RookSameFileQueen_mg;
			*endgameEval += RookSameFileQueen_eg;
		}
		
		// rooks on 7th rank
		
		if (getrank(square) != 6) continue;
		// rook on 7th rank
		U64 BBhostilepawns = BBrank7 & pos->colours[BLACK] & pos->pieces[PAWN];
		if (!BBhostilepawns && getrank(pos->Bkingpos) != 7) continue;
		// either hostile pawns on 7th rank or king is on 8th rank
		
		*openingEval += RookOn7th_mg;
		*endgameEval += RookOn7th_eg;
	}
	
	while (BBblackrooks) {
		int square = __builtin_ctzll(BBblackrooks);
		BBblackrooks &= BBblackrooks - 1;
		
		
		U64 BBfilemask = BBfileA << getfile(square);
		
		// rooks on open files
		U64 BBpawnsonfile = BBfilemask & pos->pieces[PAWN];
		U64 BBBpawnsonfile = BBfilemask & pos->pieces[PAWN] & pos->colours[BLACK];
		U64 BBWpawnsonfile = BBfilemask & pos->pieces[PAWN] & pos->colours[WHITE];
		// black rook on open file
		if (BBpawnsonfile == 0) {
			// black rook on open file
			*openingEval -= RookOpenFile_mg;
			*endgameEval -= RookOpenFile_eg;
		}
		if ((BBBpawnsonfile == 0) && (BBWpawnsonfile)) {
			// black rook on semi-open file with white pawns
			*openingEval -= RookSemiOpenFile_mg;
			*endgameEval -= RookSemiOpenFile_eg;
		}
		// rook on same file as queen
		
		U64 BBWqueensonfile = BBfilemask & (pos->pieces[QUEEN] & pos->colours[WHITE]);
		if (BBWqueensonfile) {
			*openingEval -= RookSameFileQueen_mg;
			*endgameEval -= RookSameFileQueen_eg;
		}
		
		// rooks on 7th rank
		
		// rook on 7th rank
		
		if (getrank(square) != 1) continue;
		U64 BBhostilepawns = BBrank2 & pos->colours[WHITE] & pos->pieces[PAWN];
		if (!BBhostilepawns && getrank(pos->Wkingpos) != 0) continue;
		// either hostile pawns on 7th rank or king is on 8th rank
		
		*openingEval -= RookOn7th_mg;
		*endgameEval -= RookOn7th_eg;
	}
}

void evalQueens(struct position *pos, int *openingEval, int *endgameEval) {
	
	// bonus for rooks/queens being on 7th rank
	
	U64 BBwhitequeens = pos->colours[WHITE] & pos->pieces[QUEEN];
	U64 BBblackqueens = pos->colours[BLACK] & pos->pieces[QUEEN];
	
	while (BBwhitequeens) {
		int square = __builtin_ctzll(BBwhitequeens);
		BBwhitequeens &= BBwhitequeens - 1;
		if (getrank(square) != 6) continue;
		// queen on 7th rank
		U64 BBhostilepawns = BBrank7 & pos->colours[BLACK] & pos->pieces[PAWN];
		if (!BBhostilepawns && getrank(pos->Bkingpos) != 7) continue;
		// either hostile pawns on 7th rank or king is on 8th rank
		
		*openingEval += QueenOn7th_mg;
		*endgameEval += QueenOn7th_eg;
	}
	
	while (BBblackqueens) {
		int square = __builtin_ctzll(BBblackqueens);
		BBblackqueens &= BBblackqueens - 1;
		if (getrank(square) != 1) continue;
		// queen on 7th rank
		U64 BBhostilepawns = BBrank2 & pos->colours[WHITE] & pos->pieces[PAWN];
		if (!BBhostilepawns && getrank(pos->Wkingpos) != 0) continue;
		// either hostile pawns on 7th rank or king is on 8th rank
		
		*openingEval -= QueenOn7th_mg;
		*endgameEval -= QueenOn7th_eg;
	}
}
void evalKings(struct position *pos, int *openingEval, int *endgameEval) {
	
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
			*openingEval -= KingPawnlessFlank_mg;
			*endgameEval -= KingPawnlessFlank_eg;
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
			*openingEval += KingPawnlessFlank_mg;
			*endgameEval += KingPawnlessFlank_eg;
		}
	}
	
}
void evalMobility(struct position *pos, int *openingEval, int *endgameEval) {
	
	// Evaluates mobility, king attackers, and NBRQ PST
	
	int kingattackers = 0;
	int kingattacks = 0;
	// white
	
	int idx = 0;
	double centremult = 0.5;
	double hrattackbonus = 0.5;
	struct mobreturn WNmobility = Nmobility(pos,WHITE);
	idx = min(8, max(0, WNmobility.mobility - WNmobility.unsafe * 2 + WNmobility.centre * centremult));
	*openingEval += knightMgMobility[idx];
	*endgameEval += knightEgMobility[idx];
	*openingEval += WNmobility.pstO;
	*endgameEval += WNmobility.pstE;
	*openingEval += WNmobility.homerowsattacks * hrattackbonus;
	kingattacks +=  WNmobility.kingattacks;
	kingattackers += WNmobility.kingattackers;
	
	struct mobreturn WBmobility = Bmobility(pos,WHITE);
	idx = min(13, max(0, WBmobility.mobility - WBmobility.unsafe * 2 + WBmobility.centre * centremult));
	*openingEval += bishopMgMobility[idx];
	*endgameEval += bishopEgMobility[idx];
	*openingEval += WBmobility.pstO;
	*endgameEval += WBmobility.pstE;
	*openingEval += WBmobility.homerowsattacks * hrattackbonus;
	kingattacks += WBmobility.kingattacks;
	kingattackers += WBmobility.kingattackers;
	
	struct mobreturn WRmobility = Rmobility(pos,WHITE);
	idx = min(14, max(0, WRmobility.mobility - WRmobility.unsafe * 2 + WRmobility.centre * centremult));
	*openingEval += rookMgMobility[idx];
	*endgameEval += rookEgMobility[idx];
	*openingEval += WRmobility.pstO;
	*endgameEval += WRmobility.pstE;
	*openingEval += WRmobility.homerowsattacks * hrattackbonus;
	kingattacks += 2 * WRmobility.kingattacks;
	kingattackers += WRmobility.kingattackers;
	
	struct mobreturn WQmobility = Qmobility(pos,WHITE);
	idx = min(27, max(0, WQmobility.mobility - WQmobility.unsafe * 2 + WQmobility.centre * centremult));
	*openingEval += queenMgMobility[idx];
	*endgameEval += queenEgMobility[idx];
	*openingEval += WQmobility.pstO;
	*endgameEval += WQmobility.pstE;
	*openingEval += WQmobility.homerowsattacks * hrattackbonus;
	kingattacks += 4 * WQmobility.kingattacks;
	kingattackers += WQmobility.kingattackers;
	
	*openingEval += safety_table[kingattackers][kingattacks];
	*endgameEval += safety_table[kingattackers][kingattacks];
	
	kingattacks = 0;
	kingattackers = 0;
	// black
	struct mobreturn BNmobility = Nmobility(pos,BLACK);
	idx = min(8, max(0, BNmobility.mobility - BNmobility.unsafe * 2 + BNmobility.centre * centremult));
	*openingEval -= knightMgMobility[idx];
	*endgameEval -= knightEgMobility[idx];
	*openingEval += BNmobility.pstO;
	*endgameEval += BNmobility.pstE;
	*openingEval -= BNmobility.homerowsattacks * hrattackbonus;
	kingattacks += BNmobility.kingattacks;
	kingattackers += BNmobility.kingattackers;
	
	struct mobreturn BBmobility = Bmobility(pos,BLACK);
	idx = min(13, max(0, BBmobility.mobility - BBmobility.unsafe * 2 + BBmobility.centre * centremult));
	*openingEval -= bishopMgMobility[idx];
	*endgameEval -= bishopEgMobility[idx];
	*openingEval += BBmobility.pstO;
	*endgameEval += BBmobility.pstE;
	*openingEval -= BBmobility.homerowsattacks * hrattackbonus;
	kingattacks += BBmobility.kingattacks;
	kingattackers += BBmobility.kingattackers;
	
	struct mobreturn BRmobility = Rmobility(pos,BLACK);
	idx = min(14, max(0, BRmobility.mobility - BRmobility.unsafe * 2 + BRmobility.centre * centremult));
	*openingEval -= rookMgMobility[idx];
	*endgameEval -= rookEgMobility[idx];
	*openingEval += BRmobility.pstO;
	*endgameEval += BRmobility.pstE;
	*openingEval -= BRmobility.homerowsattacks * hrattackbonus;
	kingattacks += 2 * BRmobility.kingattacks;
	kingattackers += BRmobility.kingattackers;
	
	struct mobreturn BQmobility = Qmobility(pos,BLACK);
	idx = min(27, max(0, BQmobility.mobility - BQmobility.unsafe * 2 + BQmobility.centre * centremult));
	*openingEval -= queenMgMobility[idx];
	*endgameEval -= queenEgMobility[idx];
	*openingEval += BQmobility.pstO;
	*endgameEval += BQmobility.pstE;
	*openingEval -= BQmobility.homerowsattacks * hrattackbonus;
	kingattacks += 4 * BQmobility.kingattacks;
	kingattackers += BQmobility.kingattackers;
	
	*openingEval -= safety_table[kingattackers][kingattacks];
	*endgameEval -= safety_table[kingattackers][kingattacks];
}
void evalMinorAttacks(struct position *pos, int *openingEval, int *endgameEval) {
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
		*openingEval += minorAttackBonus_mg[piece];
		*endgameEval += minorAttackBonus_eg[piece];
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
		*openingEval -= minorAttackBonus_mg[piece];
		*endgameEval -= minorAttackBonus_eg[piece];
	}
	
}
void evalMaterialImbalance(struct position *pos, int *openingEval, int *endgameEval) {
	
	int num_BN = __builtin_popcountll(pos->colours[BLACK] & pos->pieces[KNIGHT]);
	int num_BB = __builtin_popcountll(pos->colours[BLACK] & pos->pieces[BISHOP]);
	int num_BR = __builtin_popcountll(pos->colours[BLACK] & pos->pieces[ROOK]);
	int num_BQ = __builtin_popcountll(pos->colours[BLACK] & pos->pieces[QUEEN]);
	int num_WN = __builtin_popcountll(pos->colours[WHITE] & pos->pieces[KNIGHT]);
	int num_WB = __builtin_popcountll(pos->colours[WHITE] & pos->pieces[BISHOP]);
	int num_WR = __builtin_popcountll(pos->colours[WHITE] & pos->pieces[ROOK]);
	int num_WQ = __builtin_popcountll(pos->colours[WHITE] & pos->pieces[QUEEN]);
	
	int whitematval = num_WN * pieceval(KNIGHT) + num_WB * pieceval(BISHOP) + num_WR * pieceval(ROOK) + num_WQ * pieceval(QUEEN);
	int blackmatval = num_BN * pieceval(KNIGHT) + num_BB * pieceval(BISHOP) + num_BR * pieceval(ROOK) + num_BQ * pieceval(QUEEN);
	if (whitematval > blackmatval) {
		double matimb = 1.0 - (blackmatval / whitematval);
		*openingEval += matimb * ImbalanceFactor;
		*endgameEval += matimb * ImbalanceFactor;
	}
	
	else if (blackmatval > whitematval) {
		double matimb = 1.0 - (whitematval / blackmatval);
		*openingEval -= matimb * ImbalanceFactor;
		*endgameEval -= matimb * ImbalanceFactor;
	}
}
int evalEndgame(struct position *pos, int endgameEval) {
	
	// Endgame evaluation for 5 or fewer pieces
	// Taken from Schooner with permission from its author.
	
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
	
	int num_pieces = __builtin_popcountll(pos->colours[WHITE] | pos->colours[BLACK]);
	
	if (num_pieces >= 6) return endgameEval;
	
	if (num_pieces == 5) {
		
		// KNNKx KNNKP can win
		if (num_WN == 2) {
			if (pos->pieces[PAWN] | pos->pieces[QUEEN]) return endgameEval;
			if (num_BN | num_BB | num_BR) return 0;
		}
		if (num_WN == 2) {
			if (pos->pieces[PAWN] | pos->pieces[QUEEN]) return endgameEval;
			if (num_WN | num_WB | num_WR) return 0;
		}
		
		// KRNKR KRBKR
		if (num_WR && num_BR) {
			if (pos->pieces[KNIGHT] | pos->pieces[BISHOP]) return endgameEval >> 4;
		}
		
		//KBNKB KBNKN KBNKR
		if (num_WN && num_WB && (num_BR || (num_BN | num_BB))) return 0;
		if (num_BN && num_BB && (num_WR || (num_WN | num_WB))) return 0;
		
		// KBBKB
		if (num_WB == 2) {
			if (num_BB) return 0;
			
			// KBBKN
			//difficult. avoid cursed wins and blessed losses
			if (num_BN) return endgameEval >> 1;
		}
		if (num_BB == 2) {
			if (num_WB) return 0;
			
			// KBBKN
			//difficult. avoid cursed wins and blessed losses
			if (num_WB) return endgameEval >> 1;
		}
		
		//KRKBP
		
		if (num_WR && (num_BN | num_BB) && num_BP) {
			return min(endgameEval, 0);
		}
		if (num_BR && (num_WN | num_WB) && num_WP) {
			return max(endgameEval, 0);
		}
	}
	
	if (num_pieces == 4) {
		// KNKP KBKP
		if ((num_WN | num_WB) == 1 && num_BP == 1) {
			return min(endgameEval, 0);
		}
		if ((num_BN | num_BB) == 1 && num_WP == 1) {
			return max(endgameEval, 0);
		}
	}
	if (num_pieces == 3) {
		if (pos->pieces[ROOK] | pos->pieces[QUEEN]) {
			if (BBkingLookup[pos->Wkingpos] & BBkingLookup[pos->Bkingpos]) {
				return endgameEval << 1;
			}
			return endgameEval;
		}
	}
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
	
	// side to move bonus
	
	if (pos->tomove == WHITE) {
		openingEval += SideToMove_mg;
		endgameEval += SideToMove_eg;
	}
	else {
		openingEval -= SideToMove_mg;
		endgameEval -= SideToMove_eg;
	}
	
	// penalty for not being castled and not having castling rights
	
	if (!pos->Wcastled && !pos->WcastleQS && !pos->WcastleKS) {
		openingEval -= 30;
	}
	if (!pos->Bcastled && !pos->BcastleQS && !pos->BcastleKS) {
		openingEval += 30;
	}
	
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
	
	evalKRmate(pos, &openingEval, &endgameEval);
	
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
