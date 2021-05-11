#include <time.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "search.h"
#include "../chess/position.h"
#include "eval.h"
#include "../chess/attacks.h"
#include "../chess/makemove.h"
#include "../chess/movegen.h"
#include "../globals.h"
#include "../chess/draw.h"
#include "sort.h"
#include "../misc.h"
#include "TT.h"
#include "../chess/move.h"
#include "../chess/bitboards.h"
#include "../chess/magicmoves.h"

#define ONE_PLY 4
#define MAX_DEPTH 100

int mate_in(int ply) {
	return MATE_SCORE - ply;
}
int mated_in(int ply) {
	return -MATE_SCORE + ply;
}
int reduction(const struct move *move, const int depthleft, char cappiece, int legalmoves, int incheck, int givescheck) {
	assert(move);
	assert(depthleft >= 0);
	if ((!incheck) && (legalmoves > 4) && (depthleft >= 3 * ONE_PLY) && (move->prom == NONE) && (!givescheck)) {
		if (cappiece == NONE) {
			int red = ONE_PLY;
			if (depthleft >= 6 * ONE_PLY) red = 2 * ONE_PLY;
			if (legalmoves >= 20) red += ONE_PLY;
			if (move->piece == PAWN) red = ONE_PLY;
			return red;
		}
	}

	return 0;
}

clock_t getClock() {
	struct timespec tp;
	clock_gettime(CLOCK_REALTIME, &tp);
	return ((clock_t)tp.tv_sec * 1000 + tp.tv_nsec / 1000000);
}

