#ifndef EVAL_H
#define EVAL_H

#include "position.h"

int pieceval_eg(const char inpiece);
int pieceval_mg(const char inpiece);
int pieceval(const char inpiece);
int evalBoard(struct position *pos);
int taperedEval(struct position *pos);
int isEndgame(struct position *pos);
int mobility(struct position *pos, int side);
int Nmobility(struct position *pos, int side);
int Bmobility(struct position *pos, int side);
int Rmobility(struct position *pos, int side);
int Qmobility(struct position *pos, int side);
int minorAttackBonus_mg(char piece);
int minorAttackBonus_eg(char piece);

#endif