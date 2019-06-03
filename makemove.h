#ifndef MAKEMOVE_H
#define MAKEMOVE_H

#include "move.h"

void makeMove(const struct move *move, struct position *pos);
void unmakeMove(struct position *pos);
void makeMovestr(const char move[], struct position *pos);

#endif