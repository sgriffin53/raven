#include "makemove.h"
#include "position.h"
#include "move.h"
#include "bitboards.h"
#include <stdio.h>
#include <assert.h>
#include <math.h>

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
			pos->Wcastled = 1;
		}

		if ((fromsquare == E1) && (tosquare == C1)) { // white queenside castling
			setPiece(pos,E1, NONE, NONE);
			setPiece(pos,C1, colour, KING);
			setPiece(pos,D1, colour, ROOK);
			setPiece(pos,A1, NONE, NONE);
			pos->Wcastled = 1;
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
			pos->Bcastled = 1;
		}
		if ((fromsquare == E8) && (tosquare == C8)) { // black queenside castling
			setPiece(pos,E8, NONE, NONE);
			setPiece(pos,C8, colour, KING);
			setPiece(pos,D8, colour, ROOK);
			setPiece(pos,A8, NONE, NONE);
			pos->Bcastled = 1;
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
	pos->irrevidx++;
	pos->irrev[pos->irrevidx].epsquare = pos->epsquare;
	pos->irrev[pos->irrevidx].WcastleQS = pos->WcastleQS;
	pos->irrev[pos->irrevidx].WcastleKS = pos->WcastleKS;
	pos->irrev[pos->irrevidx].BcastleQS = pos->BcastleQS;
	pos->irrev[pos->irrevidx].WcastleKS = pos->BcastleKS;
	pos->irrev[pos->irrevidx].Wcastled = pos->Wcastled;
	pos->irrev[pos->irrevidx].Bcastled = pos->Bcastled;
	pos->irrev[pos->irrevidx].halfmoves = pos->halfmoves;
	pos->irrev[pos->irrevidx].Wkingpos = pos->Wkingpos;
	pos->irrev[pos->irrevidx].Bkingpos = pos->Bkingpos;
}
void unmakeMove(const struct move *move, struct position *pos) {
	pos->irrevidx--;
	pos->epsquare = pos->irrev[pos->irrevidx].epsquare;
	pos->WcastleQS = pos->irrev[pos->irrevidx].WcastleQS;
	pos->WcastleKS = pos->irrev[pos->irrevidx].WcastleKS;
	pos->BcastleQS = pos->irrev[pos->irrevidx].BcastleQS;
	pos->BcastleKS = pos->irrev[pos->irrevidx].WcastleKS;
	pos->Wcastled = pos->irrev[pos->irrevidx].Wcastled;
	pos->Bcastled = pos->irrev[pos->irrevidx].Bcastled;
	pos->halfmoves = pos->irrev[pos->irrevidx].halfmoves;
	pos->Wkingpos = pos->irrev[pos->irrevidx].Wkingpos = pos->Wkingpos;
	pos->Bkingpos = pos->irrev[pos->irrevidx].Wkingpos = pos->Bkingpos;
	pos->tomove = !pos->tomove;
	
	U64 from = 1ULL << move->from;
	U64 to = 1ULL << move->to;
	
	switch (move->type) {
		case NORMAL:
			pos->pieces[move->piece] ^= to | from;
			pos->colours[pos->tomove] ^= to | from;
			break;
		case CAPTURE:
			pos->pieces[move->piece] ^= to | from;
			pos->colours[pos->tomove] ^= to | from;
			pos->pieces[move->cappiece] ^= to;
			pos->colours[!pos->tomove] ^= to;
			break;
		case DOUBLE:
			pos->pieces[move->piece] ^= to | from;
			pos->colours[pos->tomove] ^= to | from;
			break;
		case PROMO:
		case PROMO_CAPTURE:
			pos->pieces[move->piece] ^= from;
			pos->pieces[move->prom] ^= to;
			pos->colours[pos->tomove] ^= to | from;
			if (move->cappiece) {
				pos->pieces[move->cappiece] ^= to;
				pos->colours[!pos->tomove] ^= to;
			}
			break;
		case EN_PASSANT:
			pos->pieces[move->piece] ^= to | from;
			pos->colours[pos->tomove] ^= to | from;
			if (pos->tomove == WHITE) {
				pos->pieces[PAWN] ^= to >> 8;
				pos->colours[BLACK] ^= to >> 8;
			}
			else {
				pos->pieces[PAWN] ^= to << 8;
				pos->colours[WHITE] ^= to << 8;
			}
			break;
		case KSC:
			pos->pieces[KING] ^= ksc_king[pos->tomove];
			pos->pieces[ROOK] ^= ksc_rook[pos->tomove];
			pos->colours[pos->tomove] ^= (ksc_king[pos->tomove] | ksc_rook[pos->tomove]);
			break;
		case QSC:
			pos->pieces[KING] ^= qsc_king[pos->tomove];
			pos->pieces[ROOK] ^= qsc_rook[pos->tomove];
			pos->colours[pos->tomove] ^= (qsc_king[pos->tomove] | qsc_rook[pos->tomove]);
			break;
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
	int isep = 0;
	if (piece == PAWN && endsquareidx == pos->epsquare) {
		if (pos->tomove == WHITE) {
			if (startsquareidx == endsquareidx - 11 || startsquareidx == endsquareidx - 9) {
				isep = 1;
			}
		}
		else if (pos->tomove == BLACK) {
			if (startsquareidx == endsquareidx + 11 || startsquareidx == endsquareidx + 9) {
				isep = 1;
			}
		}
	}
	int isdouble = 0;
	if (piece == PAWN) {
		if (pos->tomove == WHITE) {
			if (getrank(startsquareidx) == 1 && getrank(endsquareidx) == 3) {
				isdouble = 1;
			}
		}
		else if (pos->tomove == BLACK) {
			if (getrank(startsquareidx) == 6 && getrank(endsquareidx) == 4) {
				isdouble = 1;
			}
		}
	}
	int isqsc = 0;
	int isksc = 0;
	if (piece == KING) {
		if (pos->tomove == WHITE) {
			if (startsquareidx == E1 && endsquareidx == G1) isksc = 1;
			else if (startsquareidx == E1 && endsquareidx == C1) isqsc = 1;
		}
		if (pos->tomove == BLACK) {
			if (startsquareidx == E8 && endsquareidx == G8) isksc = 1;
			else if (startsquareidx == E8 && endsquareidx == C8) isqsc = 1;
		}
	}
	char movetype = NORMAL;
	if (cappiece != NONE && prom != NONE) movetype = PROMO_CAPTURE;
	else if (prom != NONE) movetype = PROMO;
	else if (isep) movetype = EN_PASSANT;
	else if (isdouble) movetype = DOUBLE;
	else if (isksc) movetype = KSC;
	else if (isqsc) movetype = QSC;
	struct move moveobj = {.from=startsquareidx,.to=endsquareidx,.prom=prom,.cappiece=cappiece,.piece=piece, .type=movetype};
	makeMove(&moveobj, pos);
}