
void makeMove(struct move *move, struct position *pos) {
	char piece = pos->board[move->from];
	int newepsquare = -1; // init to -1, will be changed to en passant square if there is one
	int newWkingpos = pos->Wkingpos;
	int newBkingpos = pos->Bkingpos;
	int newWcastleQS = pos->WcastleQS;
	int newWcastleKS = pos->WcastleKS;
	int newBcastleQS = pos->BcastleQS;
	int newBcastleKS = pos->BcastleKS;
	int torank = getrank(move->to);
	char cappiece = pos->board[move->to];
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
		newWcastleQS = 0;
		newWcastleKS = 0;
		newWkingpos = move->to;
		if ((move->from == 60) && (move->to == 62)) { // white kingside castling
			pos->board[60] = '0';
			pos->board[62] = 'K';
			pos->board[61] = 'R';
			pos->board[63] = '0';
		}
		if ((move->from == 60) && (move->to == 58)) { // white queenside castling
			pos->board[60] = '0';
			pos->board[58] = 'K';
			pos->board[59] = 'R';
			pos->board[56] = '0';
		}
	}
	if (piece == 'k') { // black king
		newBcastleQS = 0;
		newBcastleKS = 0;
		newBkingpos = move->to;
		if ((move->from == 4) && (move->to == 6)) { // white kingside castling
			pos->board[4] = '0';
			pos->board[6] = 'k';
			pos->board[5] = 'r';
			pos->board[7] = '0';
		}
		if ((move->from == 4) && (move->to == 2)) { // white queenside castling
			pos->board[4] = '0';
			pos->board[2] = 'k';
			pos->board[3] = 'r';
			pos->board[0] = '0';
		}
	}
	if (pos->board[0] != 'r') { // black a8 rook moved or captured
		newBcastleQS = 0;
	}
	if (pos->board[7] != 'r') { // black h8 rook moved or captured
		newBcastleKS = 0;
	}
	if (pos->board[56] != 'R') { // white a1 rook moved or captured
		newWcastleQS = 0;
	}
	if (pos->board[63] != 'R') { // white h1 rook moved or captured
		newWcastleKS = 0;
	}
	pos->tomove = !pos->tomove;
	pos->epsquare = newepsquare;
	pos->Bkingpos = newBkingpos;
	pos->Wkingpos = newWkingpos;
	pos->WcastleQS = newWcastleQS;
	pos->WcastleKS = newWcastleKS;
	pos->BcastleQS = newBcastleQS;
	pos->BcastleKS = newBcastleKS;
	posstack[posstackend] = *pos;
	posstackend++;
}
void unmakeMove(struct position *pos) {
	posstackend--;
	if (posstackend <= 0) {
		posstackend = 1;
		*pos = posstack[0];
	}
	else {
	*pos = posstack[posstackend - 1];
	struct position blankpos = {.epsquare=0,.board={},.WcastleQS=0,.WcastleKS=0,.BcastleQS=0,.BcastleKS=0,
				.tomove=0,.Wkingpos=0,.Bkingpos=0,.halfmoves=0};;
	posstack[posstackend] = blankpos;
	}
}
void makeMovestr(char move[], struct position *pos) {
	int startsquareidx;
	int endsquareidx;
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
	startsquareidx = strsquaretoidx(startsquare);
	endsquareidx = strsquaretoidx(endsquare);
	struct move moveobj = {.from=startsquareidx,.to=endsquareidx,.prom=prompiece[0]};
	makeMove(&moveobj, pos);
}