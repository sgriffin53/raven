#ifndef MAKEMOVE_H
#define MAKEMOVE_H

#include "position.h"
#include "move.h"

void makeMove(const struct move *move, struct position *pos);
void unmakeMove(const struct move *move, struct position *pos);
void makeMovestr(const char move[], struct position *pos);

#endif