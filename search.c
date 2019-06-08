#include "search.h"
#include "position.h"
#include "eval.h"
#include "attacks.h"
#include "makemove.h"
#include "movegen.h"
#include "globals.h"
#include "draw.h"
#include "sort.h"
#include "misc.h"
#include "TT.h"
#include <time.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#define MATE_SCORE 9999
#define MAX_MOVES 2048

int reduction(const struct move *move, const int depthleft, char cappiece, int legalmoves, int incheck) {
	assert(move);
	assert(depthleft >= 0);

	if ((!incheck) && (legalmoves > 0) && (cappiece == '0') && (depthleft >= 3) && (move->prom == 0)) {
		if (depthleft >= 6) return 2;
		return 1;
	}

	return 0;
}

void clearKillers(int ply) {
	struct move nomove = {.to=-1,.from=-1,.prom=-1,.cappiece=-1};
	for (int i = 0;i < ply;i++) {
		killers[ply][0] = nomove;
		killers[ply][1] = nomove;
	}
}

int qSearch(struct position *pos, int alpha, int beta, int ply, clock_t endtime) {
	assert(pos);
	assert(alpha >= -MATE_SCORE && beta <= MATE_SCORE);
	//int incheck;
	//int score;
	//int kingpos;
	if (clock() >= endtime) {
		return -MATE_SCORE;
	}
	//int ispawnless = isPawnless(pos);
	const int standpat = taperedEval(pos);
	if (standpat >= beta) {
		nodesSearched++;
		return beta;
	}
/*
	// delta pruning
	const int BIG_DELTA = 900;
	if (standpat < alpha - BIG_DELTA) {
		nodesSearched++;
		return alpha;
	}
*/
	if (alpha < standpat) alpha = standpat;

	struct move TTmove = {.to=-1,.from=-1,.prom=-1,.cappiece=-1};
	struct move moves[MAX_MOVES];
	const int num_moves = genMoves(pos,moves);
	
/*
	U64 hash;
	if (currenthash != 0) {
		hash = currenthash;
	}
	else hash = generateHash(pos);
	struct TTentry TTdata = getTTentry(&TT,hash);
	if (TTdata.hash == hash) {
		TTmove = TTdata.bestmove;
	}
	 */
	sortMoves(pos,moves,num_moves,TTmove, ply);
	
	//struct move TTmove = {.to=-1,.from=-1,.prom=-1,.cappiece=-1};
	//sortMoves(pos,moves,num_moves,TTmove);

	for (int i = 0;(i < num_moves);i++) {
		//clock_t end = clock();
		//double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
		//int time_spentms = (int)(time_spent * 1000);
		char cappiece = getPiece(pos,moves[i].to);
		if (cappiece == '0' && moves[i].prom == 0) continue;

		makeMove(&moves[i],pos);

		// check if move is legal (doesn't put in check)
		pos->tomove = !pos->tomove;
		const int incheck = isCheck(pos);
		if (incheck) {
			unmakeMove(pos);
			continue;
		}
		pos->tomove = !pos->tomove;
		currenthash = 0;
		const int score = -qSearch(pos,-beta,-alpha, ply + 1, endtime);

		nodesSearched++;

		unmakeMove(pos);

		if (score >= beta) {
			return beta;
		}
		if (score > alpha) alpha = score;
	}
	return alpha;
}

