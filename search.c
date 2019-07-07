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
	int origAlpha = alpha;
	struct move TTmove = {.to=-1,.from=-1,.prom=-1,.cappiece=-1};
	U64 hash;
	if (currenthash == 0) {
		hash = generateHash(pos);
	}
	else hash = currenthash;
	struct TTentry TTdata = getTTentry(&TT,hash);
	if (TTdata.hash == hash) {
		int flag = TTdata.flag;
		int score = TTdata.score;
		if (flag == EXACT
			|| (flag == LOWERBOUND && score >= beta)
			|| (flag == UPPERBOUND && score <= alpha)) {
				return score;
		}
		TTmove = TTdata.bestmove;
	}
	//int ispawnless = isPawnless(pos);
	const int standpat = taperedEval(pos);
	if (standpat >= beta) {
		nodesSearched++;
		//addTTentry(&TT, hash, 0, LOWERBOUND, TTmove, beta);
		return beta;
	}

	// delta pruning
	const int BIG_DELTA = 975;
	if (standpat < alpha - BIG_DELTA) {
		nodesSearched++;
		return alpha;
	}

	if (alpha < standpat) alpha = standpat;

	struct move moves[MAX_MOVES];
	const int num_moves = genMoves(pos,moves);
	int origischeck = isCheck(pos);
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
	struct move bestmove = {.to=-1,.from=-1,.prom=-1,.cappiece=-1};;
	
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
		
		/*
		int delta = standpat + 120;
		
		if (!isCheck(pos) && delta > -MATE_SCORE && delta + pieceval(cappiece) <= alpha) {
			continue;
		}
		 */
		
		currenthash = 0;
		const int score = -qSearch(pos,-beta,-alpha, ply + 1, endtime);

		nodesSearched++;
		U64 newhash;
		if (score >= beta) newhash = generateHash(pos);
		
		unmakeMove(pos);

		if (score >= beta) {
			if (TTdata.hash != hash) addTTentry(&TT, hash, 0, LOWERBOUND, moves[i], beta);
			return beta;
		}
		if (score > alpha) {
			alpha = score;
			bestmove = moves[i];
		}
	}
	if (bestmove.from != -1 && TTdata.hash != hash) {
		addTTentry(&TT, hash, 0, EXACT, bestmove, alpha);
	}
	return alpha;
}



