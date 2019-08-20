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
#include "move.h"
#include "bitboards.h"
#include "magicmoves.h"

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
			struct move blankmove = {.to=-1, .from=-1, .prom=-1};
			countermoves[i][j] = blankmove;
		}
	}
}
int qSearch(struct position *pos, int alpha, int beta, int ply, clock_t endtime) {
	assert(pos);
	assert(alpha >= -MATE_SCORE && beta <= MATE_SCORE);
	if (ply > seldepth) seldepth = ply;
	//int incheck;
	//int score;
	//int kingpos;
	if (clock() >= endtime) {
		return -MATE_SCORE;
	}
	//int origAlpha = alpha;
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
	const int num_moves = genMoves(pos,moves, 1);
	//int origischeck = isCheck(pos);
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
		//char cappiece = getPiece(pos,moves[i].to);
		//char cappiece = moves[i].cappiece;

		//futility pruning in qsearch
		/*
		if (!isEndgame(pos) && pieceval(cappiece) + 120 + standpat <= alpha) {
			if (!isCheck(pos)) continue;
		}
		*/
		
		int SEEvalue = SEEcapture(pos, moves[i].from, moves[i].to, pos->tomove);
		if (SEEvalue < 0) continue;
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
		//U64 newhash;
		//if (score >= beta) newhash = generateHash(pos);
		
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

struct singulardata {
	int extend;
	int score;
};

struct singulardata move_is_singular(struct position *pos, struct move *moves, int num_moves, int depth, int beta, int ply, clock_t endtime) {
	int rBeta = beta - 4 * depth;
	int rAlpha = rBeta - 1;
	int rDepth = (depth - 3) / 2;
	
	for (int i = 0;i < num_moves;i++) {
		const struct move currentmove = moves[i];
		makeMove(&currentmove, pos);
		pos->tomove = !pos->tomove;
		if (isCheck(pos)) {
			unmakeMove(pos);
			continue;
		}
		pos->tomove = !pos->tomove;
		struct move pv;
		int score = -alphaBeta(pos, -rBeta, -rAlpha, rDepth, 0, ply + 1, &pv, endtime);
		unmakeMove(pos);
		if (score >= rBeta) {
			struct singulardata sd = {.extend=0, .score=score};
			return sd;
		}
	}
	struct singulardata sd = {.extend=1, .score=rAlpha};
	return sd;
}
int alphaBeta(struct position *pos, int alpha, int beta, int depthleft, int nullmove, int ply, struct move *pv, clock_t endtime) {
	assert(pos);
	assert(alpha >= -MATE_SCORE && beta <= MATE_SCORE);
	assert(beta > alpha);
	//assert(depthleft >= 0);
	if (ply > seldepth) seldepth = ply;
	int origdepthleft = depthleft;
	if (depthleft <= 0) depthleft = 0;
	if (clock() >= endtime) {
		return -MATE_SCORE;
	}
	currenthash = 0;
	if (isThreefold(pos)) return 0;
	if (pos->halfmoves >= 100) return 0;
	int incheck = isCheck(pos);
	if (incheck) depthleft++;
	/*
	if (!incheck && depthleft <= 0) {
		int lazyeval = evalBoard(pos);
		int lazyalpha = 1000;
		int lazybeta = 400;
		if (lazyeval - lazybeta >= beta) return beta;
		if (lazyeval + lazyalpha < alpha) return alpha;
	}
	 */
	if (depthleft <= 0) {
		return qSearch(pos, alpha, beta, ply + 1, endtime);

	}
	nodesSearched += 1;
	struct move bestmove = {.to=-1,.from=-1,.prom=-1,.cappiece=-1};;
	struct move TTmove = {.to=-1,.from=-1,.prom=-1,.cappiece=-1};
	//TTmove = *pv;
	int origAlpha = alpha;
	int origBeta = beta;
	
	
	
	
	U64 hash;
	if (currenthash != 0) {
		hash = currenthash;
	}
	else if (hashstack[hashstackend - 1] != 0) {
		hash = hashstack[hashstackend - 1];
	}
	else hash = generateHash(pos);
	struct TTentry TTdata = getTTentry(&TT,hash);
	if (TTdata.hash == hash) {
		if (TTdata.depth == origdepthleft) {
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
	
	int staticeval = taperedEval(pos);
	
	// static null pruning (reverse futility pruning)
	
	if (beta <= MATE_SCORE) {
		if (depthleft == 1 && staticeval - 300 > beta) return beta;
		if (depthleft == 2 && staticeval - 525 > beta) return beta;
		if (depthleft == 3 && staticeval - 900 > beta) depthleft--;
	}
	
	// null move pruning
	
	// if (!nullmove && !isEndgame(pos) && !incheck && !(alpha != beta - 1)) {
	
	if (!nullmove && !incheck && ply != 0 && depthleft >= 3 && !isEndgame(pos)) {
		const int orighalfmoves = pos->halfmoves;
		const int origepsquare = pos->epsquare;
		pos->tomove = !pos->tomove;
		pos->halfmoves = 0;
		pos->epsquare = -1;
		posstack[posstackend] = *pos;
		posstackend++;
		int R = 3;
		int verR = 3;
		/*
		if (ply % 2 == 1) { 
			R = 5;
			verR = 4;
		}
		 */
		const int val = -alphaBeta(pos,-beta,-beta+1, depthleft - 1 - R, 1, ply + 1, pv, endtime);
		pos->tomove = !pos->tomove;
		pos->halfmoves = orighalfmoves;
		pos->epsquare = origepsquare;
		posstackend--;
		if (val >= origBeta || val >= beta) {
			const int verification = alphaBeta(pos,beta - 1,beta, depthleft - 1 - verR, 1, ply + 1, pv, endtime); // alpha_beta(p, md, beta - 1, beta, d, false, false);
			
			if (verification >= beta) return beta;
		}
	}
	
	// null move reductions
	/*
	if (!nullmove && depthleft >= 3 && !incheck && !isendgame) {
		int R = depthleft > 6 ? 4 : 3;
		
		// make null move
		
		const int orighalfmoves = pos->halfmoves;
		const int origepsquare = pos->epsquare;
		pos->tomove = !pos->tomove;
		pos->halfmoves = 0;
		pos->epsquare = -1;
		posstack[posstackend] = *pos;
		posstackend++;
		
		int score = -alphaBeta(pos, 0 - beta, 1 - beta, depthleft - R - 1, 1, ply + 1, pv, endtime);
		
		// unmake null move
		
		pos->tomove = !pos->tomove;
		pos->halfmoves = orighalfmoves;
		pos->epsquare = origepsquare;
		posstackend--;
		if (score >= beta) {
			//return score;
			depthleft -= 4;
			if (depthleft <= 0) {
				return taperedEval(pos);
			}
		}
	}
	*/
	// razoring
	
	/*
	const int razoring_margin[4] = {0, 333, 353, 324};
	if (!incheck && depthleft < 4 && staticeval <= alpha - razoring_margin[depthleft] && ply != 0) {
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
	if (!incheck && depthleft <= 2 && staticeval <= alpha - 300) {
		if (depthleft == 1) return qSearch(pos, alpha, beta, ply + 1, endtime);
		int rWindow = alpha - 300;
		int value = qSearch(pos,rWindow,rWindow+1,ply + 1, endtime);
		if (value <= rWindow) return value;
	}
	*/
	
	/*
	if (!incheck && depthleft <= 2 && ply != 0) {
		const int ralpha = alpha - 250 - depthleft * 50;
		if (staticeval < ralpha) {
			if (depthleft == 1 && ralpha < alpha) {
				return qSearch(pos, alpha, beta, ply + 1, endtime);
			}
			int value = qSearch(pos, ralpha, ralpha + 1, ply + 1, endtime);
			if (value <= ralpha) return value;
		}
	}
	*/
	int f_prune = 0;
	
	int fmargin[4] = { 0, 200, 300, 500 };
	if (depthleft <= 3
	&&  !incheck
	&&   abs(alpha) < 9000
	&&   staticeval + fmargin[depthleft] <= alpha)
		 f_prune = 1;	
	
	// IID
	
	if (TTmove.from == -1 && depthleft >= 7) {
		int newdepth = 3;
		if (newdepth < 1) newdepth = 1;
		int val = alphaBeta(pos, alpha, beta, newdepth, 0, ply + 1, pv, endtime);
		//int val = qSearch(pos, alpha, beta, ply + 1, endtime);
		//TTmove = *pv;
		TTdata = getTTentry(&TT, hash);
        if (TTdata.hash == hash) {
            TTmove = TTdata.bestmove;
        }
	}
	/*
	//if (TTmove.from == -1 && depthleft >= 6 && staticeval + 150 >= beta) {
	if (TTmove.from == -1 && depthleft >= 6 && staticeval > alpha - 200) {
		//int new_depth = 3 * (depthleft / 4) - 1;
		int new_depth = depthleft - 3;
		if (new_depth < 1) new_depth = 1;
		assert(new_depth > 0);
		//if (new_depth < 1) new_depth = 1;
		const int score = alphaBeta(pos, alpha, beta, new_depth, 0, ply + 1, pv, endtime);
		//const int score = -alphaBeta(pos, -beta, -alpha, new_depth, 0, ply + 1, pv, endtime);
		//TTmove = *pv;
		TTdata = getTTentry(&TT, hash);
		assert(TTdata.hash == hash);
		TTmove = TTdata.bestmove;
		
	}
	*/
	if (TTmove.from == -1) TTmove = *pv;
	struct move moves[MAX_MOVES];
	const int num_moves = genMoves(pos,moves, 0);
	sortMoves(pos,moves,num_moves,TTmove,ply);
	
	// single reply extensions
	
	/*
	if (num_moves == 1) {
		depthleft++;
	}
	*/
	
	// Prob Cut
	
	if (depthleft >= 5 && abs(beta) <= MATE_SCORE && staticeval + 100 >= beta + 100) {
		int rbeta = min(MATE_SCORE, beta + 100);
		int probcutcount = 0;
		for (int i = 0;i < num_moves;i++) {
			makeMove(&moves[i],pos);
			pos->tomove = !pos->tomove;
			if (isCheck(pos)) {
				unmakeMove(pos);
				continue;
			}
			pos->tomove = !pos->tomove;
			probcutcount++;
			int probcutscore;
			//probcutscore = -qSearch(pos, -rbeta, -rbeta + 1, ply + 1, endtime);
			//if (probcutscore >= rbeta) probcutscore = -alphaBeta(pos, -rbeta, -rbeta + 1, depthleft - 4, 0, ply + 1, pv, endtime);
			probcutscore = -alphaBeta(pos, -rbeta, -rbeta + 1, depthleft - 4, 0, ply + 1, pv, endtime);
			unmakeMove(pos);
			if (probcutscore >= rbeta) {
				return probcutscore;
			}
		}
	}
	
	int allorigdepthleft = depthleft;
	
	int score = 0;
	int bestscore = INT_MIN;
	int legalmoves = 0;
	int fullwindow = 1;
	
	for (int i = 0;i < num_moves;i++) {
		char piece = moves[i].piece;
		char cappiece = moves[i].cappiece;
		
		int histval = history[pos->tomove][moves[i].from][moves[i].to];
		int butterflyval = butterfly[pos->tomove][moves[i].from][moves[i].to];
		//int histscore = histval;
		//if (butterflyval != 0) histscore = histval / butterflyval;
		
		int isTTmove = 0;
		if (TTmove.from == moves[i].from && TTmove.to == moves[i].to && TTmove.prom == moves[i].prom) isTTmove = 1;
		
		int isKiller = 0;
		if (killers[ply][0].from == moves[i].from && killers[ply][0].to == moves[i].to && killers[ply][0].prom == moves[i].prom) isKiller = 1;
		if (killers[ply][1].from == moves[i].from && killers[ply][1].to == moves[i].to && killers[ply][1].prom == moves[i].prom) isKiller = 1;
		
		int histmargins[13] = { 120, 120, 120, 120, 150, 180, 180, 350, 550, 1000, 1500, 2000, 3000 };
		//int histmargins[13] = { 120, 80, 100, 120, 120, 140, 140, 250, 750, 1100, 1500, 2000 };
		int histmargin;
		if (rootdepth <= 12) histmargin = histmargins[rootdepth];
		else histmargin = 3000;
		//if (ply % 2 == 1) {
		//	if (rootdepth <= 12) histmargin = histmargins_aggressive[rootdepth];
		//	else histmargin = 2000;
		//}
		
		
		double cutoffpercent = ((double)histval * 100.0 / (double)(histval + butterflyval));
		
		if (!isTTmove && moves[i].cappiece == '0' && !isKiller
			&& bestmove.from != -1 && legalmoves >= 1 && (histval + butterflyval) > histmargin && cutoffpercent < 1.25 && ply != 0) {
			continue;
		}
		int extension = 0;
		
		
		// Singular extensions and multi-cut
		/*
		int isTTmove = 0;
		if ((moves[i].from == TTmove.from) && (moves[i].to == TTmove.to) && (moves[i].prom == TTmove.prom)) {
			isTTmove = 1;
		}
		*/
		/*
		//if (depthleft >= 5 && ply != 0 && isTTmove && abs(TTdata.score) < MATE_SCORE && TTdata.flag == LOWERBOUND) {
		if (legalmoves == 1 && num_moves > 1 && depthleft >= 5 && isTTmove && abs(TTdata.score) < MATE_SCORE && TTdata.depth >= depthleft - 3 && TTdata.flag != UPPERBOUND) {
		 
			
			int singularBeta = TTdata.score - 2 * depthleft;
			int halfdepth = depthleft / 2;
			int value = alphaBeta(pos, singularBeta - 1, singularBeta, halfdepth - 2, 0, ply + 1, pv, endtime);
			if (value < singularBeta) {
				depthleft++;
			}
			else if (staticeval >= beta && singularBeta >= beta) {
				//*pv = TTmove;
				//return singularBeta;
			}
			 
			 
			struct move newmoves[MAX_MOVES];
			const int num_moves = genMoves(pos,newmoves, 0);
			struct move newTTmove = {.from=-1,.to=-1, .prom='0', .piece='0', .cappiece='0'};
			U64 newhash = generateHash(pos);
			struct TTentry TTdata = getTTentry(&TT,hash);
			if (TTdata.hash == hash) newTTmove = TTdata.bestmove;
			sortMoves(pos,newmoves,num_moves,newTTmove,ply);
			struct singulardata sd;
			sd = move_is_singular(pos, newmoves, num_moves, depthleft, beta, ply, endtime);
			if (sd.extend == 1) {
				extension = 1;
				printf("extended\n");
			}
			else if (sd.score >= beta) {
				return sd.score;
				printf("pruned\n");
			}
		
			
		}
		*/
		
		makeMove(&moves[i],pos);
		pos->tomove = !pos->tomove;
		if (isCheck(pos)) {
			unmakeMove(pos);
			continue;
		}
		pos->tomove = !pos->tomove;
	
		
		int givescheck = isCheck(pos);
		legalmoves++;
		
		//if (histscore > 0) {
		//	printf("hist val: %d\n",histscore);
		//}
		if (f_prune
		&& legalmoves > 1
		&&  cappiece == '0'
		&&  moves[i].prom == 0
		//&&  histscore <= fhistorythreshold[depthleft]
		&& !givescheck
		&& ply != 0) {
		//&&  !isAttacked(pos,kingpos,pos->tomove)) {
			unmakeMove(pos);
			continue;
		}
		
		int r = reduction(&moves[i], depthleft, cappiece, legalmoves, incheck, givescheck, ply);
		
		if (piece == 'p' && getrank(moves[i].to) == 1) {
			extension = 1;
		}
		else if (piece == 'P' && getrank(moves[i].to) == 6) {
			extension = 1;
		}
		
		// PV search - doesn't work
		
		//r = r - extension;
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
				//if (r > 0 && score > alpha) {
				//	score = -alphaBeta(pos, -beta, -alpha, depthleft - 1, 0, ply + 1, pv, endtime);
				//}
			}
		}
		*/
		 
		// Search

		score = -alphaBeta(pos, -beta, -alpha, depthleft - 1 - r + extension, 0, ply + 1, pv, endtime);
		
		// Redo search
		
		if (r > 0 && score > alpha) {
			score = -alphaBeta(pos, -beta, -alpha, depthleft - 1 + extension, 0, ply + 1, pv, endtime);
		}
		if (ply == 0) {
			//printf("ab: depth: %d, move: %s, score: %d\n", depthleft, movetostr(moves[i]), score);
		}
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
				struct move prevmove = movestack[movestackend - 1];
				countermoves[prevmove.from][prevmove.to] = moves[i];
			}
			break;
		}
		else {
			// no beta cut off
			if (cappiece == '0') {
				butterfly[pos->tomove][moves[i].from][moves[i].to] += pow(2,depthleft);
			}
		}
		depthleft = allorigdepthleft;
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
	
	addTTentry(&TT, hash, origdepthleft, newflag, bestmove, bestscore);
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

	//for (int i = 0;i < 1024;i++) {
	//	hashstack[i] = 0;
	//}
	
	// Result
	struct move bestmove;
	
	seldepth = 0;
	
	double time_spent;
	int time_spentms;
	
	// Timing code
	const clock_t begin = clock();
	clock_t endtime = clock() + (movetime / 1000.0 * CLOCKS_PER_SEC);
	clock_t maxendtime = endtime + (movetime * 1 / 1000.0 * CLOCKS_PER_SEC);
	clock_t origendtime = endtime;
	
	assert(maxendtime > endtime);
	//printf("start %d end %d maxend %d\n",begin,endtime,maxendtime);
	// Movegen
	struct move moves[MAX_MOVES];
	const int num_moves = genMoves(&pos, moves, 0);

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
	int lastlastscore = 0;
	struct move pvlist[128];
	for(int d = 1; d <= searchdepth; ++d) {
		
		time_spent = (double)(clock() - begin) / CLOCKS_PER_SEC;
		time_spentms = (int)(time_spent*1000);
		rootdepth = d;
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
		//int movetimedonepercent = time_spentms * 100 / movetime;
		if (pos.tomove == WHITE) {
			if (wtime < btime) losingontime = 1;
		}
		else {
			if (btime < wtime) losingontime = 1;
		}
		double timeleftpercent;
		if (pos.tomove == WHITE) {
			timeleftpercent = (double)wtime * 100.0 / (double)origwtime; // time left as percentage of original time
		}
		if (pos.tomove == BLACK) {
			timeleftpercent = (double)btime * 100.0 / (double)origbtime; // time left as percentage of original time
		}
		if (d > 4 && abs(score - lastscore) >= 0 && !losingontime && timeleftpercent > 30.0) {
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
				clock_t newendtime = clock() + + remaining_timems + remaining_timems * 0.015;
				if (newendtime > maxendtime) newendtime = maxendtime;
				//printf("extended time: max: %d, original: %d, new: %d, expected: %.2f\n",maxendtime,origendtime,newendtime,expectedtime);
				//printf("extended time from %d to %d, max %d\n",endtime,newendtime,maxendtime);
				endtime = newendtime;
			}
		}
		
		// Predict whether we have enough time for next search and break if not
		
		if (d > 1 && time_spentms > 30 && endtime == origendtime) {
			if (time_spent_prevms == 0) time_spent_prevms = 1;
			//double factor = time_spentms / time_spent_prevms;
			double expectedtime = time_spentms * 4;
			int expectedendtime = clock() + expectedtime;
			if (expectedendtime > endtime) break;
		}
		
		// Aspiration windows
		
		/*
		if (d <= 3) {
			score = alphaBeta(&pos, -MATE_SCORE, MATE_SCORE, d, 0, 0, &pv, endtime);
		}
		else {
			int b[6] = {10, 100, 1000, MATE_SCORE};
			//lastscore = 0;
			for (int i = 0;i < 4;i++) {
				if (i == 3) lastlastscore = 0;
				int min = lastlastscore - b[i];
				int max = lastlastscore + b[i];
				score = alphaBeta(&pos, min, max, d, 0, 0, &pv, endtime);
				if (-b[i] < score && score < b[i]) break;
				//if (score > -b[i] && score < b[i]) break;
			}
		}
		
		lastlastscore = lastscore;
		lastscore = score;
		*/
		 /*
		int windowSize = 16;
		int alpha, beta, delta = windowSize;
		alpha = d >= 3 ? max(-MATE_SCORE, lastscore - delta) : -MATE_SCORE;
		beta = d >= 3 ? min(MATE_SCORE, lastscore + delta) : MATE_SCORE;
		int searchesdone = 0;
		while (searchesdone < 6) {
			score = alphaBeta(&pos, alpha, beta, d, 0, 0, &pv, endtime);
			if (score > alpha && score < beta) {
				//printf("aspiration completed after %d searches (alpha = %d, beta = %d).\n",searchesdone+1,alpha,beta);
				break;
			}

			// Search failed low
			if (score <= alpha) {
				beta = (alpha + beta) / 2;
				alpha = max(-MATE_SCORE, alpha - delta);
			}
			
			// Search failed high
			if (score >= beta) {
				beta = min(MATE_SCORE, beta + delta);
			}
			
			// Expand the search window
			delta = delta + delta / 2;
			searchesdone += 1;
			if (searchesdone >= 5) {
				alpha = -MATE_SCORE;
				beta = MATE_SCORE;
			}
			//printf("delta %d\n",delta);
		}
		*/
		//lastlastscore = lastscore;
		//lastscore = score;
		
		

		score = alphaBeta(&pos, -MATE_SCORE, MATE_SCORE, d, 0, 0, &pv, endtime);
		
		//Ignore the result if we ran out of time
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
		printf(" seldepth %i", seldepth);
		printf(" nodes %" PRIu64, nodesSearched);
		printf(" time %i", (int)(time_spent*1000));
		printf(" nps %i", nps);
		printf(" score cp %i", score);
		//printf(" pv %s", movetostr(bestmove));
		struct pvline pvline = getPV(&pos,d);
		printf(" pv");
		for (int i = 0;i < pvline.size; i++) {
			printf(" %s",movetostr(pvline.moves[i]));
		}
		printf("\n");
		lastsearchdepth = d;
		if (score == MATE_SCORE || score == -MATE_SCORE) break;
	}
	time_spent = (double)(clock() - begin) / CLOCKS_PER_SEC;
	time_spentms = (int)(time_spent*1000);
	
	printf("info time %d", time_spentms);
	printf("\n");
	printf("bestmove %s\n", movetostr(bestmove));

	return bestmove;
}
int SEEcapture(struct position *pos, int from, int to, int side) {
	int value = 0;
	char piece = getPiece(pos,from);
	char cappiece = getPiece(pos,to);
	struct move capmove = {.from=from, .to=to, .prom=0, .piece=piece, .cappiece=cappiece};
	makeMove(&capmove, pos);
	value = pieceval(cappiece) - SEE(pos, to, !side);
	unmakeMove(pos);
	return value;
}
int SEE(struct position *pos, int square, int side) {
	int value = 0;
	struct move capmove = get_smallest_attacker(pos, square, side);
	int cappiece = getPiece(pos, square);
	//printf("%c\n",capmove.piece);
	//dspBoard(pos);
	if (capmove.piece != -1) {
		//dspBoard(pos);
		makeMove(&capmove, pos);
		//printf("made move %s\n", movetostr(capmove));
		//dspBoard(pos);
		value = max(0, pieceval(cappiece) - SEE(pos, square, !side));
		unmakeMove(pos);
	}
	return value;
}
struct move get_smallest_attacker(struct position *pos, int square, int side) {
	U64 BBmypieces;
	U64 BBopppieces;
	U64 BBsquare = 1ULL << square;
	U64 BBoccupied = pos->BBwhitepieces | pos->BBblackpieces;
	if (side == WHITE) BBmypieces = pos->BBwhitepieces;
	else BBmypieces = pos->BBblackpieces;
	if (side == BLACK) BBopppieces = pos->BBwhitepieces;
	else BBopppieces = pos->BBblackpieces;
	struct move blankmove = {.to=-1, .from=-1, .prom=-1, .piece=-1, .cappiece=-1};
	struct move returnmove = {.to=square, .from=-1, .prom=0, .piece=-1, .cappiece=getPiece(pos,square)};
	// pawns
	if (side == WHITE) {
		U64 BBattacks = soWeOne(BBsquare) | soEaOne(BBsquare);
		if (BBattacks & pos->BBpawns & BBmypieces) {
			returnmove.from = __builtin_ctzll(BBattacks & pos->BBpawns & BBmypieces);
			returnmove.piece = 'P';
			return returnmove;
		}
	}
	else { // black
		U64 BBattacks = noWeOne(BBsquare) | noEaOne(BBsquare);
		if (BBattacks & pos->BBpawns & BBmypieces) {
			returnmove.from = __builtin_ctzll(BBattacks & pos->BBpawns & BBmypieces);
			returnmove.piece = 'p';
			return returnmove;
		}
	}
	// Knights
	U64 BBattacks = BBknightattacks(BBsquare);
	if (BBattacks & BBmypieces & pos->BBknights) {
		if (side == WHITE) {
			returnmove.from = __builtin_ctzll(BBattacks & pos->BBknights & BBmypieces);
			returnmove.piece = 'N';
			return returnmove;
		}
		else {
			returnmove.from = __builtin_ctzll(BBattacks & pos->BBknights & BBmypieces);
			returnmove.piece = 'n';
			return returnmove;
		}
	}
	// Bishops
	BBattacks = Bmagic(square, BBoccupied);
	if (BBattacks & BBmypieces & pos->BBbishops) {
		if (side == WHITE) {
			returnmove.from = __builtin_ctzll(BBattacks & pos->BBbishops & BBmypieces);
			returnmove.piece = 'B';
			return returnmove;
		}
		else {
			returnmove.from = __builtin_ctzll(BBattacks & pos->BBbishops & BBmypieces);
			returnmove.piece = 'b';
			return returnmove;
		}
	}
	// Rooks
	BBattacks = Rmagic(square, BBoccupied);
	if (BBattacks & BBmypieces & pos->BBrooks) {
		if (side == WHITE) {
			returnmove.from = __builtin_ctzll(BBattacks & pos->BBrooks & BBmypieces);
			returnmove.piece = 'R';
			return returnmove;
		}
		else {
			returnmove.from = __builtin_ctzll(BBattacks & pos->BBrooks & BBmypieces);
			returnmove.piece = 'r';
			return returnmove;
		}
	}
	// Queens
	BBattacks = Rmagic(square, BBoccupied) | Bmagic(square, BBoccupied);
	if (BBattacks & BBmypieces & pos->BBqueens) {
		if (side == WHITE) {
			returnmove.from = __builtin_ctzll(BBattacks & pos->BBqueens & BBmypieces);
			returnmove.piece = 'Q';
			return returnmove;
		}
		else {
			returnmove.from = __builtin_ctzll(BBattacks & pos->BBqueens & BBmypieces);
			returnmove.piece = 'q';
			return returnmove;
		}
	}
	BBattacks = BBkingattacks(BBsquare);
	if (BBattacks & BBmypieces & pos->BBkings) {
		if (side == WHITE) {
			returnmove.from = __builtin_ctzll(BBattacks & pos->BBkings & BBmypieces);
			returnmove.piece = 'K';
			return returnmove;
		}
		else {
			returnmove.from = __builtin_ctzll(BBattacks & pos->BBkings & BBmypieces);
			returnmove.piece = 'k';
			return returnmove;
		}
	}
	return blankmove;
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