int outOfTime(clock_t endtime) {
	static int counter = 0;

	if (counter == 0) {
		counter = 512;
		return getClock() >= endtime;
	}
	--counter;
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
	if (outOfTime(endtime)) {
		return NO_SCORE;
	}
	struct move TTmove = {.to=-1,.from=-1,.prom=NONE,.cappiece=NONE};
	const int standpat = taperedEval(pos);
	if (standpat >= beta) {
		return beta;
	}
	
	// delta pruning

	if (alpha < standpat) alpha = standpat;

	struct move moves[MAX_MOVES];
	const int num_moves = genMoves(pos,moves, 1);
	
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

		if (score == -NO_SCORE) {
			return NO_SCORE;
		}
		if (score >= beta) {
			return beta;
		}
		if (score > alpha) {
			alpha = score;
			bestmove = moves[i];
		}
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
	if (outOfTime(endtime)) {
		return NO_SCORE;
	}
	if (isThreefold(pos)) return 0;
	if (pos->halfmoves >= 100) return 0;
	if (isInsufficientMaterial(pos)) return 0;
	int incheck = isCheck(pos);
	if (incheck) depthleft += ONE_PLY; // check extensions
	if (depthleft <= 0) {
		return qSearch(pos, alpha, beta, ply + 1, endtime);
	}
	struct move bestmove = {.to=-1,.from=-1,.prom=NONE,.cappiece=NONE};;
	struct move TTmove = {.to=-1,.from=-1,.prom=NONE,.cappiece=NONE};
	struct move nullref = {.to=-1,.from=-1,.prom=NONE,.cappiece=NONE};
	int origAlpha = alpha;
	int origBeta = beta;
	
	
	
	// TT lookup
	
	U64 hash = generateHash(pos);
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
		}
	}
	
	int staticeval = taperedEval(pos); // get static eval


	if (depthleft < 3 * ONE_PLY && !incheck && abs(beta) - 1 > -MATE_SCORE + 100) {
		int eval_margin = 120 * depthleft / ONE_PLY;
		if (staticeval - eval_margin >= beta) return staticeval - eval_margin;
	}
	
	// null move pruning

	if (!nullmove && !incheck && ply != 0 && depthleft >= 3 * ONE_PLY && (staticeval >= beta)) {
		const int orighalfmoves = pos->halfmoves;
		const int origepsquare = pos->epsquare;
		pos->tomove = !pos->tomove;
		pos->halfmoves = 0;
		pos->epsquare = -1;
		posstack[posstackend] = *pos;
		posstackend++;
		int verR = 3 * ONE_PLY;
		int R = 3 * ONE_PLY;
		const int val = -alphaBeta(pos,-beta,-beta+1, depthleft - ONE_PLY - R, 1, ply + 1, pv, endtime, !cut);
		if (val == -NO_SCORE) {
			return NO_SCORE;
		}
		U64 nullhash = generateHash(pos);
		struct TTentry nullTTdata = getTTentry(&TT, nullhash);
		if (nullhash == nullTTdata.hash) nullref = nullTTdata.bestmove;
		pos->tomove = !pos->tomove;
		pos->halfmoves = orighalfmoves;
		pos->epsquare = origepsquare;
		posstackend--;
		if (val >= origBeta || val >= beta) {
			const int verification = alphaBeta(pos,beta - 1,beta, depthleft - ONE_PLY - verR, 1, ply + 1, pv, endtime, !cut); // alpha_beta(p, md, beta - 1, beta, d, false, false);
			if (verification == NO_SCORE) {
				return NO_SCORE;
			}
			if (verification >= beta) return beta;
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
	extension = 0;
	struct move moves[MAX_MOVES];
	int num_moves = genMoves(pos,moves, 0);
	sortMoves(pos,moves,num_moves,TTmove, ply);
	
	int allorigdepthleft = depthleft;
	
	int score = 0;
	
	for (int i = 0;i < num_moves && !beatsbeta;i++) {
		depthleft = allorigdepthleft;
		
		char cappiece = moves[i].cappiece;
		
		int ext = 0;
		
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
		int r = reduction(&moves[i], depthleft, cappiece, legalmoves, incheck, givescheck);
		
		score = -alphaBeta(pos, -beta, -alpha, depthleft - ONE_PLY - r, 0, ply + 1, pv, endtime, !cut);
		if (r > 0 && score > alpha) {
			score = -alphaBeta(pos, -beta, -alpha, depthleft - ONE_PLY, 0, ply + 1, pv, endtime, !cut);
		}
		
		// Unmake the move
		
		unmakeMove(pos);

		if (score == -NO_SCORE) {
			return NO_SCORE;
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
				//struct move prevmove = movestack[movestackend - 1];
				//countermoves[prevmove.from][prevmove.to] = moves[i];
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
			return -MATE_SCORE + ply;
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
	if (bestscore <= MATE_SCORE - 100 && bestscore >= -MATE_SCORE + 100) addTTentry(&TT, hash, origdepthleft, newflag, bestmove, bestscore);
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
	const clock_t begin = getClock();
	clock_t endtime = begin + movetime;
	//clock_t maxendtime = endtime + (movetime * 0.30 / 1000.0 * CLOCKS_PER_SEC);
	clock_t origendtime = endtime;
	
	//assert(maxendtime > endtime);
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
	int time_spent_prevms = 0;
	int score = 0;
	
	for(int d = 1; d <= min(MAX_DEPTH, searchdepth); ++d) {
		
		time_spentms = getClock() - begin;
		//time_spent = time_spentms / 1000.0;
		rootdepth = d;
		
		// Predict whether we have enough time for next search and break if not
		
		score = alphaBeta(&pos, -MATE_SCORE, MATE_SCORE, d * ONE_PLY, 0, 0, &pv, endtime, 0);
		
		//Ignore the result if we ran out of time
		if (d > 1 && score == NO_SCORE) {
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
		time_spentms = getClock() - begin;
		time_spent = time_spentms / 1000.0;
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
	time_spentms = getClock() - begin;
	time_spent = time_spentms / 1000.0;
	
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
	U64 BBsquare = 1ULL << square;
	U64 BBoccupied = pos->colours[WHITE] | pos->colours[BLACK];
	if (side == WHITE) BBmypieces = pos->colours[WHITE];
	else BBmypieces = pos->colours[BLACK];
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
