#ifndef EVAL_H
#define EVAL_H

#include "position.h"
struct mobreturn {
	int mobility;
	int kingattacks;
	int kingattackers;
	int pstO;
	int pstE;
	int unsafe;
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
int minorAttackBonus_mg(char piece);
int minorAttackBonus_eg(char piece);
int count_attackers(struct position *pos, int square, int side);

#endif