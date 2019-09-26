
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
	int piece = move->piece;
	int cappiece = move->cappiece;
	int piececol = getPieceCol(pos, move->from); 
	int cappiececol = getPieceCol(pos, move->to); 
	int newepsquare = -1; // init to -1, change to ep square if there is one
	int torank = getrank(move->to);
	int prompiece = move->prom;
	int tosquare = move->to;
	int fromsquare = move->from;
	int epsquare = pos->epsquare;
	setPiece(pos,fromsquare, -1, -1);
	setPiece(pos,tosquare,piece, pos->tomove);
	if (cappiece != -1) {
		pos->halfmoves = 0;
	}
	if (piece == PAWN && piececol == WHITE) {
		pos->halfmoves = 0;
		if (torank == 7) {
			// promotion
			setPiece(pos,tosquare,prompiece, WHITE);
			pos->colours[WHITE] |= (1ULL << tosquare);
			pos->pieces[prompiece] |= (1ULL << tosquare);
		}
		if (move->to == epsquare) {
			// pawn moves en passant
			// remove captured piece
			setPiece(pos,epsquare - 8, -1, -1);
		}
		if (abs(fromsquare - tosquare) == 16) {
			newepsquare = tosquare - 8;
		}
	}
	else if (piece == PAWN && piececol == BLACK) {
		pos->halfmoves = 0;
		if (torank == 0) {
			// promotion
			setPiece(pos,tosquare,prompiece, BLACK);
			pos->colours[BLACK] |= (1ULL << tosquare);
			pos->pieces[prompiece] |= (1ULL << tosquare);
		}
		if (move->to == epsquare) {
			// pawn moves en passant
			// remove captured piece
			//printf("setting %d to 0",pos->epsquare - 8);
			setPiece(pos,epsquare + 8, -1, -1);
		}
		if (abs(fromsquare - tosquare) == 16) {
			//pawn moves 2 spaces forward
			newepsquare = tosquare+ 8;
		}
	}
	else if (piece == KING && piececol == WHITE) { // white king
		pos->WcastleQS = 0;
		pos->WcastleKS = 0;
		pos->Wkingpos = tosquare;

		if ((fromsquare == E1) && (tosquare == G1)) { // white kingside castling
			setPiece(pos,E1, -1, -1);
			setPiece(pos,G1, KING, WHITE);
			setPiece(pos,F1,ROOK, WHITE);
			setPiece(pos,H1, -1, -1);
		}

		if ((fromsquare == E1) && (tosquare == C1)) { // white queenside castling
			setPiece(pos,E1, -1, -1);
			setPiece(pos,C1, KING, WHITE);
			setPiece(pos,D1, ROOK, WHITE);
			setPiece(pos,A1, -1, -1);
		}
	}
	else if (piece == KING && piececol == BLACK) { // black king
		pos->BcastleQS = 0;
		pos->BcastleKS = 0;
		pos->Bkingpos = tosquare;
		if ((fromsquare == E8) && (tosquare == G8)) { // black kingside castling
			setPiece(pos,E8, -1, -1);
			setPiece(pos,G8, KING, BLACK);
			setPiece(pos,F8, ROOK, BLACK);
			setPiece(pos,H8, -1, -1);
		}
		if ((fromsquare == E8) && (tosquare == C8)) { // black queenside castling
			setPiece(pos,E8, -1, -1);
			setPiece(pos,C8, KING, BLACK);
			setPiece(pos,D8, ROOK, BLACK);
			setPiece(pos,A8, -1, -1);
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
	if (piece == ROOK && piececol == BLACK) {
		if (fromsquare == A8) {
			pos->BcastleQS = 0;
		}
		else if (fromsquare == H8) {
			pos->BcastleKS = 0;
		}
	}
	else if (piece == ROOK && piececol == WHITE) {
		if (fromsquare == A1) {
			pos->WcastleQS = 0;
		}
		else if (fromsquare == H1) {
			pos->WcastleKS = 0;
		}
	}
	if (cappiece == ROOK && cappiececol == BLACK) {
		if (tosquare == A8) {
			pos->BcastleQS = 0;
		}
		else if (tosquare == H8) {
			pos->BcastleKS = 0;
		}
	}
	else if (cappiece == ROOK && cappiececol == WHITE) {
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
	int cappiece = getPiece(pos, endsquareidx);
	int piece = getPiece(pos,startsquareidx);
	int prompieceint;
	switch (prompiece[0]) {
		case '0': prompieceint = -1;
		case 'n': prompieceint = KNIGHT;
		case 'b': prompieceint = BISHOP;
		case 'r': prompieceint = ROOK;
		case 'q': prompieceint = QUEEN;
	}
	struct move moveobj = {.from=startsquareidx,.to=endsquareidx,.prom=prompieceint,.cappiece=cappiece,.piece=piece};

	makeMove(&moveobj, pos);
}