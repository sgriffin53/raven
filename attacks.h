#ifndef ATTACKS_H
#define ATTACKS_H

#include "position.h"

int isAttacked(const struct position *pos, int square, int colour);
int isCheck(const struct position *pos);

#endif
