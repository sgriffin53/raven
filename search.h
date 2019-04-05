#ifndef SEARCH_H
#define SEARCH_H

#include "makemove.h"
#include "movegen.h"
#include "functions.h"
#include "eval.h"

int negaMax(struct position *pos,int depth,int timeLeft) {
	assert(depth >= 0);
	nodesSearched++;
	
	if (depth == 0) {
		//return taperedEval(pos);
	}
	
	int kingpos;
	struct move moves[MAX_MOVES];
	int maxScore = -9999;
	int num_moves = genLegalMoves(pos,moves);
	clock_t begin = clock();
	int timeElapsed = 0;
	int numcheckmoves = 0;
	for (int i = 0;(i < num_moves && timeElapsed == 0);i++) {
		
		clock_t end = clock();
		double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
		int time_spentms = (int)(time_spent * 1000);
		if (time_spentms >= timeLeft) {
			timeElapsed = 1;
			break;
		}
		
		makeMove(&moves[i],pos);
		
		pos->tomove = !pos->tomove;
		if (pos->tomove == WHITE) kingpos = pos->Wkingpos;
		else kingpos = pos->Bkingpos;
		int incheck = isCheck(pos,kingpos);
		if (incheck) {
			unmakeMove(pos);
			numcheckmoves++;
			continue;
		}
		pos->tomove = !pos->tomove;
		
		int score = -negaMax(pos,depth - 1, (timeLeft - time_spentms));
		
		unmakeMove(pos);
		
		if (score > maxScore) {
			maxScore = score;
		}
	}
	
	if (num_moves == numcheckmoves) {
		// no legal moves
		if (pos->tomove == WHITE) kingpos = pos->Wkingpos;
		else kingpos = pos->Bkingpos;
		int incheck = isCheck(pos,kingpos);
		if (incheck) {
			// side to move is in checkmate
			return -MATE_SCORE;
		}
		else {
			// stalemate
			return 0;
		}
	}
	
	if (isThreefold(pos)) return 0;
	if (pos->halfmoves >= 100) return 0;
	
	return maxScore;
}
int qSearch(struct position *pos, int alpha, int beta, int timeLeft) {
	assert(pos);
	assert(alpha >= -99999 && beta <= 99999);
	int incheck;
	int score;
	int kingpos;
	struct move moves[MAX_MOVES];
	int standpat = taperedEval(pos);
	if (standpat >= beta) {
		nodesSearched++;
		return beta;
	}
	if (alpha < standpat) alpha = standpat;
	clock_t begin = clock();
	int num_moves = genLegalMoves(pos,moves);
	int timeElapsed = 0;
	
	for (int i = 0;(i < num_moves && timeElapsed == 0);i++) {
		clock_t end = clock();
		double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
		int time_spentms = (int)(time_spent * 1000);
		if (time_spentms >= timeLeft) {
			timeElapsed = 1;
			break;
		}

		if (moves[i].cappiece == '0') continue;
		
		makeMove(&moves[i],pos);

		// check if move is legal (doesn't put in check)
		pos->tomove = !pos->tomove;
		if (pos->tomove == WHITE) kingpos = pos->Wkingpos;
		else kingpos = pos->Bkingpos;
		incheck = isCheck(pos,kingpos);
		if (incheck) {
			unmakeMove(pos);
			continue;
		}
		pos->tomove = !pos->tomove;

		// score node
		score = -qSearch(pos,-beta,-alpha, (time_spentms - timeLeft));
		
		nodesSearched++;
		
		unmakeMove(pos);
		
		if (score >= beta) return beta;
		if (score > alpha) alpha = score;
	}
	return alpha;
}
int alphaBeta(struct position *pos, int alpha, int beta, int depthleft, int nullmove, int timeLeft) {
	assert(pos);
	assert(alpha >= -99999 && beta <= 99999);
	if (timeLeft <= 0) {
		return alpha;
	}
	
	nodesSearched++;
	
	if (isThreefold(pos)) return 0;
	if (pos->halfmoves >= 100) return 0;
	
	// check extensions
	int kingpos;
	if (pos->tomove == WHITE) kingpos = pos->Wkingpos;
	else kingpos = pos->Bkingpos;
	int incheck = isCheck(pos,kingpos);
	if (incheck) depthleft++;
	
	if (depthleft <= 0) {
		return qSearch(pos,alpha,beta,timeLeft);
		//return evalBoard(pos);
	}
/*
	// null move pruning - doesn't work
	
	if ((!incheck) && (!nullmove) && (depthleft > 2) && (!isEndgame(pos))) {
		int oldhalfmoves = pos->halfmoves;
		pos->halfmoves = 0;
		pos->tomove = !pos->tomove; // Making a null-move
		posstackend++;
		posstack[posstackend] = *pos;
		int score = -alphaBeta(pos, -beta, -beta+1, depthleft - 1 - 2,1,timeLeft);
		pos->tomove = !pos->tomove; // Unmaking the null-move
		posstackend--;
		pos->halfmoves = oldhalfmoves;

		if(score >= beta) return score; // Cutoff
	}
*/	
	struct move moves[MAX_MOVES];
	int num_moves = genLegalMoves(pos,moves);
	sortMoves(pos,moves,num_moves);
	clock_t begin = clock();
	int timeElapsed = 0;
	int numcheckmoves = 0;
	for (int i = 0;(i < num_moves && timeElapsed == 0);i++) {
		clock_t end = clock();
		double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
		int time_spentms = (int)(time_spent * 1000);
		makeMove(&moves[i],pos);
		pos->tomove = !pos->tomove;
		if (pos->tomove == WHITE) kingpos = pos->Wkingpos;
		else kingpos = pos->Bkingpos;
		int incheck = isCheck(pos,kingpos);
		if (incheck) {
			unmakeMove(pos);
			numcheckmoves++;
			continue;
		}
		pos->tomove = !pos->tomove;
		
		//late move reduction
		int score;
		int movenum = (i - numcheckmoves);
		if (pos->tomove == WHITE) kingpos = pos->Wkingpos;
		else kingpos = pos->Bkingpos;
		incheck = isCheck(pos,kingpos);
		//if ((movenum > 3) && (depthleft >= 4) && (moves[i].cappiece == '0') && (moves[i].prom == 0) && (!incheck)) { // +25 elo over version without
		if (moves[i].cappiece == '0') { // + 95 elo over version without LMR
			// try to reduce non-capture moves
			score = -alphaBeta(pos,-beta,-alpha, depthleft - 1 - 1,0,(timeLeft - time_spentms));
			if (score > alpha) {
				//re search
				score = -alphaBeta(pos,-beta,-alpha, depthleft - 1,0,(timeLeft - time_spentms));
			}
		}
		else {
			score = -alphaBeta(pos,-beta,-alpha, depthleft - 1,0,(timeLeft - time_spentms));
		}
		unmakeMove(pos);
		
		if (score >= beta) {
			return beta;
		}
		if (score > alpha) {
			alpha = score;
		}
	}
	
	if (num_moves == numcheckmoves) {
		// no legal moves
		if (pos->tomove == WHITE) kingpos = pos->Wkingpos;
		else kingpos = pos->Bkingpos;
		int incheck = isCheck(pos,kingpos);
		if (incheck) {
			// side to move is in checkmate
			return -MATE_SCORE;
		}
		else {
			// stalemate
			return 0;
		}
	}
	
	return alpha;
}
struct move search(struct position pos, int searchdepth,int movetime) {
	assert(&pos);
	assert(searchdepth>=0);
	assert(movetime>0);
	nodesSearched = 0;
	
