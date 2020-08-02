#ifndef SORT_H
#define SORT_H

#include <ctype.h>
#include <stdio.h>

#include "position.h"
#include "move.h"

int capval(char piece);

int mvvlva(char piece, char cappiece);

void sortMoves(struct position *pos, struct move *moves, const int num_moves, struct move TTmove, int ply);
int sortScore(struct position *pos, struct move *move, struct move TTmove, int ply);

#endif
