
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "position.h"
#include "makemove.h"
#include "assert.h"
#include "globals.h"

void makeMove(const struct move *move, struct position *pos) {
	assert(move);
	assert(pos);
	pos->halfmoves += 1;
	char piece = getPiece(pos,move->from);
	char cappiece = getPiece(pos,move->to);
	int newepsquare = -1; // init to -1, change to ep square if there is one
	int torank = getrank(move->to);
	char prompiece = move->prom;
	int tosquare = move->to;
	int fromsquare = move->from;
	int epsquare = pos->epsquare;
	setPiece(pos,fromsquare,'0');
	setPiece(pos,tosquare,piece);
	if (cappiece != '0') {
		pos->halfmoves = 0;
	}
	if (piece == 'P') {
		pos->halfmoves = 0;
		if (torank == 7) {
			// promotion
			setPiece(pos,tosquare,toupper(prompiece));
			pos->BBwhitepieces |= (1ULL << tosquare);
			if (prompiece == 'q') {
				pos->BBqueens |= (1ULL << tosquare);
			}
			else if (prompiece == 'r') {
				pos->BBrooks |= (1ULL << tosquare);
			}
			else if (prompiece == 'b') {
				pos->BBbishops |= (1ULL << tosquare);
			}
			else if (prompiece == 'n') {
				pos->BBknights |= (1ULL << tosquare);
			}
		}
		if (move->to == epsquare) {
			// pawn moves en passant
			// remove captured piece
			setPiece(pos,epsquare - 8,'0');
		}
		if (abs(fromsquare - tosquare) == 16) {
			newepsquare = tosquare - 8;
		}
	}
	else if (piece == 'p') {
		pos->halfmoves = 0;
		if (torank == 0) {
			// promotion
			setPiece(pos,tosquare,prompiece);
			pos->BBblackpieces |= (1ULL << tosquare);
			if (prompiece == 'q') {
				pos->BBqueens |= (1ULL << tosquare);
			}
			else if (prompiece == 'r') {
				pos->BBrooks |= (1ULL << tosquare);
			}
			else if (prompiece == 'b') {
				pos->BBbishops |= (1ULL << tosquare);
			}
			else if (prompiece == 'n') {
				pos->BBknights |= (1ULL << tosquare);
			}
		}
		if (move->to == epsquare) {
			// pawn moves en passant
			// remove captured piece
			//printf("setting %d to 0",pos->epsquare - 8);
			setPiece(pos,epsquare + 8,'0');
		}
		if (abs(fromsquare - tosquare) == 16) {
			//pawn moves 2 spaces forward
			newepsquare = tosquare+ 8;
		}
	}
	else if (piece == 'K') { // white king
		pos->WcastleQS = 0;
		pos->WcastleKS = 0;
		pos->Wkingpos = tosquare;

		if ((fromsquare == E1) && (tosquare == G1)) { // white kingside castling
			setPiece(pos,E1,'0');
			setPiece(pos,G1,'K');
			setPiece(pos,F1,'R');
			setPiece(pos,H1,'0');
		}

		if ((fromsquare == E1) && (tosquare == C1)) { // white queenside castling
			setPiece(pos,E1,'0');
			setPiece(pos,C1,'K');
			setPiece(pos,D1,'R');
			setPiece(pos,A1,'0');
		}
	}
	else if (piece == 'k') { // black king
		pos->BcastleQS = 0;
		pos->BcastleKS = 0;
		pos->Bkingpos = tosquare;
		if ((fromsquare == E8) && (tosquare == G8)) { // black kingside castling
			setPiece(pos,E8,'0');
			setPiece(pos,G8,'k');
			setPiece(pos,F8,'r');
			setPiece(pos,H8,'0');
		}
		if ((fromsquare == E8) && (tosquare == C8)) { // black queenside castling
			setPiece(pos,E8,'0');
			setPiece(pos,C8,'k');
			setPiece(pos,D8,'r');
			setPiece(pos,A8,'0');
		}
	}
	/*
	if (getPiece(pos,A8) != 'r') { // black a8 rook moved or captured
		pos->BcastleQS = 0;
	}
	if (getPiece(pos,H8) != 'r') { // black h8 rook moved or captured
		pos->BcastleKS = 0;
	}
	if (getPiece(pos,A1) != 'R') { // white a1 rook moved or captured
		pos->WcastleQS = 0;
	}
	if (getPiece(pos,H1) != 'R') { // white h1 rook moved or captured
		pos->WcastleKS = 0;
	}
	 */
	if (piece == 'r') {
		if (fromsquare == A8) {
			pos->BcastleQS = 0;
		}
		else if (fromsquare == H8) {
			pos->BcastleKS = 0;
		}
	}
	else if (piece == 'R') {
		if (fromsquare == A1) {
			pos->WcastleQS = 0;
		}
		else if (fromsquare == H1) {
			pos->WcastleKS = 0;
		}
	}
	if (cappiece == 'r') {
		if (tosquare == A8) {
			pos->BcastleQS = 0;
		}
		else if (tosquare == H8) {
			pos->BcastleKS = 0;
		}
	}
	else if (cappiece == 'R') {
		if (tosquare == A1) {
			pos->WcastleQS = 0;
		}
		else if (tosquare == H1) {
			pos->WcastleKS = 0;
		}
	}
	pos->tomove = !pos->tomove;
	pos->epsquare = newepsquare;
	posstack[posstackend] = *pos;
	posstackend++;
}
void unmakeMove(struct position *pos) {
	assert(pos);
	posstackend--;
	if (posstackend <= 0) {
		posstackend = 1;
		*pos = posstack[0];
	}
	else {
	*pos = posstack[posstackend - 1];
	}
}
void makeMovestr(const char move[], struct position *pos) {
	assert(move);
	assert(pos);
	char startsquare[3];
	char endsquare[3];
	char prompiece[2];

	startsquare[0] = move[0];
	startsquare[1] = move[1];
	startsquare[2] = 0;
	endsquare[0] = move[2];
	endsquare[1] = move[3];
	endsquare[2] = 0;
	prompiece[0] = move[4];
	prompiece[1] = 0;

	int startsquareidx = strsquaretoidx(startsquare);
	int endsquareidx = strsquaretoidx(endsquare);

	struct move moveobj = {.from=startsquareidx,.to=endsquareidx,.prom=prompiece[0]};

	makeMove(&moveobj, pos);
}