	struct move moves[MAX_MOVES];
	int kingpos;
	clock_t begin = clock();
	int timeElapsed = 0;
	double time_spent;
	struct move bestmove;
	int nps;
	int num_moves = genLegalMoves(&pos,moves);
	struct move lastbestmove = moves[0];
	for (int curdepth = 1; (curdepth < searchdepth+1 && timeElapsed == 0);curdepth++) {
		int bestScore = -MATE_SCORE;
		for (int i = 0;i < num_moves;i++) {
			clock_t end = clock();
			time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
			int time_spentms = (int)(time_spent * 1000);
			if (time_spentms >= movetime) {
				//bestmove = lastbestmove;
				timeElapsed = 1;
				break;
			}
			assert((moves[i].to >= 0 && moves[i].to <= 63));
			makeMove(&moves[i],&pos);
			pos.tomove = !pos.tomove;
			if (pos.tomove == WHITE) kingpos = pos.Wkingpos;
			else kingpos = pos.Bkingpos;
			int incheck = isCheck(&pos,kingpos);
			if (incheck) {
				unmakeMove(&pos);
				continue;
			}
			pos.tomove = !pos.tomove;
			
			int curscore;
			//int curscore = -negaMax(&pos,curdepth-1,(movetime - time_spentms));
			if (isThreefold(&pos)) {
				curscore = 0;
			}
			else if (pos.halfmoves >= 100) {
				curscore = 0;
			}
			else {
				curscore = -alphaBeta(&pos,-99999,99999,curdepth-1,0,(movetime - time_spentms));
			}

			if (curscore == MATE_SCORE) {
				printf("info depth %d nodes %d time %d nps %d score mate %d pv %s\n",(curdepth),nodesSearched,((int)(time_spent*1000)),nps,curdepth,movetostr(moves[i]));
				fflush(stdout);
				unmakeMove(&pos);
				return moves[i];
			}
			
			if (curscore > bestScore) {
				bestScore = curscore;
				bestmove = moves[i];
			}
			
			unmakeMove(&pos);
			
			nps = nodesSearched / time_spent;
		}
		
		lastbestmove = bestmove;
		/*
		printf("moves before: ");
		for (int i =0;i<num_moves;i++) {
			printf("%s ",movetostr(moves[i]));
		}
		printf("\n");
		*/
		// moves bestmove to start of moves array
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
		/*
		printf("moves after: ");
		for (int i =0;i<num_moves;i++) {
			printf("%s ",movetostr(moves[i]));
		}
		printf("\n");
		*/
		printf("info depth %d nodes %d time %d nps %d score cp %d pv %s\n",(curdepth),nodesSearched,((int)(time_spent*1000)),nps,bestScore,movetostr(bestmove));
		fflush(stdout);
	}
	return bestmove;
}
#endif