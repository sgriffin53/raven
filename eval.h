#ifndef EVAL_H
#define EVAL_H

#include "position.h"
struct mobreturn {
	int mobility;
	int mobility2; //number of squares that can be reached in 2 moves but not 1
	int kingattacks;
	int kingattackers;
	int pstO;
	int pstE;
	int unsafe;
};
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
int pieceval_eg(const char inpiece);
int pieceval_mg(const char inpiece);
int pieceval(const char inpiece);
int evalBoard(struct position *pos);
int taperedEval(struct position *pos);
int taperedEval_old(struct position *pos);
int isEndgame(struct position *pos);
int mobility(struct position *pos, int side);
struct mobreturn Nmobility(struct position *pos, int side);
struct mobreturn Bmobility(struct position *pos, int side);
struct mobreturn Rmobility(struct position *pos, int side);
struct mobreturn Qmobility(struct position *pos, int side);
//int minorAttackBonus_mg(char piece);
//int minorAttackBonus_eg(char piece);
int minorAttackBonus_mg[6];
int minorAttackBonus_eg[6];
int count_attackers(struct position *pos, int square, int side);
int blockage(int Wkingpos, int Bkingpos, U64 wPawns, U64 bPawns);

void evalMaterial(struct position *pos, int *openingEval, int *endgameEval);
void evalKPPST(struct position *pos, int *openingEval, int *endgameEval);
void evalPawns(struct position *pos, int *openingEval, int *endgameEval);
void evalKnights(struct position *pos, int *openingEval, int *endgameEval);
void evalBishops(struct position *pos, int *openingEval, int *endgameEval);
void evalKRmate(struct position *pos, int *openingEval, int *endgameEval);
void evalRooks(struct position *pos, int *openingEval, int *endgameEval);
void evalQueens(struct position *pos, int *openingEval, int *endgameEval);
void evalKings(struct position *pos, int *openingEval, int *endgameEval);
void evalMobility(struct position *pos, int *openingEval, int *endgameEval);
void evalMinorAttacks(struct position *pos, int *openingEval, int *endgameEval);
void evalMaterialImbalance(struct position *pos, int *openingEval, int *endgameEval);
int finalEval(struct position *pos, int *openingEval, int *endgameEval);


#endif