#ifndef TUNE_EVAL_H
#define TUNE_EVAL_H
#include "../position.h"

int tuneParams[1024];
int tuneParamsInit[1024];
char tuneParamsName[1024][128];

int tune_eval(struct position *pos);
int tune_minorAttackBonus_mg[6];
int tune_minorAttackBonus_eg[6];
int tune_knightMgMobility[9];


int tune_minorAttackBonus_mg[6];
int tune_minorAttackBonus_eg[6];

// Minor piece attack bonus

int minorAttackBonus_mg[6];
int minorAttackBonus_eg[6];

// Piece mobility

int tune_knightMgMobility[9];

// (10 * x Pow 0.5) - 15};

int tune_knightEgMobility[9];
// (20 * x Pow 0.5) - 30};

int tune_bishopMgMobility[14];

// (14 * x Pow 0.5) - 25};

int tune_bishopEgMobility[14];

// (28 * x Pow 0.5) - 50};

int tune_rookMgMobility[15];

// (6 * x Pow 0.5) - 10};

int tune_rookEgMobility[15];

// (28 * x Pow 0.5) - 50};

int tune_queenMgMobility[28];

// (4 * x Pow 0.5) - 10};

int tune_queenEgMobility[28];

// (20 * x Pow 0.5) - 50};

int tune_piecevalues[7];

const int tune_arrCenterManhattanDistance[64];

int tune_pieceval(int inpiece);
struct mobreturn tune_Nmobility(struct position *pos, int side);
struct mobreturn tune_Bmobility(struct position *pos, int side);
struct mobreturn tune_Rmobility(struct position *pos, int side);
struct mobreturn tune_Qmobility(struct position *pos, int side);

int tune_isEndgame(struct position *pos);
void setParams();
int tune_pieceval_mg(int inpiece);
int tune_pieceval_eg(int inpiece);

#endif