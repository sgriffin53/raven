#ifndef EVAL_H
#define EVAL_H

#include "position.h"

int pieceval(const char inpiece);
int evalBoard(struct position *pos);
int taperedEval(struct position *pos);
int isEndgame(struct position *pos);
int mobility(struct position *pos, int side);

#endif