int alphaBeta(struct position *pos, int alpha, int beta, int depthleft, int nullmove, int ply, struct move *pv, clock_t endtime) {
	assert(pos);
	assert(alpha >= -MATE_SCORE && beta <= MATE_SCORE);
	assert(beta > alpha);
	assert(depthleft >= 0);
	
	if (depthleft <= 0) depthleft = 0;
	if (clock() >= endtime) {
		return -MATE_SCORE;
	}
	nodesSearched += 1;
	currenthash = 0;
	if (isThreefold(pos)) return 0;
	if (pos->halfmoves >= 100) return 0;
	int incheck = isCheck(pos);
	if (incheck) depthleft++;
	
	struct move bestmove;
	struct move TTmove = {.to=-1,.from=-1,.prom=-1,.cappiece=-1};
	//TTmove = *pv;
	int origAlpha = alpha;
	int origBeta = beta;
	
	
	
	
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

	if (depthleft <= 0) {
		return qSearch(pos, alpha, beta, ply + 1, endtime);

	}
	
	int staticeval = taperedEval(pos);
	if (!incheck && (ply > 0) && depthleft < 7 && staticeval - 90 * depthleft >= beta && staticeval < 9999) {
		return staticeval;
	}
	// null move pruning
	
	// if (!nullmove && !isEndgame(pos) && !incheck && !(alpha != beta - 1)) {
	/*
	if (!nullmove && !incheck && ply != 0 && depthleft >= 3 && !isEndgame(pos)) {
		const int orighalfmoves = pos->halfmoves;
		const int origepsquare = pos->epsquare;
		pos->tomove = !pos->tomove;
		pos->halfmoves = 0;
		pos->epsquare = -1;
		posstack[posstackend] = *pos;
		posstackend++;
		const int val = -alphaBeta(pos,-beta,-beta+1, depthleft - 1 - 2, 1, ply + 1, pv, endtime);
		pos->tomove = !pos->tomove;
		pos->halfmoves = orighalfmoves;
		pos->epsquare = origepsquare;
		posstackend--;
		if (val >= beta) {
			//int verification = alphaBeta(pos,beta - 1,beta, depthleft - 1 - 3, 1, ply + 1, pv, endtime); // alpha_beta(p, md, beta - 1, beta, d, false, false);
			
			//if (verification >= beta) return beta;
			return beta;
		}
	}
	 */
	// another attempt at null move pruning - doesn't work - gives illegal pvs
	/*
	if (TTmove.from == -1 && !incheck && !nullmove && !isEndgame(pos) && staticeval >= beta && depthleft >= 2) {
		pos->tomove = !pos->tomove;
		int val = -alphaBeta(pos,-beta,-beta+1, depthleft - 1 - 0, 1, ply + 1, pv, endtime);
		pos->tomove = !pos->tomove;
		if (val >= beta) {
			int verification = -alphaBeta(pos,beta - 1,beta, depthleft - 0 - 0, 1, ply + 1, pv, endtime); // alpha_beta(p, md, beta - 1, beta, d, false, false);
			
			if (verification >= beta) return beta;
		}
	}
	*/
 	//if (TTmove.from == -1) TTmove = *pv;
	
	// razoring
	/*
	const int razoring_margin[4] = {0, 333, 353, 324};
	if (!incheck && depthleft < 3 && staticeval <= alpha - razoring_margin[depthleft] && ply != 0) {
		if (depthleft <= 1) {
			return qSearch(pos, alpha, beta, ply + 1, endtime);
		}
		int margin = alpha - razoring_margin[depthleft];
		int qvalue = qSearch(pos, margin, margin + 1, ply + 1, endtime);
		if (qvalue <= margin) {
			return qvalue;
		}
	}
	 */
	/*
	if (!incheck && depthleft <= 2 && taperedEval(pos) <= alpha - 300) {
		if (depthleft == 1) return qSearch(pos, alpha, beta, ply + 1, endtime);
		int rWindow = alpha - 300;
		int value = qSearch(pos,rWindow,rWindow+1,ply + 1, endtime);
		if (value <= rWindow) return value;
	}
	*/
	//U64 hash = generateHash(pos);
	int f_prune = 0;
	int fmargin[4] = { 0, 200, 300, 500 };

	if (depthleft <= 3
	&&  !incheck
	&&   abs(alpha) < 9000
	&&   staticeval + fmargin[depthleft] <= alpha)
		 f_prune = 1;	
	
	// IID
	
	if (TTmove.from == -1 && depthleft > 2) {
		int newdepth = depthleft - 2;
		int val = alphaBeta(pos, alpha, beta, newdepth, 0, ply + 1, pv, endtime);
		TTmove = *pv;
		//TTdata = getTTentry(&TT, hash);
        //if (TTdata.hash == hash) {
        //    TTmove = TTdata.bestmove;
        //}
	}
	 
	if (TTmove.from == -1) TTmove = *pv;
	/*
	int new_depth = depthleft;
    if (TTmove.from == -1 && depthleft >= 6 && staticeval + 150 >= beta) {
        new_depth = 3 * depthleft / 4 - 2;
		int score = alphaBeta(pos, alpha, beta, new_depth - 1, 0, ply + 1, pv, endtime);
        TTdata = getTTentry(&TT, hash);
        if (TTdata.hash == hash) {
            TTmove = TTdata.bestmove;
        }
    }
*/
	struct move moves[MAX_MOVES];
	const int num_moves = genMoves(pos,moves);
	sortMoves(pos,moves,num_moves,TTmove,ply);
	int score;
	int bestscore = INT_MIN;
	int legalmoves = 0;
	int fullwindow = 1;
	int extended = 0;
	
	for (int i = 0;i < num_moves;i++) {
		char piece = getPiece(pos,moves[i].from);
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
		
		extended = 0;
		if (piece == 'p' && getrank(moves[i].to) == 1) {
			depthleft += 1;
			extended = 1;
		}
		else if (piece == 'P' && getrank(moves[i].to) == 6) {
			extended = 1;
			depthleft += 1;
		}
		
		// PV search - doesn't work
		/*
		if (fullwindow) {
			score = -alphaBeta(pos, -beta, -alpha, depthleft - 1 - r, 0, ply + 1, pv, endtime);
			if (r > 0 && score > alpha) {
				score = -alphaBeta(pos, -beta, -alpha, depthleft - 1, 0, ply + 1, pv, endtime);
			}
		}
		else {
			score = -alphaBeta(pos, -alpha-1, -alpha, depthleft - 1, 0, ply + 1, pv, endtime);
			//if (r > 0 && score > alpha) {
			//	score = -alphaBeta(pos, -alpha-1, -alpha, depthleft - 1, 0, ply + 1, pv, endtime);
			//}
			if (score > alpha) {
				score = -alphaBeta(pos, -beta, -alpha, depthleft - 1 - r, 0, ply + 1, pv, endtime);
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
		if (extended) depthleft--;
		
		unmakeMove(pos);
		
		if (score > alpha) {
			fullwindow = 0;
		}
		if (score > bestscore) {
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
struct pvline getPV(struct position *pos, int depth) {
	struct pvline pvline;
	U64 hash = generateHash(pos);
	struct TTentry TTdata = getTTentry(&TT,hash);
	pvline.moves[0] = TTdata.bestmove;
	pvline.size = 1;
	int movesmade = 0;
	int movesunmade = 0;
	for (int i = 1;i < depth;i++) {
		struct move bestmove = TTdata.bestmove;
		makeMove(&bestmove,pos);
		movesmade++;
		pos->tomove = !pos->tomove;
		if (isCheck(pos)) {
			pos->tomove = !pos->tomove;
			unmakeMove(pos);
			movesunmade++;
			pvline.size -= 1;
			break;
		}
		pos->tomove = !pos->tomove;
		hash = generateHash(pos);
		TTdata = getTTentry(&TT,hash);
		if (TTdata.hash != hash) break;
		pvline.moves[i] = TTdata.bestmove;
		pvline.size++;
	}
	//printf("\n-- %d %d %d\n",movesmade,movesunmade,pvline.size);
	for (int i = 0;i < (movesmade - movesunmade);i++) {
		unmakeMove(pos);
		//movesunmade++;
	}
	//printf("\n%d %d\n",movesmade, movesunmade);
	
	return pvline;
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
	
	
	double time_spent;
	int time_spentms;
	
	// Timing code
	const clock_t begin = clock();
	clock_t endtime = clock() + (movetime / 1000.0 * CLOCKS_PER_SEC);
	clock_t maxendtime = endtime + (movetime * 1 / 1000.0 * CLOCKS_PER_SEC);
	clock_t origendtime = endtime;
	//printf("start %d end %d maxend %d\n",begin,endtime,maxendtime);
	// Movegen
	struct move moves[MAX_MOVES];
	const int num_moves = genMoves(&pos, moves);

	// Only move
	//if (num_moves == 1) return moves[0];
	clearHistory();
	// Calculate hash
	//const U64 hash = generateHash(&pos);
	
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
	int time_spent_prevms;
	int score = 0;
	int lastscore = 0;
	struct move pvlist[128];
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
		//rootdepth = d;
		int origendtime = endtime;
		time_spent = (double)(clock() - begin) / CLOCKS_PER_SEC;
		time_spentms = (int)(time_spent*1000);
		/*
		if (abs(score - lastscore) > 100 && d > 3) {
			int remaining_time = endtime - time_spent;
			int remaining_timems = remaining_time*1000;
			if (time_spent_prevms == 0) time_spent_prevms = 1;
			double factor = time_spentms / time_spent_prevms;
			double expectedtime = time_spentms * factor;
			int expectedendtime = clock() + expectedtime;
			//int newendtime = endtime + expectedtime;
			int newendtime = origendtime + remaining_timems * 4;
			if (newendtime > maxendtime) newendtime = maxendtime;
			endtime = newendtime;
			
			//if (newendtime < maxendtime) endtime = newendtime;
		}
		 */
		//printf("clock %d cur %d max %d\n",clock(),endtime,maxendtime);
		// Check how many times the PV has changed in the last 4 depths
		
		int losingontime = 0;
		int movetimedonepercent = time_spentms * 100 / movetime;
		if (pos.tomove == WHITE) {
			if (wtime < btime) losingontime = 1;
		}
		else {
			if (btime < wtime) losingontime = 1;
		}
		int timeleftpercent;
		if (pos.tomove == WHITE) {
			timeleftpercent = wtime * 100 / origwtime; // time left as percentage of original time
		}
		if (pos.tomove == BLACK) {
			timeleftpercent = btime * 100 / origbtime; // time left as percentage of original time
		}
		if (d > 4 && abs(score - lastscore) >= 0 && !losingontime && timeleftpercent > 30) {
			int timeschanged = 0;
			
			for (int i = d - 1;i >= 0 && i > d - 5;i--) {
				int issame = 0;
				struct move lastpv = pvlist[i-1];
				if (pvlist[i].to == lastpv.to && pvlist[i].from == lastpv.from && pvlist[i].prom == lastpv.prom) {
					issame = 1;
				}
				if (!issame) timeschanged++;
			}
			
			if (timeschanged >= 1) { 
				// PV move has changed at least one time in last 4 iterations
				// Extend search time
				double remaining_time = endtime - time_spent;
				double remaining_timems = remaining_time;
				/*
				if (time_spent_prevms == 0) time_spent_prevms = 1;
				double factor = time_spentms / time_spent_prevms;
				factor = 4.0;
				double expectedtime = time_spentms * factor;
				int expectedendtime = clock() + expectedtime;
				 */
				int newendtime = clock() + + remaining_timems + remaining_timems * 0.015;
				if (newendtime > maxendtime) newendtime = maxendtime;
				//printf("extended time: max: %d, original: %d, new: %d, expected: %.2f\n",maxendtime,origendtime,newendtime,expectedtime);
				//printf("extended time from %d to %d, max %d\n",endtime,newendtime,maxendtime);
				endtime = newendtime;
			}
		}
		
		lastscore = score;
		// Predict whether we have enough time for next search and break if not
		
		if (d > 1 && time_spentms > 30 && endtime == origendtime) {
			if (time_spent_prevms == 0) time_spent_prevms = 1;
			double factor = time_spentms / time_spent_prevms;
			double expectedtime = time_spentms * 4;
			int expectedendtime = clock() + expectedtime;
			if (expectedendtime > endtime) break;
		}
		
		score = alphaBeta(&pos, -MATE_SCORE, MATE_SCORE, d, 0, 0, &pv, endtime);
		
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
		 
		time_spent_prevms = time_spentms;
		
		// Update results
		//bestmove = TTdata.bestmove;
		bestmove = pv;
		pvlist[d] = pv;
		time_spent = (double)(clock() - begin) / CLOCKS_PER_SEC;
		time_spentms = (int)(time_spent*1000);
		int nps = nodesSearched / time_spent;
		// Info string
		printf("info");
		printf(" depth %i", d);
		printf(" nodes %" PRIu64, nodesSearched);
		printf(" time %i", (int)(time_spent*1000));
		printf(" nps %i", nps);
		printf(" score cp %i", score);
		//printf(" pv %s", movetostr(bestmove));
		struct pvline pvline = getPV(&pos,d);
		printf(" pv ");
		for (int i = 0;i < pvline.size; i++) {
			printf("%s ",movetostr(pvline.moves[i]));
		}
		printf("\n");
		if (score == MATE_SCORE || score == -MATE_SCORE) break;
	}
	time_spent = (double)(clock() - begin) / CLOCKS_PER_SEC;
	time_spentms = (int)(time_spent*1000);
	
	printf("info time %d", time_spentms);
	printf("\n");
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