#include <assert.h>
#include <ctype.h>
#include "makemove.h"
#include "globals.h"

void makeMove(const struct move *move, struct position *pos) {
	assert(move);
	assert(pos);

	char piece = pos->board[move->from];
	int newepsquare = -1; // init to -1, will be changed to en passant square if there is one
	int torank = getrank(move->to);
	int cappiece = pos->board[move->to];
	pos->board[move->to] = piece;
	pos->board[move->from] = '0';

	pos->halfmoves = pos->halfmoves + 1;

	if (cappiece != '0') {
		pos->halfmoves = 0;
	}

	if (piece == 'P') { // white pawn
		if (torank == 0) { // promotion
			pos->board[move->to] = toupper(move->prom);
		}

		if (move->to == pos->epsquare) { // white en passant move
			pos->board[(pos->epsquare + 8)] = '0'; // remove captured piece
		}

		if ((move->from - move->to) == 16) { // pawn moved 2 spaces forward
			newepsquare = move->to + 8; // set ep square
		}

		pos->halfmoves = 0;
	}

	if (piece == 'p') { // black pawn
		if (torank == 7) { // promotion
			pos->board[move->to] = move->prom;
		}

		if (move->to == pos->epsquare) { // black en passant move
			pos->board[(pos->epsquare - 8)] = '0'; // remove captured piece
		}

		if ((move->to - move->from) == 16) { // pawn moved 2 spaces forward
			newepsquare = move->to - 8; // set ep square
		}

		pos->halfmoves = 0;
	}

	if (piece == 'K') { // white king
		pos->WcastleQS = 0;
		pos->WcastleKS = 0;
		pos->Wkingpos = move->to;

		if ((move->from == E1) && (move->to == G1)) { // white kingside castling
			pos->board[E1] = '0';
			pos->board[G1] = 'K';
			pos->board[F1] = 'R';
			pos->board[H1] = '0';
		}

		if ((move->from == E1) && (move->to == C1)) { // white queenside castling
			pos->board[E1] = '0';
			pos->board[C1] = 'K';
			pos->board[D1] = 'R';
			pos->board[A1] = '0';
		}
	}

	if (piece == 'k') { // black king
		pos->BcastleQS = 0;
		pos->BcastleKS = 0;
		pos->Bkingpos = move->to;
		if ((move->from == E8) && (move->to == G8)) { // black kingside castling
			pos->board[E8] = '0';
			pos->board[G8] = 'k';
			pos->board[F8] = 'r';
			pos->board[H8] = '0';
		}
		if ((move->from == E8) && (move->to == C8)) { // black queenside castling
			pos->board[E8] = '0';
			pos->board[C8] = 'k';
			pos->board[D8] = 'r';
			pos->board[A8] = '0';
		}
	}
	if (pos->board[0] != 'r') { // black a8 rook moved or captured
		pos->BcastleQS = 0;
	}
	if (pos->board[7] != 'r') { // black h8 rook moved or captured
		pos->BcastleKS = 0;
	}
	if (pos->board[56] != 'R') { // white a1 rook moved or captured
		pos->WcastleQS = 0;
	}
	if (pos->board[63] != 'R') { // white h1 rook moved or captured
		pos->WcastleKS = 0;
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
