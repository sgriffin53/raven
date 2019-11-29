
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
	char piece = move->piece;
	char cappiece = move->cappiece;
	int newepsquare = -1; // init to -1, change to ep square if there is one
	int torank = getrank(move->to);
	char prompiece = move->prom;
	int tosquare = move->to;
	int fromsquare = move->from;
	int epsquare = pos->epsquare;
	int colour = pos->tomove;
	setPiece(pos,fromsquare, NONE, NONE);
	setPiece(pos,tosquare,pos->tomove, piece);
	if (cappiece != NONE) {
		pos->halfmoves = 0;
	}
	if (piece == PAWN && colour == WHITE) {
		pos->halfmoves = 0;
		if (torank == 7) {
			// promotion
			setPiece(pos,tosquare, colour, prompiece);
			pos->colours[WHITE] |= (1ULL << tosquare);
			if (prompiece == QUEEN) {
				pos->pieces[QUEEN] |= (1ULL << tosquare);
			}
			else if (prompiece == ROOK) {
				pos->pieces[ROOK] |= (1ULL << tosquare);
			}
			else if (prompiece == BISHOP) {
				pos->pieces[BISHOP] |= (1ULL << tosquare);
			}
			else if (prompiece == KNIGHT) {
				pos->pieces[KNIGHT] |= (1ULL << tosquare);
			}
		}
		if (move->to == epsquare) {
			// pawn moves en passant
			// remove captured piece
			setPiece(pos,epsquare - 8,NONE, NONE);
		}
		if (abs(fromsquare - tosquare) == 16) {
			newepsquare = tosquare - 8;
		}
	}
	else if (piece == PAWN && colour == BLACK) {
		pos->halfmoves = 0;
		if (torank == 0) {
			// promotion
			setPiece(pos,tosquare, colour, prompiece);
			pos->colours[BLACK] |= (1ULL << tosquare);
			if (prompiece == QUEEN) {
				pos->pieces[QUEEN] |= (1ULL << tosquare);
			}
			else if (prompiece == ROOK) {
				pos->pieces[ROOK] |= (1ULL << tosquare);
			}
			else if (prompiece == BISHOP) {
				pos->pieces[BISHOP] |= (1ULL << tosquare);
			}
			else if (prompiece == KNIGHT) {
				pos->pieces[KNIGHT] |= (1ULL << tosquare);
			}
		}
		if (move->to == epsquare) {
			// pawn moves en passant
			// remove captured piece
			setPiece(pos,epsquare + 8, NONE, NONE);
		}
		if (abs(fromsquare - tosquare) == 16) {
			//pawn moves 2 spaces forward
			newepsquare = tosquare+ 8;
		}
	}
	else if (piece == KING && colour == WHITE) { // white king
		pos->WcastleQS = 0;
		pos->WcastleKS = 0;
		pos->Wkingpos = tosquare;

		if ((fromsquare == E1) && (tosquare == G1)) { // white kingside castling
			setPiece(pos,E1, NONE, NONE);
			setPiece(pos,G1, colour, KING);
			setPiece(pos,F1, colour, ROOK);
			setPiece(pos,H1, NONE, NONE);
		}

		if ((fromsquare == E1) && (tosquare == C1)) { // white queenside castling
			setPiece(pos,E1, NONE, NONE);
			setPiece(pos,C1, colour, KING);
			setPiece(pos,D1, colour, ROOK);
			setPiece(pos,A1, NONE, NONE);
		}
	}
	else if (piece == KING && colour == BLACK) { // black king
		pos->BcastleQS = 0;
		pos->BcastleKS = 0;
		pos->Bkingpos = tosquare;
		if ((fromsquare == E8) && (tosquare == G8)) { // black kingside castling
			setPiece(pos,E8, NONE, NONE);
			setPiece(pos,G8, colour, KING);
			setPiece(pos,F8, colour, ROOK);
			setPiece(pos,H8, NONE, NONE);
		}
		if ((fromsquare == E8) && (tosquare == C8)) { // black queenside castling
			setPiece(pos,E8, NONE, NONE);
			setPiece(pos,C8, colour, KING);
			setPiece(pos,D8, colour, ROOK);
			setPiece(pos,A8, NONE, NONE);
		}
	}
	if (piece == ROOK && colour == BLACK) {
		if (fromsquare == A8) {
			pos->BcastleQS = 0;
		}
		else if (fromsquare == H8) {
			pos->BcastleKS = 0;
		}
	}
	else if (piece == ROOK && colour == WHITE) {
		if (fromsquare == A1) {
			pos->WcastleQS = 0;
		}
		else if (fromsquare == H1) {
			pos->WcastleKS = 0;
		}
	}
	if (cappiece == ROOK && colour == WHITE) {
		if (tosquare == A8) {
			pos->BcastleQS = 0;
		}
		else if (tosquare == H8) {
			pos->BcastleKS = 0;
		}
	}
	else if (cappiece == ROOK && colour == BLACK) {
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
	movestack[movestackend] = *move;
	movestackend++;
	hashstackend = posstackend;
}
void unmakeMove(struct position *pos) {
	assert(pos);
	hashstack[hashstackend - 1] = 0;
	posstackend--;
	movestackend--;
	hashstackend--;
	if (posstackend <= 0) {
		posstackend = 1;
		movestackend = 1;
		hashstackend = 1;
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
	char cappiece = getPiece(pos, endsquareidx);
	char piece = getPiece(pos,startsquareidx);
	char prom = NONE;
	switch (prompiece[0]) {
		case 'q': prom = QUEEN; break;
		case 'r': prom = ROOK; break;
		case 'b': prom = BISHOP; break;
		case 'n': prom = KNIGHT; break;
	}
	struct move moveobj = {.from=startsquareidx,.to=endsquareidx,.prom=prom,.cappiece=cappiece,.piece=piece};

	makeMove(&moveobj, pos);
}