int alphaBeta(struct position *pos, int alpha, int beta, int depthleft, int nullmove, int ply, clock_t endtime) {
	assert(pos);
	assert(alpha >= -MATE_SCORE && beta <= MATE_SCORE);
	assert(beta > alpha);
	assert(depthleft >= 0);
	
	if (clock() >= endtime) {
		return -MATE_SCORE;
	}
	//ss->ply++;
	nodesSearched += 1;
	currenthash = 0;
	if (isThreefold(pos)) return 0;
	if (pos->halfmoves >= 100) return 0;
	int incheck = isCheck(pos);
	if (incheck) depthleft++;
	if (depthleft <= 0) {
		return qSearch(pos, alpha, beta, ply + 1, endtime);
	}
	struct move bestmove;
	struct move TTmove = {.to=-1,.from=-1,.prom=-1,.cappiece=-1};
	int origAlpha = alpha;
/*
	U64 hash;
	if (currenthash == 0) {
		hash = generateHash(pos);
	}
	else hash = currenthash;
	struct TTentry TTdata = getTTentry(&TT,hash);
	if (TTdata.hash == hash) {
		if (TTdata.depth >= depthleft) {
			int flag = TTdata.flag;
			int score = TTdata.score;
			if (flag == EXACT) {
				return score;
			}
			else if (flag == LOWERBOUND) {
				alpha = max(score, alpha);
			}
			else if (flag == UPPERBOUND) {
				beta = min(beta, score);
			}
			if (alpha >= beta) {
				return score;
			}
		}
		TTmove = TTdata.bestmove;
	}
	 */
	int f_prune = 0;
	int fmargin[4] = { 0, 200, 300, 500 };

	if (depthleft <= 3
	&&  !incheck
	&&   abs(alpha) < 9000
	&&   taperedEval(pos) + fmargin[depthleft] <= alpha)
		 f_prune = 1;
	struct move moves[MAX_MOVES];
	const int num_moves = genMoves(pos,moves);
	//struct searchstack *ss;
	sortMoves(pos,moves,num_moves,TTmove,ply);
	int score;
	int bestscore = INT_MIN;
	int legalmoves = 0;
	
	for (int i = 0;i < num_moves;i++) {
		char cappiece = getPiece(pos,moves[i].to);
		makeMove(&moves[i],pos);
		pos->tomove = !pos->tomove;
		if (isCheck(pos)) {
			unmakeMove(pos);
			continue;
		}
		pos->tomove = !pos->tomove;
		
		int kingpos;
		if (pos->tomove == WHITE) kingpos = pos->Wkingpos;
		else kingpos = pos->Bkingpos;
		if (f_prune
		&& legalmoves
		&&  cappiece == '0'
		&&  moves[i].prom == 0) {
		//&&  !isAttacked(pos,kingpos,pos->tomove)) {
			unmakeMove(pos);
			continue;
		}
		
		legalmoves++;
		const int r = reduction(&moves[i], depthleft, cappiece, legalmoves, incheck);
		
		// Search
		int score = -alphaBeta(pos, -beta, -alpha, depthleft - 1 - r, 0, ply + 1, endtime);

		// Redo search
		if (r > 0 && score > alpha) {
			score = -alphaBeta(pos, -beta, -alpha, depthleft - 1, 0, ply + 1, endtime);
		}
		//score = -alphaBeta(pos, -beta, -alpha, depthleft - 1, 0, endtime);
		unmakeMove(pos);
		if (score >= bestscore) {
			bestscore = score;
			bestmove = moves[i];
		}
		if (bestscore >= alpha) {
			alpha = bestscore;
		}
		if (alpha >= beta) {
			numbetacutoffs++;
			if (legalmoves == 1) numinstantbetacutoffs++;
			if (cappiece == '0') {
				killers[ply][1] = killers[ply][0];
				killers[ply][0] = moves[i];
			}
			break;
		}
	}
	if (legalmoves == 0) {
		// no legal moves
		if (incheck) {
			// side to move is in checkmate
			return -MATE_SCORE;
		}
		else {
			// stalemate
			return 0;
		}
	}

	int newflag;
	if (bestscore <= origAlpha) {
		newflag = UPPERBOUND;
	}
	else if (bestscore >= beta) {
		newflag = LOWERBOUND;
	}
	else {
		newflag = EXACT;
	}
	//addTTentry(&TT, hash, depthleft, newflag, bestmove, bestscore);
	return bestscore;
}
struct move search(struct position pos, int searchdepth,int movetime) {
	nodesSearched = 0;
	struct move moves[MAX_MOVES];
	struct move bestmove;
	clock_t begin = clock();
	double time_spent;
	clock_t endtime = clock() + (movetime / 1000.0 * CLOCKS_PER_SEC);
	int nps;
	int num_moves = genMoves(&pos,moves);
	int legalmoves = 0;
	int timeElapsed = 0;
	int legalmoveidx = 0;
	int bestScore = INT_MIN;
	for (int curdepth = 1; (curdepth < searchdepth+1 && timeElapsed == 0);curdepth++) {
		int bestScore = INT_MIN;
		clearKillers(128);
		for (int i = 0;i < num_moves;i++) {
			makeMove(&moves[i],&pos);
			pos.tomove = !pos.tomove;
			int incheck = isCheck(&pos);
			if (incheck) {
				unmakeMove(&pos);
				continue;
			}
			pos.tomove = !pos.tomove;
			legalmoves++;
			legalmoveidx = i;
			clock_t end = clock();
			time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
			if (clock() >= endtime) {
				//bestmove = lastbestmove;
				unmakeMove(&pos);
				timeElapsed = 1;
				break;
			}
			int curscore = -alphaBeta(&pos,-MATE_SCORE,MATE_SCORE,curdepth-1,0, 0, endtime);
			if (curscore == MATE_SCORE) {
				end = clock();
				time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
				nps = nodesSearched / time_spent;
				printf("info depth %d nodes %" PRIu64 " time %d nps %d score mate %d pv %s\n",(curdepth),nodesSearched,((int)(time_spent*1000)),nps,curdepth,movetostr(moves[i]));
				printf("bestmove %s\n",movetostr(moves[i]));

				unmakeMove(&pos);
				return moves[i];
			}
			//printf("cur %d best %d move %s\n",curscore,bestScore,movetostr(moves[i]));
			if (curscore > bestScore) {
				bestScore = curscore;
				bestmove = moves[i];
			}

			unmakeMove(&pos);

			nps = nodesSearched / time_spent;
		}
		if (legalmoves == 1) bestmove = moves[legalmoveidx];
		if (nodesSearched == 0) bestmove = moves[legalmoveidx];
		if (clock() >= endtime) {break;}
		struct move newmoves[MAX_MOVES];
		int num_newmoves = 1;
		newmoves[0] = bestmove;
		int j = 1;
		for (int i =0;i < num_moves;i++) {
			// if bestmove != moves[i]
			if ( (bestmove.to != moves[i].to) || (bestmove.from != moves[i].from) || (bestmove.prom != moves[i].prom) ) {
				newmoves[j] = moves[i];
				j++;
				num_newmoves++;
			}
		}
		moves[0] = bestmove;
		num_moves = 1;
		for (int i=1;i<num_newmoves;i++) {
			// if bestmove != newmoves[i]
			if ( (bestmove.to != newmoves[i].to) || (bestmove.from != newmoves[i].from) || (bestmove.prom != newmoves[i].prom)) {
			moves[num_moves] = newmoves[i];
			num_moves++;
			}
		}
		printf("info depth %d nodes %" PRIu64 " time %d nps %d score cp %d pv %s\n",(curdepth),nodesSearched,((int)(time_spent*1000)),nps,bestScore,movetostr(bestmove));
	}
	printf("bestmove %s\n",movetostr(bestmove));
	return bestmove;
}