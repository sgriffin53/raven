#ifndef SEARCH_H
#define SEARCH_H

#include <time.h>
#include "position.h"
#include "move.h"

struct pvline {
	struct move moves[1024];
	int size;
};
int SEEcapture(struct position *pos, int from, int to, int side);
int SEE(struct position *pos, int square, int side);
struct move get_smallest_attacker(struct position *pos, int square, int side);
int alphaBeta(struct position *pos, int alpha, int beta, int depthleft, int nullmove, int ply, struct move *pv, clock_t endtime);
struct move search(struct position pos, int searchdepth,int movetime);
int reduction(const struct move *move, const int depthleft, char cappiece, int legalmoves, int incheck, int givescheck, int ply);
int qSearch(struct position *pos, int alpha, int beta, int ply, clock_t endtime);
void clearKillers(int ply);
void clearHistory();

#endif