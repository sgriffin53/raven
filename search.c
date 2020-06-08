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

#define ONE_PLY 4

int mate_in(int ply) {
	return MATE_SCORE - ply;
}
int mated_in(int ply) {
	return -MATE_SCORE + ply;
}
int reduction(const struct move *move, const int depthleft, char cappiece, int legalmoves, int incheck, int givescheck, int ply) {
	assert(move);
	assert(depthleft >= 0);
	if ((!incheck) && (legalmoves > 4) && (depthleft >= 3 * ONE_PLY) && (move->prom == NONE) && (!givescheck)) {
		if (cappiece == NONE) {
			int red = ONE_PLY;
			if (depthleft >= 6 * ONE_PLY) red = 2 * ONE_PLY;
			if (legalmoves >= 20) red += ONE_PLY;
			return red;
		}
	}

	return 0;
}

void clearKillers(int ply) {
	struct move nomove = {.to=-1,.from=-1,.prom=-1,.cappiece=-1};
	for (int i = 0;i < ply;i++) {
		killers[i][0] = nomove;
		killers[i][1] = nomove;
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
	if (clock() >= endtime) {
		return -MATE_SCORE;
	}
	struct move TTmove = {.to=-1,.from=-1,.prom=NONE,.cappiece=NONE};
	U64 hash;
	if (currenthash == 0) {
		hash = generateHash(pos);
	}
	else hash = currenthash;
	struct TTentry TTdata = getTTentry(&TT,hash);
	if (TTdata.hash == hash) {
		int isvalid = 1;
		if (getColour(pos, TTdata.bestmove.from) != pos->tomove) isvalid = 0;
		if (getColour(pos, TTdata.bestmove.to) == pos->tomove) isvalid = 0;
		if (isvalid) {
			int flag = TTdata.flag;
			int score = TTdata.score;
			if (flag == EXACT
				|| (flag == LOWERBOUND && score >= beta)
				|| (flag == UPPERBOUND && score <= alpha)) {
					return score;
			}
			TTmove = TTdata.bestmove;
		}
	}
	const int standpat = taperedEval(pos);
	if (standpat >= beta) {
		return beta;
	}
	
	// delta pruning
	const int BIG_DELTA = 975;
	if (standpat < alpha - BIG_DELTA) {
		return alpha;
	}
	if (alpha < standpat) alpha = standpat;

	struct move moves[MAX_MOVES];
	const int num_moves = genMoves(pos,moves, 1);
	sortMoves(pos,moves,num_moves,TTmove, ply);
	
	struct move bestmove = {.to=-1,.from=-1,.prom=NONE,.cappiece=NONE};;
	for (int i = 0;(i < num_moves);i++) {
		if (moves[i].cappiece != NONE) {
			int SEEvalue = SEEcapture(pos, moves[i].from, moves[i].to, pos->tomove);
			if (SEEvalue < 0) continue;
		}
		makeMove(&moves[i],pos);

		// check if move is legal (doesn't put in check)
		pos->tomove = !pos->tomove;
		const int incheck = isCheck(pos);
		if (incheck) {
			unmakeMove(pos);
			continue;
		}
		pos->tomove = !pos->tomove;
		nodesSearched++;
		
		currenthash = 0;
		const int score = -qSearch(pos,-beta,-alpha, ply + 1, endtime);
		
		unmakeMove(pos);

		if (score >= beta) {
			if (TTdata.hash != hash) {
				if (beta != 0 && beta <= MATE_SCORE - 100 && beta >= -MATE_SCORE + 100) addTTentry(&TT, hash, 0, LOWERBOUND, moves[i], beta);
			}
			return beta;
		}
		if (score > alpha) {
			alpha = score;
			bestmove = moves[i];
		}
	}
	if (bestmove.from != -1 && TTdata.hash != hash) {
		if (alpha != 0 && alpha <= MATE_SCORE - 100 && alpha >= -MATE_SCORE + 100) addTTentry(&TT, hash, 0, EXACT, bestmove, alpha);
	}
	return alpha;
}

int alphaBeta(struct position *pos, int alpha, int beta, int depthleft, int nullmove, int ply, struct move *pv, clock_t endtime, int cut) {
	assert(pos);
	assert(alpha >= -MATE_SCORE && beta <= MATE_SCORE);
	assert(beta > alpha);
	if (ply > seldepth) seldepth = ply;
	int origdepthleft = depthleft;
	if (depthleft <= 0) depthleft = 0;
	if (clock() >= endtime) {
		return -MATE_SCORE;
	}
	currenthash = 0;
	if (isThreefold(pos)) return 0;
	if (pos->halfmoves >= 100) return 0;
	if (isInsufficientMaterial(pos)) return 0;
	int incheck = isCheck(pos);
	if (incheck) depthleft += ONE_PLY;
	if (depthleft <= 0) {
		return qSearch(pos, alpha, beta, ply + 1, endtime);
		//return taperedEval(pos);

	}
	struct move bestmove = {.to=-1,.from=-1,.prom=NONE,.cappiece=NONE};;
	struct move TTmove = {.to=-1,.from=-1,.prom=NONE,.cappiece=NONE};
	struct move nullref = {.to=-1,.from=-1,.prom=NONE,.cappiece=NONE};
	int origAlpha = alpha;
	int origBeta = beta;
	
	
	
	// Mate distance pruning
	
	if (ply != 0) {
		alpha = max(mated_in(ply), alpha);
		beta = min(mate_in(ply+1), beta);
		if (alpha >= beta) return alpha;
	}
	int staticeval = -MATE_SCORE - 100;
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
		int isvalid = 1;
		if (getColour(pos, TTdata.bestmove.from) != pos->tomove) isvalid = 0;
		if (getColour(pos, TTdata.bestmove.to) == pos->tomove) isvalid = 0;
		if (isvalid) {
			if (TTdata.depth >= origdepthleft) {
				int flag = TTdata.flag;
				int score = TTdata.score;
				
				if (flag == EXACT && TTdata.depth == origdepthleft) { // only return exact hits at exact depth match
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
			staticeval = TTdata.score;
		}
	}
	
	if (staticeval == -MATE_SCORE - 100) staticeval = taperedEval(pos);
	
	// eval pruning
	
	if (depthleft < 3 * ONE_PLY && !incheck && abs(beta) - 1 > -MATE_SCORE + 100) {
		int eval_margin = 120 * depthleft / ONE_PLY;
		if (staticeval - eval_margin >= beta) return staticeval - eval_margin;
	}
	
	// static null pruning (reverse futility pruning)
	
	
	if (!nullmove && beta <= MATE_SCORE) {
		if (depthleft == 1 * ONE_PLY && staticeval - 300 > beta) return beta;
		if (depthleft == 2 * ONE_PLY && staticeval - 525 > beta) return beta;
		if (depthleft == 3 * ONE_PLY && staticeval - 900 > beta) depthleft -= ONE_PLY;
	}
	
	// null move pruning
	
	int TTfailhigh = 0;
	if (TTdata.hash == hash && TTdata.flag != UPPERBOUND && TTdata.score >= beta) TTfailhigh = 1;
	
	if (!nullmove && !incheck && ply != 0 && depthleft >= 3 * ONE_PLY && (TTfailhigh || staticeval >= beta)) {
		const int orighalfmoves = pos->halfmoves;
		const int origepsquare = pos->epsquare;
		pos->tomove = !pos->tomove;
		pos->halfmoves = 0;
		pos->epsquare = -1;
		posstack[posstackend] = *pos;
		posstackend++;
		int verR = 3 * ONE_PLY;
		int R = 3 * ONE_PLY;
		if (gamephase(pos) >= 85) { // endgame
			// decrease search reduction
			
			R = 2 * ONE_PLY;
			verR = 2 * ONE_PLY;
		}
		const int val = -alphaBeta(pos,-beta,-beta+1, depthleft - ONE_PLY - R, 1, ply + 1, pv, endtime, !cut);
		U64 nullhash = generateHash(pos);
		struct TTentry nullTTdata = getTTentry(&TT, nullhash);
		if (nullhash == nullTTdata.hash) nullref = nullTTdata.bestmove;
		pos->tomove = !pos->tomove;
		pos->halfmoves = orighalfmoves;
		pos->epsquare = origepsquare;
		posstackend--;
		if (val >= origBeta || val >= beta) {
			const int verification = alphaBeta(pos,beta - 1,beta, depthleft - ONE_PLY - verR, 1, ply + 1, pv, endtime, !cut); // alpha_beta(p, md, beta - 1, beta, d, false, false);
			if (verification >= beta) return beta;
		}
	}
	
	int f_prune = 0;
	
	//int fmargin[4] = { 0, 200, 300, 500 };
	int fmargin[8] = {0, 150, 250, 350, 450, 550, 650, 750};
	if (depthleft <= 7 * ONE_PLY
	&&  !incheck
	&&   abs(alpha) < 9000
	&&   staticeval + fmargin[depthleft / ONE_PLY] <= alpha)
		 f_prune = 1;	
	
	// IID
	
	if (TTmove.from == -1 && depthleft >= 7 * ONE_PLY) {
		
		int newdepth = 3 * ONE_PLY;
		int val = alphaBeta(pos, alpha, beta, newdepth, 0, ply + 1, pv, endtime, !cut);
		TTdata = getTTentry(&TT, hash);
        if (TTdata.hash == hash) {
			int isvalid = 1;
			if (getColour(pos, TTdata.bestmove.from) != pos->tomove) isvalid = 0;
			if (getColour(pos, TTdata.bestmove.to) == pos->tomove) isvalid = 0;
			if (isvalid) TTmove = TTdata.bestmove;
        }
	}
	
	int bestscore = INT_MIN;
	int searchedKiller0 = 0;
	int searchedKiller1 = 0;
	int searchedCM = 0;
	int searchedTTmove = 0;
	int beatsbeta = 0;
	int legalmoves = 0;
	int quiets = 0;
	int extension = 0;
	struct move premoves[4];
	int num_premoves = 0;
	if (TTmove.from != -1) {
		searchedTTmove = 1;
		premoves[num_premoves] = TTmove;
		num_premoves++;
	}
	struct move prevmove = movestack[movestackend - 1];
	struct move countermove = countermoves[prevmove.from][prevmove.to];
	struct move curmove;
	for (int i = 0;i < num_premoves;i++) {
		curmove = premoves[i];
		extension = 0;
		
		if (curmove.piece == PAWN && pos->tomove == BLACK) {
			U64 BBarea = BBrank2 | BBrank3 | BBrank4 | BBrank5;
			if (gamephase(pos) >= 80) BBarea = ~0; // extend all passed pawn moves in endgame
			U64 BBpiece = 1ULL << curmove.from;
			if (BBpiece & BBarea) {
				// pawn is on rank 2-5
				U64 BBenemypawns = BBpasserLookup[BLACK][curmove.from] & (pos->colours[WHITE] & pos->pieces[PAWN]);
				if (!BBenemypawns) {
					// pawn is passed
					extension = 1 * ONE_PLY;
				}
			}
		}
		else if (curmove.piece == PAWN && pos->tomove == WHITE) {
			U64 BBarea = BBrank4 | BBrank5 | BBrank6 | BBrank7;
			if (gamephase(pos) >= 80) BBarea = ~0; // extend all passed pawn moves in endgame
			U64 BBpiece = 1ULL << curmove.from;
			if (BBpiece & BBarea) {
				// pawn is on rank 2-5
				U64 BBenemypawns = BBpasserLookup[WHITE][curmove.from] & (pos->colours[BLACK] & pos->pieces[PAWN]);
				if (!BBenemypawns) {
					// pawn is passed
					extension = 1 * ONE_PLY;
				}
			}
		}
		struct move lastmove = movestack[movestackend - 1];
		if (pieceval(lastmove.cappiece) == pieceval(lastmove.piece) && curmove.to == lastmove.to) {
			// recapture extension
			extension = ONE_PLY;
		}
		int r = 0;
		int SEEvalue = SEEcapture(pos, curmove.from, curmove.to, pos->tomove);
		if (SEEvalue < 0) r = ONE_PLY; // reduce bad captures
		makeMove(&curmove,pos);
		legalmoves++;
		if (curmove.cappiece == NONE) quiets++;
		
		int score = -alphaBeta(pos, -beta, -alpha, depthleft - ONE_PLY + extension - r, 0, ply + 1, pv, endtime, !cut);
		if (r > 0 && score > alpha) {
			score = -alphaBeta(pos, -beta, -alpha, depthleft - ONE_PLY + extension, 0, ply + 1, pv, endtime, !cut);
		}
		unmakeMove(pos);
		
		if (score > bestscore) {
			bestscore = score;
			bestmove = curmove;
		}
		if (bestscore >= alpha) {
			alpha = score;
		}
		if (alpha >= beta) {
			numbetacutoffs++;
			if (legalmoves == 1) numinstantbetacutoffs++;
			beatsbeta = 1;
			if (curmove.cappiece == NONE) {
				killers[ply][1] = killers[ply][0];
				killers[ply][0] = curmove;
				//history[pos->tomove][curmove.from][curmove.to] += pow(2.0,(double)depthleft);
				history[pos->tomove][curmove.from][curmove.to] += (depthleft / ONE_PLY) * (depthleft / ONE_PLY);
				countermoves[prevmove.from][prevmove.to] = curmove;
			}
			if (bestscore != 0 && bestscore <= MATE_SCORE - 100 && bestscore >= -MATE_SCORE + 100) addTTentry(&TT, hash, origdepthleft, LOWERBOUND, bestmove, bestscore);
			*pv = curmove;
			return score;
		}
		else {
			// no beta cut off
			if (curmove.cappiece == NONE) {
				//butterfly[pos->tomove][curmove.from][curmove.to] += pow(2.0,(double)depthleft);
				butterfly[pos->tomove][curmove.from][curmove.to] += (depthleft / ONE_PLY) * (depthleft / ONE_PLY);
			}
		}
	}
	extension = 0;
	struct move moves[MAX_MOVES];
	int num_moves = 0;
	if (!beatsbeta) {
		num_moves = genMoves(pos,moves, 0);
		sortMoves(pos,moves,num_moves,TTmove,ply);
	}
	
	// Prob Cut
	
	if (!beatsbeta && depthleft >= 5 * ONE_PLY && abs(beta) <= MATE_SCORE && staticeval >= beta && cut) {
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
			probcutscore = -alphaBeta(pos, -rbeta, -rbeta + 1, depthleft - 4 * ONE_PLY, 0, ply + 1, pv, endtime, !cut);
			unmakeMove(pos);
			if (probcutscore >= rbeta) {
				return probcutscore;
			}
		}
	}
	
	// multicut
	
	int MCR = 8;
	int MCC = 3;
	int MCM = 6;
	if (!incheck && depthleft >= MCR * ONE_PLY && cut) {
		int c = 0;
		for (int i = 0;i < min(MCM,num_moves);i++) {
			makeMove(&moves[i],pos);
			pos->tomove = !pos->tomove;
			if (isCheck(pos)) {
				unmakeMove(pos);
				continue;
			}
			pos->tomove = !pos->tomove;
			int score = -alphaBeta(pos, -beta, -alpha, depthleft - ONE_PLY - MCR * ONE_PLY, 0, ply + 1, pv, endtime, !cut);
			unmakeMove(pos);
			if (score >= beta) {
				c++;
				if (c == MCC) {
					return beta;
				}
			}
			else {
				if (i - c > MCM - MCC) break; // abort if we can't meet the cut off quota
			}
		}
	}
	
	int allorigdepthleft = depthleft;
	
	int score = 0;
	int fullwindow = 1;
	
	
	for (int i = 0;i < num_moves && !beatsbeta;i++) {
		depthleft = allorigdepthleft;
		
		int numpassers = 0;
		
		char piece = moves[i].piece;
		char cappiece = moves[i].cappiece;
		
		int histval = history[pos->tomove][moves[i].from][moves[i].to];
		int butterflyval = butterfly[pos->tomove][moves[i].from][moves[i].to];
		
		int isTTmove = 0;
		if (TTmove.from == moves[i].from && TTmove.to == moves[i].to && TTmove.prom == moves[i].prom) isTTmove = 1;
		
		if (isTTmove && searchedTTmove) continue; // already searched TT move
		
		int isKiller = 0;
		int isKiller0 = 0;
		int isKiller1 = 0;
		int isCM = 0;
		if (killers[ply][0].from == moves[i].from && killers[ply][0].to == moves[i].to && killers[ply][0].prom == moves[i].prom) {
			isKiller = 1;
			isKiller0 = 1;
		}
		if (killers[ply][1].from == moves[i].from && killers[ply][1].to == moves[i].to && killers[ply][1].prom == moves[i].prom) {
			isKiller = 1;
			isKiller1 = 1;
		}
		if (countermove.to == moves[i].to && countermove.from == moves[i].from && countermove.prom == moves[i].prom) isCM = 1;
		
		if (isKiller0 && searchedKiller0) continue; // already searched first killer
		if (isKiller1 && searchedKiller1) continue; // already searched second killer
		if (isCM && searchedCM) continue; // already searched countermove
		
		int histmargins[13] = { 120, 120, 120, 120, 150, 180, 180, 350, 550, 1000, 1500, 2000, 3000 };
		int histmargin;
		if (rootdepth <= 12) histmargin = histmargins[rootdepth];
		else histmargin = 3000;
		
		double cutoffpercent = ((double)histval * 100.0 / (double)(histval + butterflyval));
		
		int escapesnr = 0;
		if (nullref.to == moves[i].from) escapesnr = 1;
		
		// history pruning
		
		if (!escapesnr && !incheck && !nullmove && depthleft <= 21 * ONE_PLY && !isTTmove && moves[i].cappiece == NONE && !isKiller
			&& bestmove.from != -1 && legalmoves >= 4 && (histval + butterflyval) > histmargin && cutoffpercent < 1.25 && ply != 0) {
			continue;
		}
		
		// SEE pruning
		
		int SEEvalue = SEEcapture(pos, moves[i].from, moves[i].to, pos->tomove);
		if (depthleft <= 8 * ONE_PLY && bestscore > -MATE_SCORE && SEEvalue <= -80 * (depthleft / ONE_PLY) * (depthleft / ONE_PLY)) {
			continue;
		}
		int extension = 0;
		
		// Make the move
		
		makeMove(&moves[i],pos);
		pos->tomove = !pos->tomove;
		if (isCheck(pos)) {
			unmakeMove(pos);
			continue;
		}
		
		pos->tomove = !pos->tomove;
		
		nodesSearched++;
		int givescheck = isCheck(pos);
		legalmoves++;
		
		if (moves[i].cappiece == NONE) quiets++;
		 
		// futility pruning
		
		if (f_prune
		&& legalmoves > 1
		&&  moves[i].prom == NONE
		&& !givescheck
		&& ply != 0) {
			if (cappiece == NONE) {
			unmakeMove(pos);
			continue;
			}
		}
		int r = reduction(&moves[i], depthleft, cappiece, legalmoves, incheck, givescheck, ply);
		r = max(0, min(r,3 * ONE_PLY));
			
		if (cutoffpercent >= 20.0 && r == 2 * ONE_PLY) {
			// limit reduction of moves with good history to one ply
			r = ONE_PLY;
		}
		if (moves[i].piece == PAWN && pos->tomove == WHITE) {
			U64 BBarea = BBrank2 | BBrank3 | BBrank4 | BBrank5;
			if (gamephase(pos) >= 80) BBarea = ~0; // extend all passed pawn moves in endgame
			U64 BBpiece = 1ULL << moves[i].from;
			if (BBpiece & BBarea) {
				// pawn is on rank 2-5
				U64 BBenemypawns = BBpasserLookup[BLACK][moves[i].from] & (pos->colours[WHITE] & pos->pieces[PAWN]);
				if (!BBenemypawns) {
					// pawn is passed
					extension = 1 * ONE_PLY;
				}
			}
		}
		else if (moves[i].piece == PAWN && pos->tomove == BLACK) {
			U64 BBarea = BBrank4 | BBrank5 | BBrank6 | BBrank7;
			U64 BBpiece = 1ULL << moves[i].from;
			if (gamephase(pos) >= 80) BBarea = ~0; // extend all passed pawn moves in endgame
			if (BBpiece & BBarea) {
				// pawn is on rank 2-5
				U64 BBenemypawns = BBpasserLookup[WHITE][moves[i].from] & (pos->colours[BLACK] & pos->pieces[PAWN]);
				if (!BBenemypawns) {
					// pawn is passed
					extension = 1 * ONE_PLY;
				}
			}
		}
		
		struct move lastmove = movestack[movestackend - 2];
		if (pieceval(lastmove.cappiece) == pieceval(lastmove.piece) && moves[i].to == lastmove.to) {
			// recapture extension
			extension = ONE_PLY;
		}
		 
		//struct position lastpos = posstack[posstackend - 2];
		//int SEEvalue = SEEcapture(&lastpos, moves[i].from, moves[i].to, lastpos.tomove);
		if (SEEvalue < 0) depthleft -= ONE_PLY; // reduce bad captures
		 
		// PVS Search

		if (legalmoves == 1) {
			score = -alphaBeta(pos, -beta, -alpha, depthleft - ONE_PLY + extension, 0, ply + 1, pv, endtime, !cut);
		}
		else {
			// narrow window search with reductions
			score = -alphaBeta(pos, -alpha - 1, -alpha, depthleft - ONE_PLY - r + extension, 0, ply + 1, pv, endtime, !cut);
			if (score > alpha) {
				// full window research with no reduction
				score = -alphaBeta(pos, -beta, -alpha, depthleft - ONE_PLY + extension, 0, ply + 1, pv, endtime, !cut);
			}
		}
		
		// Redo search
		
		//if (r > 0 && score > alpha) {
		//	score = -alphaBeta(pos, -beta, -alpha, depthleft - ONE_PLY + extension, 0, ply + 1, pv, endtime, !cut);
		//}
		// Unmake the move
		
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
			if (cappiece == NONE) {
				killers[ply][1] = killers[ply][0];
				killers[ply][0] = moves[i];
				//history[pos->tomove][moves[i].from][moves[i].to] += pow(2.0,(double)depthleft);
				history[pos->tomove][moves[i].from][moves[i].to] += (depthleft / ONE_PLY) * (depthleft / ONE_PLY);
				struct move prevmove = movestack[movestackend - 1];
				countermoves[prevmove.from][prevmove.to] = moves[i];
			}
			break;
		}
		else {
			// no beta cut off
			if (cappiece == NONE) {
				//butterfly[pos->tomove][moves[i].from][moves[i].to] += pow(2.0,(double)depthleft);
				butterfly[pos->tomove][moves[i].from][moves[i].to] += (depthleft / ONE_PLY) * (depthleft / ONE_PLY);
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
	if (bestscore != 0 && bestscore <= MATE_SCORE - 100 && bestscore >= -MATE_SCORE + 100) addTTentry(&TT, hash, origdepthleft, newflag, bestmove, bestscore);
	addPVTTentry(&PVTT, hash, bestmove, bestscore);
	*pv = bestmove;
	assert(bestmove.to >= 0 && bestmove.to <= 63 && bestmove.from >= 0 && bestmove.from <= 63);
	return bestscore;
}
int gamephase(struct position *pos) {
	int opening = 0;
	int endgame = 100;
	int phase = finalEval(pos, &opening, &endgame);
	if (pos->tomove == BLACK) phase = -phase;
	return phase;
}
struct pvline getPV(struct position *pos, int depth) {
	struct pvline pvline;
	U64 hash = generateHash(pos);
	struct PVTTentry TTdata = getPVTTentry(&PVTT,hash);
	pvline.moves[0] = TTdata.bestmove;
	pvline.size = 1;
	int movesmade = 0;
	int movesunmade = 0;
	for (int i = 1;i < depth;i++) {
		struct move bestmove = TTdata.bestmove;
		int isvalid = 1;
		if (getColour(pos, TTdata.bestmove.from) != pos->tomove) isvalid = 0;
		if (getColour(pos, TTdata.bestmove.to) == pos->tomove) isvalid = 0;
		if (!isvalid) {
			pvline.size -= 1;
			break;
		}
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
		TTdata = getPVTTentry(&PVTT,hash);
		if (TTdata.hash != hash) break;
		pvline.moves[i] = TTdata.bestmove;
		pvline.size++;
	}
	for (int i = 0;i < (movesmade - movesunmade);i++) {
		unmakeMove(pos);
	}
	
	return pvline;
}
struct move search(struct position pos, int searchdepth, int movetime, int strictmovetime) {
	assert(searchdepth >= 0);
	assert(movetime > 0);
	// Reset stats
	nodesSearched = 0;
	numbetacutoffs = 0;
	numinstantbetacutoffs = 0;
	
	// Result
	struct move bestmove;
	
	seldepth = 0;
	
	double time_spent;
	int time_spentms;
	
	// Timing code
	const clock_t begin = clock();
	clock_t endtime = clock() + (movetime / 1000.0 * CLOCKS_PER_SEC);
	clock_t maxendtime = endtime + (movetime * 0.60 / 1000.0 * CLOCKS_PER_SEC);
	clock_t origendtime = endtime;
	
	assert(maxendtime > endtime);
	// Movegen
	struct move moves[MAX_MOVES];
	const int num_moves = genMoves(&pos, moves, 0);

	clearHistory();
	
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
	
	if (!strictmovetime && movestackend >= 1) {
		struct position lastpos = posstack[posstackend - 2];
		struct move lastmove = movestack[movestackend - 1];
		int SEEvalue = SEEcapture(&lastpos, lastmove.from, lastmove.to, lastpos.tomove);
		if (SEEvalue <= -300) {
		//	printf("sacrifice: %s\n", movetostr(lastmove));
		//	dspBoard(&lastpos);
			endtime = maxendtime;
		}
	}
	for(int d = 1; d <= searchdepth; ++d) {
		
		time_spent = (double)(clock() - begin) / CLOCKS_PER_SEC;
		time_spentms = (int)(time_spent*1000);
		rootdepth = d;
		
		// Check how many times the PV has changed in the last 4 depths
		
		int losingontime = 0;
		
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
		
		if (d > 4 && abs(score - lastscore) >= 0 && !losingontime && timeleftpercent > 30.0 && !strictmovetime) {
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
				clock_t newendtime = clock() + + (remaining_timems + remaining_timems * 0.015);
				if (newendtime > maxendtime) newendtime = maxendtime;
				endtime = newendtime;
			}
		}
		
		// Predict whether we have enough time for next search and break if not
		
		if (d > 1 && time_spentms > 30 && endtime == origendtime && !strictmovetime) {
			if (time_spent_prevms == 0) time_spent_prevms = 1;
			//double factor = time_spentms / time_spent_prevms;
			double expectedtime = time_spentms * 4;
			int expectedendtime = clock() + expectedtime;
			if (expectedendtime > endtime) break;
		}
		// Increase time to max if opponent's last move was a sacrifice
		//printf("%d::\n", movestackend);
		/*

		 */
		 
		if (endtime > totalendtime) endtime = totalendtime;
		score = alphaBeta(&pos, -MATE_SCORE, MATE_SCORE, d * ONE_PLY, 0, 0, &pv, endtime, 0);
		
		//Ignore the result if we ran out of time
		if (d > 1 && clock() >= endtime) {
			break;
		}

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
		if (!silentsearch) {
			printf("info");
			printf(" depth %i", d);
			printf(" seldepth %i", seldepth);
			printf(" nodes %" PRIu64, nodesSearched);
			printf(" time %i", (int)(time_spent*1000));
			if (time_spent > 0) printf(" nps %i", nps);
			printf(" score cp %i", score);
			struct pvline pvline = getPV(&pos,d);
			printf(" pv");
			int pvmatch = 0;
			if (pvline.moves[0].from == bestmove.from && pvline.moves[0].to == bestmove.to && pvline.moves[0].prom == bestmove.prom) pvmatch = 1;
			if (pvmatch) {
				for (int i = 0;i < pvline.size; i++) {
					printf(" %s",movetostr(pvline.moves[i]));
				}
			}
			else {
				// pv didn't match bestmove, just give bestmove to avoid returning illegal PVs
				printf(" %s", movetostr(bestmove));
			}
			printf("\n");
			//printf("NW research rate: %f\n", (double)((double)totnwresearches * (100 / (double)totnwsearches)));
			//printf("\n");
		}
		lastsearchdepth = d;
		if (score == MATE_SCORE || score == -MATE_SCORE) break;
	}
	time_spent = (double)(clock() - begin) / CLOCKS_PER_SEC;
	time_spentms = (int)(time_spent*1000);
	
	if (!silentsearch) {
		printf("info time %d", time_spentms);
		printf("\n");
		printf("bestmove %s\n", movetostr(bestmove));
	}
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
	if (capmove.piece != -1) {
		makeMove(&capmove, pos);
		value = max(0, pieceval(cappiece) - SEE(pos, square, !side));
		unmakeMove(pos);
	}
	return value;
}
struct move get_smallest_attacker(struct position *pos, int square, int side) {
	U64 BBmypieces;
	U64 BBopppieces;
	U64 BBsquare = 1ULL << square;
	U64 BBoccupied = pos->colours[WHITE] | pos->colours[BLACK];
	if (side == WHITE) BBmypieces = pos->colours[WHITE];
	else BBmypieces = pos->colours[BLACK];
	if (side == BLACK) BBopppieces = pos->colours[WHITE];
	else BBopppieces = pos->colours[BLACK];
	struct move blankmove = {.to=-1, .from=-1, .prom=-1, .piece=-1, .cappiece=-1};
	struct move returnmove = {.to=square, .from=-1, .prom=0, .piece=-1, .cappiece=getPiece(pos,square)};
	// pawns
	if (side == WHITE) {
		U64 BBattacks = soWeOne(BBsquare) | soEaOne(BBsquare);
		if (BBattacks & pos->pieces[PAWN] & BBmypieces) {
			returnmove.from = __builtin_ctzll(BBattacks & pos->pieces[PAWN] & BBmypieces);
			returnmove.piece = PAWN;
			return returnmove;
		}
	}
	else { // black
		U64 BBattacks = noWeOne(BBsquare) | noEaOne(BBsquare);
		if (BBattacks & pos->pieces[PAWN] & BBmypieces) {
			returnmove.from = __builtin_ctzll(BBattacks & pos->pieces[PAWN] & BBmypieces);
			returnmove.piece = PAWN;
			return returnmove;
		}
	}
	// Knights
	U64 BBattacks = BBknightLookup[square];
	if (BBattacks & BBmypieces & pos->pieces[KNIGHT]) {
		if (side == WHITE) {
			returnmove.from = __builtin_ctzll(BBattacks & pos->pieces[KNIGHT] & BBmypieces);
			returnmove.piece = KNIGHT;
			return returnmove;
		}
		else {
			returnmove.from = __builtin_ctzll(BBattacks & pos->pieces[KNIGHT] & BBmypieces);
			returnmove.piece = KNIGHT;
			return returnmove;
		}
	}
	// Bishops
	BBattacks = Bmagic(square, BBoccupied);
	if (BBattacks & BBmypieces & pos->pieces[BISHOP]) {
		if (side == WHITE) {
			returnmove.from = __builtin_ctzll(BBattacks & pos->pieces[BISHOP] & BBmypieces);
			returnmove.piece = BISHOP;
			return returnmove;
		}
		else {
			returnmove.from = __builtin_ctzll(BBattacks & pos->pieces[BISHOP] & BBmypieces);
			returnmove.piece = BISHOP;
			return returnmove;
		}
	}
	// Rooks
	BBattacks = Rmagic(square, BBoccupied);
	if (BBattacks & BBmypieces & pos->pieces[ROOK]) {
		if (side == WHITE) {
			returnmove.from = __builtin_ctzll(BBattacks & pos->pieces[ROOK] & BBmypieces);
			returnmove.piece = ROOK;
			return returnmove;
		}
		else {
			returnmove.from = __builtin_ctzll(BBattacks & pos->pieces[ROOK] & BBmypieces);
			returnmove.piece = ROOK;
			return returnmove;
		}
	}
	// Queens
	BBattacks = Rmagic(square, BBoccupied) | Bmagic(square, BBoccupied);
	if (BBattacks & BBmypieces & pos->pieces[QUEEN]) {
		if (side == WHITE) {
			returnmove.from = __builtin_ctzll(BBattacks & pos->pieces[QUEEN] & BBmypieces);
			returnmove.piece = QUEEN;
			return returnmove;
		}
		else {
			returnmove.from = __builtin_ctzll(BBattacks & pos->pieces[QUEEN] & BBmypieces);
			returnmove.piece = QUEEN;
			return returnmove;
		}
	}
	BBattacks = BBkingLookup[square];
	if (BBattacks & BBmypieces & pos->pieces[KING]) {
		if (side == WHITE) {
			returnmove.from = __builtin_ctzll(BBattacks & pos->pieces[KING] & BBmypieces);
			returnmove.piece = KING;
			return returnmove;
		}
		else {
			returnmove.from = __builtin_ctzll(BBattacks & pos->pieces[KING] & BBmypieces);
			returnmove.piece = KING;
			return returnmove;
		}
	}
	return blankmove;
}