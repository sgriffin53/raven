#ifndef SEARCH_H
#define SEARCH_H

#include <time.h>
#include "position.h"
#include "move.h"

void clearKillers(int ply);
int alphaBeta(struct position *pos, int alpha, int beta, int depthleft, int nullmove, int ply, clock_t endtime);
struct move search(struct position pos, int searchdepth,int movetime);
int reduction(const struct move *move, const int depthleft, char cappiece, int legalmoves, int incheck);
int qSearch(struct position *pos, int alpha, int beta, int ply, clock_t endtime);

#endif