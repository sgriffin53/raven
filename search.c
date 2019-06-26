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
#include <math.h>

#define MATE_SCORE 9999
#define MAX_MOVES 2048

int reduction(const struct move *move, const int depthleft, char cappiece, int legalmoves, int incheck, int givescheck, int ply) {
	assert(move);
	assert(depthleft >= 0);
	if ((!incheck) && (legalmoves > 4) && (cappiece == '0') && (depthleft >= 3) && (move->prom == 0) && (!givescheck)) {
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
void clearHistory() {
	for (int i = 0;i < 64;i++) {
		for (int j = 0;j < 64;j++) {
			history[WHITE][i][j] = 0;
			history[BLACK][i][j] = 0;
			butterfly[WHITE][i][j] = 0;
			butterfly[BLACK][i][j] = 0;
		}
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

	// delta pruning
	const int BIG_DELTA = 975;
	if (standpat < alpha - BIG_DELTA) {
		nodesSearched++;
		return alpha;
	}

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

int alphaBeta(struct position *pos, int alpha, int beta, int depthleft, int nullmove, int ply, struct move *pv, clock_t endtime) {
	assert(pos);
	assert(alpha >= -MATE_SCORE && beta <= MATE_SCORE);
	assert(beta > alpha);
	assert(depthleft >= 0);
	
	if (clock() >= endtime) {
		return -MATE_SCORE;
	}
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
	TTmove = *pv;
	int origAlpha = alpha;
	
	/*
	// null move pruning - doesn't work
	// if (!nullmove && !isEndgame(pos) && !incheck && !(alpha != beta - 1)) {
	
	if (!nullmove && !isEndgame(pos) && !incheck && ply != 0) {
		pos->tomove = !pos->tomove;
		int val = -alphaBeta(pos,-beta,-beta+1, depthleft - 1 - 2, 1, ply + 1, pv, endtime);
		pos->tomove = !pos->tomove;
		if (val >= beta) return beta;
	}
	*/

	U64 hash;
	if (currenthash == 0) {
		hash = generateHash(pos);
	}
	else hash = currenthash;
	struct TTentry TTdata = getTTentry(&TT,hash);
	if (TTdata.hash == hash) {
		if (TTdata.depth == depthleft) {
			int flag = TTdata.flag;
			int score = TTdata.score;
			
			if (flag == EXACT) {
				*pv = TTdata.bestmove;
				return score;
			}
			else if (flag == LOWERBOUND) {
				alpha = max(score, alpha);
			}
			else if (flag == UPPERBOUND) {
				beta = min(beta, score);
			}
			if (alpha >= beta) {
				*pv = TTdata.bestmove;
				return score;
			}
			
		}
		TTmove = TTdata.bestmove;
	}

	//U64 hash = generateHash(pos);
	int f_prune = 0;
	int fmargin[4] = { 0, 200, 300, 500 };

	if (depthleft <= 3
	&&  !incheck
	&&   abs(alpha) < 9000
	&&   taperedEval(pos) + fmargin[depthleft] <= alpha)
		 f_prune = 1;
	struct move moves[MAX_MOVES];
	const int num_moves = genMoves(pos,moves);
	sortMoves(pos,moves,num_moves,TTmove,ply);
	int score;
	int bestscore = INT_MIN;
	int legalmoves = 0;
	int fullwindow = 1;
	for (int i = 0;i < num_moves;i++) {
		char cappiece = getPiece(pos,moves[i].to);
		makeMove(&moves[i],pos);
		pos->tomove = !pos->tomove;
		if (isCheck(pos)) {
			unmakeMove(pos);
			continue;
		}
		pos->tomove = !pos->tomove;

		int givescheck = isCheck(pos);
		legalmoves++;
		if (f_prune
		&& legalmoves > 1
		&&  cappiece == '0'
		&&  moves[i].prom == 0
		&& !givescheck
		&& ply != 0) {
		//&&  !isAttacked(pos,kingpos,pos->tomove)) {
			unmakeMove(pos);
			continue;
		}
		int r = reduction(&moves[i], depthleft, cappiece, legalmoves, incheck, givescheck, ply);
		

		// PV search - doesn't work
		/*
		if (fullwindow) {
			score = -alphaBeta(pos, -beta, -alpha, depthleft - 1 - r, 0, ply + 1, pv, endtime);
			if (r > 0 && score > alpha) {
				score = -alphaBeta(pos, -beta, -alpha, depthleft - 1, 0, ply + 1, pv, endtime);
			}
		}
		else {
			score = -alphaBeta(pos, -alpha-1, -alpha, depthleft - 1 - r, 0, ply + 1, pv, endtime);
			if (r > 0 && score > alpha) {
				score = -alphaBeta(pos, -alpha-1, -alpha, depthleft - 1, 0, ply + 1, pv, endtime);
			}
			if (score > alpha && score < beta) {
				score = -alphaBeta(pos, -beta, -alpha, depthleft - 1, 0, ply + 1, pv, endtime);
				if (r > 0 && score > alpha) {
					score = -alphaBeta(pos, -beta, -alpha, depthleft - 1, 0, ply + 1, pv, endtime);
				}
			}
		}
		 */
		// Search
		
		score = -alphaBeta(pos, -beta, -alpha, depthleft - 1 - r, 0, ply + 1, pv, endtime);
		
		// Redo search
		if (r > 0 && score > alpha) {
			score = -alphaBeta(pos, -beta, -alpha, depthleft - 1, 0, ply + 1, pv, endtime);
		}
		
		//score = -alphaBeta(pos, -beta, -alpha, depthleft - 1, 0, endtime);
		unmakeMove(pos);
		if (score > alpha) {
			fullwindow = 0;
		}
		//for (int i = 0;i < ply;i++) printf("\t");
		//printf("depth %d - searching move: %s\n",depthleft,movetostr(moves[i]));
		if (score > bestscore) {
			//for (int i = 0;i < ply;i++) printf("\t");
			//printf("score: %d - bestscore: %d\n",score,bestscore);
			bestscore = score;
			bestmove = moves[i];
			//for (int i = 0;i < ply;i++) printf("\t");
			//printf("depth %d - new best move: %s\n",depthleft,movetostr(bestmove));
			//if (ply == 0) *pv = bestmove;
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
				history[pos->tomove][moves[i].from][moves[i].to] += pow(2,depthleft);
			}
			break;
		}
		else {
			// no beta cut off
			if (cappiece == '0') {
				butterfly[pos->tomove][moves[i].from][moves[i].to] += pow(2,depthleft);
			}
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
	
	addTTentry(&TT, hash, depthleft, newflag, bestmove, bestscore);
	*pv = bestmove;
	assert(bestmove.to >= 0 && bestmove.to <= 63 && bestmove.from >= 0 && bestmove.from <= 63);
	return bestscore;
}
struct move search(struct position pos, int searchdepth, int movetime) {
	assert(searchdepth >= 0);
	assert(movetime > 0);
	// Reset stats
	nodesSearched = 0;
	numbetacutoffs = 0;
	numinstantbetacutoffs = 0;

	// Result
	struct move bestmove;

	// Timing code
	const clock_t begin = clock();
	const clock_t endtime = clock() + (movetime / 1000.0 * CLOCKS_PER_SEC);

	// Movegen
	struct move moves[MAX_MOVES];
	const int num_moves = genMoves(&pos, moves);

	// Only move
	//if (num_moves == 1) return moves[0];
	clearHistory();
	// Calculate hash
	//const U64 hash = generateHash(&pos);
	int lastscore = 0;
	
	// set a PV in case one doesn't get set due to threefold or 50 move rule returning without setting a pv
	struct move pv;
	
	for (int i = 0;i < num_moves;i++) {
		makeMove(&moves[i],&pos);
		pos.tomove = !pos.tomove;
		int incheck = isCheck(&pos);
		if (incheck) {
			unmakeMove(&pos);
			continue;
		}
		pos.tomove = !pos.tomove;
		unmakeMove(&pos);
		pv = moves[i];
		break;
	}
	
	for(int d = 1; d <= searchdepth; ++d) {
		
		/*
		int score = 0;
		
		if (d <= 3) {
			score = alphaBeta(&pos, -MATE_SCORE, MATE_SCORE, d, 0, 0, &pv, endtime);
		}
		else {
			int b[6] = {50, 200, MATE_SCORE};
			for (int i = 0;i < 3;i++) {
				int min = lastscore -b[i];
				int max = lastscore + b[i];
				score = alphaBeta(&pos, min, max, d, 0, 0, &pv, endtime);
				if (-b[i] < score && score < b[i]) break;
				//if (score > -b[i] && score < b[i]) break;
			}
		}
		 
		lastscore = 0;
		*/
		const int score = alphaBeta(&pos, -MATE_SCORE, MATE_SCORE, d, 0, 0, &pv, endtime);
		
		// Ignore the result if we ran out of time
		if (d > 1 && clock() >= endtime) {
			break;
		}

		// Get our TT entry
		//const struct TTentry TTdata = getTTentry(&TT, hash);
		//assert(TTdata.hash == hash);

		// Check pv
		
		#ifndef DNDEBUG
		int found = 0;
		for(int i = 0; i < num_moves; ++i) {
			if (moves[i].from == pv.from &&
				moves[i].to == pv.to &&
				moves[i].prom == pv.prom &&
				moves[i].cappiece == pv.cappiece) {
				found = 1;
				break;
			}
		}
		
		assert(found);
		#endif
		 

		// Update results
		//bestmove = TTdata.bestmove;
		bestmove = pv;
		const double time_spent = (double)(clock() - begin) / CLOCKS_PER_SEC;
		int nps = nodesSearched / time_spent;
		// Info string
		printf("info");
		printf(" depth %i", d);
		printf(" nodes %" PRIu64, nodesSearched);
		printf(" time %i", (int)(time_spent*1000));
		printf(" nps %i", nps);
		printf(" score cp %i", score);
		printf(" pv %s", movetostr(bestmove));
		printf("\n");
	}


	printf("bestmove %s\n", movetostr(bestmove));


	return bestmove;
}
/*
// old unrolled root search

struct move search(struct position pos, int searchdepth,int movetime) {
	nodesSearched = 0;
	struct move moves[MAX_MOVES];
	struct move bestmove;
	clock_t begin = clock();
	double time_spent;
	clock_t endtime = clock() + (movetime / 1000.0 * CLOCKS_PER_SEC);
	int nps;
	struct move TTmove = {.to=-1,.from=-1,.prom=-1,.cappiece=-1};
	int num_moves = genMoves(&pos,moves);
	//sortMoves(&pos,moves,num_moves,TTmove,0);
	int legalmoves = 0;
	int timeElapsed = 0;
	int legalmoveidx = 0;
	int bestScore = INT_MIN;
	clearHistory();
	for (int curdepth = 1; (curdepth < searchdepth+1 && timeElapsed == 0);curdepth++) {
		int bestScore = INT_MIN;
		clearKillers(128);
		//clearHistory();

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
*/