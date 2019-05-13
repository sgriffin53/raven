#ifndef SEARCH_H
#define SEARCH_H

#include <time.h>
#include "position.h"

#define MATE_SCORE 99999

struct move search(struct position pos, int searchdepth, int movetime);
int alphaBeta(struct position *pos, int alpha, int beta, int depthleft, int nullmove, clock_t endtime);
int qSearch(struct position *pos, int alpha, int beta, clock_t endtime, struct move TTmove);

#endif
