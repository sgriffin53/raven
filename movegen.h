#ifndef MOVEGEN_H
#define MOVEGEN_H

int Kdirs[8][2] = {{-1,-1},{-1,+1},{+1,-1},{+1,+1},{0,1},{0,-1},{-1,0},{1,0}};
int Ndirs[8][2] = {{-2,-1},{-1,-2},{+1,-2},{+2,-1},{+2,+1},{+1,+2},{-1,+2},{-2,+1}};
int Bdirs[4][2] = {{-1,-1},{-1,+1},{+1,-1},{+1,+1}};
int Rdirs[4][2] = {{0,1},{0,-1},{-1,0},{1,0}};
int Qdirs[8][2] = {{-1,-1},{-1,+1},{+1,-1},{+1,+1},{0,1},{0,-1},{-1,0},{1,0}};

int genLegalPawnMoves(struct position *pos, int square, struct move *Pmoves) {
	assert(pos);
	assert(square >= 0 && square <= 63);
	assert(Pmoves);
	int newfile, newrank;
	int newsquare;
	char cappiece;
	int pmovesend = 0;
	
	int file = getfile(square);
	int rank = getrank(square);
	int startsquare = fileranktosquareidx(file,rank);
	
	if ((pos->tomove == WHITE)) {
		// one square forward
		newfile = file;
		newrank = rank - 1;
		newsquare = fileranktosquareidx(newfile,newrank);
		while (1) {
			if ((newfile < 0) || (newfile > 7) || (newrank < 0) || (newrank > 7)) {
				break;
			}
			if (rank == 1) {
				//move is promotion, handled later
				break;
			}
			if (pos->board[newsquare] != '0') {
				break;
			}
			Pmoves[pmovesend].from = startsquare;
			Pmoves[pmovesend].to = newsquare;
			Pmoves[pmovesend].prom = 0;
			pmovesend += 1;
			break;
		}
		// two squares forward
		if ((rank == 6) && (pos->tomove == WHITE)) {
			newfile = file;
			newrank = rank - 1;
			newsquare = fileranktosquareidx(newfile,newrank);
			while (1) {
				if ((newfile < 0) || (newfile > 7) || (newrank < 0) || (newrank > 7)) {
					break;
				}
				//check if square one forward is blocked
				if (pos->board[newsquare] != '0') {
					break;
				}
				// check if square two forward is blocked
				newfile = file;
				newrank = rank - 2;
				newsquare = fileranktosquareidx(newfile,newrank);
				if ((newfile < 0) || (newfile > 7) || (newrank < 0) || (newrank > 7)) {
					break;
				}
				if (pos->board[newsquare] != '0') {
					break;
				}
				Pmoves[pmovesend].from = startsquare;
				Pmoves[pmovesend].to = newsquare;
				Pmoves[pmovesend].prom = 0;
				pmovesend += 1;
				break;
			}
		}
		
		// captures
		newfile = file - 1;
		newrank = rank - 1;
		newsquare = fileranktosquareidx(newfile,newrank);
		while (1) {
			if ((newfile < 0) || (newfile > 7) || (newrank < 0) || (newrank > 7)) {
				break;
			}
			if (rank == 1) {
				//move is promotion, handled later
				break;
			}
			cappiece = pos->board[newsquare];
			if (! isBlackPiece(cappiece) ) {
				// square is not black piece
				break;
			}
			Pmoves[pmovesend].from = startsquare;
			Pmoves[pmovesend].to = newsquare;
			Pmoves[pmovesend].prom = 0;
			pmovesend += 1;
			break;
		}
		newfile = file + 1;
		newrank = rank - 1;
		newsquare = fileranktosquareidx(newfile,newrank);
		while (1) {
			if ((newfile < 0) || (newfile > 7) || (newrank < 0) || (newrank > 7)) {
				break;
			}
			if (rank == 1) {
				//move is promotion, handled later
				break;
			}
			cappiece = pos->board[newsquare];
			if (! isBlackPiece(cappiece) ) {
				// square is not black piece
				break;
			}
			Pmoves[pmovesend].from = startsquare;
			Pmoves[pmovesend].to = newsquare;
			Pmoves[pmovesend].prom = 0;
			pmovesend += 1;
			break;
		}
		
		// en passant
		newfile = file - 1;
		newrank = rank - 1;
		newsquare = fileranktosquareidx(newfile,newrank);
		while (1) {
			if ((newfile < 0) || (newfile > 7) || (newrank < 0) || (newrank > 7)) {
				break;
			}
			if (newsquare == pos->epsquare) {
				Pmoves[pmovesend].from = startsquare;
				Pmoves[pmovesend].to = newsquare;
				Pmoves[pmovesend].prom = 0;
				pmovesend += 1;
			}
			break;
		}
		
		newfile = file + 1;
		newrank = rank - 1;
		newsquare = fileranktosquareidx(newfile,newrank);
		while (1) {
			if ((newfile < 0) || (newfile > 7) || (newrank < 0) || (newrank > 7)) {
				break;
			}
			if (newsquare == pos->epsquare) {
				Pmoves[pmovesend].from = startsquare;
				Pmoves[pmovesend].to = newsquare;
				Pmoves[pmovesend].prom = 0;
				pmovesend += 1;
			}
			break;
		}
		
		// promotion
		if ((rank == 1)) {
			// capture up and left
			newfile = file - 1;
			newrank = rank - 1;
			newsquare = fileranktosquareidx(newfile,newrank);
			cappiece = pos->board[newsquare];
			while (1) {
				if ((newfile < 0) || (newfile > 7) || (newrank < 0) || (newrank > 7)) {
					break;
				}
				if (! isBlackPiece(cappiece)) {
					// piece is not black piece
					break;
				}
				Pmoves[pmovesend].from = startsquare;
				Pmoves[pmovesend].to = newsquare;
				Pmoves[pmovesend].prom = 'b';
				pmovesend += 1;
				Pmoves[pmovesend].from = startsquare;
				Pmoves[pmovesend].to = newsquare;
				Pmoves[pmovesend].prom = 'n';
				pmovesend += 1;
				Pmoves[pmovesend].from = startsquare;
				Pmoves[pmovesend].to = newsquare;
				Pmoves[pmovesend].prom = 'r';
				pmovesend += 1;
				Pmoves[pmovesend].from = startsquare;
				Pmoves[pmovesend].to = newsquare;
				Pmoves[pmovesend].prom = 'q';
				pmovesend += 1;
				break;
			}
			
			// capture up and right
			newfile = file + 1;
			newrank = rank - 1;
			newsquare = fileranktosquareidx(newfile,newrank);
			cappiece = pos->board[newsquare];
			while (1) {
				if ((newfile < 0) || (newfile > 7) || (newrank < 0) || (newrank > 7)) {
					break;
				}
				if (! isBlackPiece(cappiece)) {
					// piece is not black piece
					break;
				}
				Pmoves[pmovesend].from = startsquare;
				Pmoves[pmovesend].to = newsquare;
				Pmoves[pmovesend].prom = 'b';
				pmovesend += 1;
				Pmoves[pmovesend].from = startsquare;
				Pmoves[pmovesend].to = newsquare;
				Pmoves[pmovesend].prom = 'n';
				pmovesend += 1;
				Pmoves[pmovesend].from = startsquare;
				Pmoves[pmovesend].to = newsquare;
				Pmoves[pmovesend].prom = 'r';
				pmovesend += 1;
				Pmoves[pmovesend].from = startsquare;
				Pmoves[pmovesend].to = newsquare;
				Pmoves[pmovesend].prom = 'q';
				pmovesend += 1;
				break;
			}
			
			// move forward one square
			newfile = file;
			newrank = rank - 1;
			newsquare = fileranktosquareidx(newfile,newrank);
			cappiece = pos->board[newsquare];
			while (1) {
				if ((newfile < 0) || (newfile > 7) || (newrank < 0) || (newrank > 7)) {
					break;
				}
				if (cappiece != '0' ) {
					// promotion square is not empty
					break;
				}
				Pmoves[pmovesend].from = startsquare;
				Pmoves[pmovesend].to = newsquare;
				Pmoves[pmovesend].prom = 'b';
				pmovesend += 1;
				Pmoves[pmovesend].from = startsquare;
				Pmoves[pmovesend].to = newsquare;
				Pmoves[pmovesend].prom = 'n';
				pmovesend += 1;
				Pmoves[pmovesend].from = startsquare;
				Pmoves[pmovesend].to = newsquare;
				Pmoves[pmovesend].prom = 'r';
				pmovesend += 1;
				Pmoves[pmovesend].from = startsquare;
				Pmoves[pmovesend].to = newsquare;
				Pmoves[pmovesend].prom = 'q';
				pmovesend += 1;
				break;
			}
		}
	}
	
	if ((pos->tomove == BLACK)) {
		// one square forward
		newfile = file;
		newrank = rank + 1;
		newsquare = fileranktosquareidx(newfile,newrank);
		while (1) {
			if ((newfile < 0) || (newfile > 7) || (newrank < 0) || (newrank > 7)) {
				break;
			}
			if (rank == 6) {
				//move is promotion, handled later
				break;
			}
			if (pos->board[newsquare] != '0') {
				break;
			}
			Pmoves[pmovesend].from = startsquare;
			Pmoves[pmovesend].to = newsquare;
			Pmoves[pmovesend].prom = 0;
			pmovesend += 1;
			break;
		}
		
		// two squares forward
		if ((rank == 1) && (pos->tomove == BLACK)) {
			//check if square one forward is blocked
			newfile = file;
			newrank = rank + 1;
			newsquare = fileranktosquareidx(newfile,newrank);
			while (1) {
				if ((newfile < 0) || (newfile > 7) || (newrank < 0) || (newrank > 7)) {
					break;
				}
				if (pos->board[newsquare] != '0') {
					break;
				}
				// check if square two forward is blocked
				newfile = file;
				newrank = rank + 2;
				newsquare = fileranktosquareidx(newfile,newrank);
				if ((newfile < 0) || (newfile > 7) || (newrank < 0) || (newrank > 7)) {
					break;
				}
				if (pos->board[newsquare] != '0') {
					break;
				}
				// add pawn move
				Pmoves[pmovesend].from = startsquare;
				Pmoves[pmovesend].to = newsquare;
				Pmoves[pmovesend].prom = 0;
				pmovesend += 1;
				break;
			}
			
		}
		// captures
		newfile = file - 1;
		newrank = rank + 1;
		newsquare = fileranktosquareidx(newfile,newrank);
		while (1) {
			if ((newfile < 0) || (newfile > 7) || (newrank < 0) || (newrank > 7)) {
				break;
			}
			if (rank == 6) {
				//move is promotion, handled later
				break;
			}
			cappiece = pos->board[newsquare];
			if (! isWhitePiece(cappiece) ) {
				// square is not white piece
				break;
			}
			Pmoves[pmovesend].from = startsquare;
			Pmoves[pmovesend].to = newsquare;
			Pmoves[pmovesend].prom = 0;
			pmovesend += 1;
			break;
		}
		
		newfile = file + 1;
		newrank = rank + 1;
		newsquare = fileranktosquareidx(newfile,newrank);
		while (1) {
			if ((newfile < 0) || (newfile > 7) || (newrank < 0) || (newrank > 7)) {
				break;
			}
			if (rank == 6) {
				//move is promotion, handled later
				break;
			}
			cappiece = pos->board[newsquare];
			if (! isWhitePiece(cappiece) ) {
				// square is not white piece
				break;
			}
			Pmoves[pmovesend].from = startsquare;
			Pmoves[pmovesend].to = newsquare;
			Pmoves[pmovesend].prom = 0;
			pmovesend += 1;
			break;
		}
		
		// en passant
		newfile = file - 1;
		newrank = rank + 1;
		newsquare = fileranktosquareidx(newfile,newrank);
		while (1) {
			if ((newfile < 0) || (newfile > 7) || (newrank < 0) || (newrank > 7)) {
				break;
			}
		
			if (newsquare == pos->epsquare) {
				Pmoves[pmovesend].from = startsquare;
				Pmoves[pmovesend].to = newsquare;
				Pmoves[pmovesend].prom = 0;
				pmovesend += 1;
			}
			break;
		
		}
		
		newfile = file + 1;
		newrank = rank + 1;
		newsquare = fileranktosquareidx(newfile,newrank);
		while (1) {
			if ((newfile < 0) || (newfile > 7) || (newrank < 0) || (newrank > 7)) {
				break;
			}
			if (newsquare == pos->epsquare) {
				Pmoves[pmovesend].from = startsquare;
				Pmoves[pmovesend].to = newsquare;
				Pmoves[pmovesend].prom = 0;
				pmovesend += 1;
			}
			break;
		}

		// promotion
		if ((rank == 6)) {
			// capture down and left
			newfile = file - 1;
			newrank = rank + 1;
			newsquare = fileranktosquareidx(newfile,newrank);
			cappiece = pos->board[newsquare];
			while (1) {
				if ((newfile < 0) || (newfile > 7) || (newrank < 0) || (newrank > 7)) {
					break;
				}
				if (! isWhitePiece(cappiece) ) {
					// piece is not white piece
					break;
				}
				Pmoves[pmovesend].from = startsquare;
				Pmoves[pmovesend].to = newsquare;
				Pmoves[pmovesend].prom = 'b';
				pmovesend += 1;
				Pmoves[pmovesend].from = startsquare;
				Pmoves[pmovesend].to = newsquare;
				Pmoves[pmovesend].prom = 'n';
				pmovesend += 1;
				Pmoves[pmovesend].from = startsquare;
				Pmoves[pmovesend].to = newsquare;
				Pmoves[pmovesend].prom = 'r';
				pmovesend += 1;
				Pmoves[pmovesend].from = startsquare;
				Pmoves[pmovesend].to = newsquare;
				Pmoves[pmovesend].prom = 'q';
				pmovesend += 1;
				break;
			}
			
			// capture down and right
			newfile = file + 1;
			newrank = rank + 1;
			newsquare = fileranktosquareidx(newfile,newrank);
			cappiece = pos->board[newsquare];
			while (1) {
				if ((newfile < 0) || (newfile > 7) || (newrank < 0) || (newrank > 7)) {
					break;
				}
				if (! isWhitePiece(cappiece) ) {
					// piece is not white piece
					break;
				}
				Pmoves[pmovesend].from = startsquare;
				Pmoves[pmovesend].to = newsquare;
				Pmoves[pmovesend].prom = 'b';
				pmovesend += 1;
				Pmoves[pmovesend].from = startsquare;
				Pmoves[pmovesend].to = newsquare;
				Pmoves[pmovesend].prom = 'n';
				pmovesend += 1;
				Pmoves[pmovesend].from = startsquare;
				Pmoves[pmovesend].to = newsquare;
				Pmoves[pmovesend].prom = 'r';
				pmovesend += 1;
				Pmoves[pmovesend].from = startsquare;
				Pmoves[pmovesend].to = newsquare;
				Pmoves[pmovesend].prom = 'q';
				pmovesend += 1;
				break;
			}
			
			
			// move forward one square
			newfile = file;
			newrank = rank + 1;
			newsquare = fileranktosquareidx(newfile,newrank);
			cappiece = pos->board[newsquare];
			while (1) {
				if ((newfile < 0) || (newfile > 7) || (newrank < 0) || (newrank > 7)) {
					break;
				}
				if (cappiece != '0' ) {
					// promotion square is not empty
					break;
				}
				Pmoves[pmovesend].from = startsquare;
				Pmoves[pmovesend].to = newsquare;
				Pmoves[pmovesend].prom = 'b';
				pmovesend += 1;
				Pmoves[pmovesend].from = startsquare;
				Pmoves[pmovesend].to = newsquare;
				Pmoves[pmovesend].prom = 'n';
				pmovesend += 1;
				Pmoves[pmovesend].from = startsquare;
				Pmoves[pmovesend].to = newsquare;
				Pmoves[pmovesend].prom = 'r';
				pmovesend += 1;
				Pmoves[pmovesend].from = startsquare;
				Pmoves[pmovesend].to = newsquare;
				Pmoves[pmovesend].prom = 'q';
				pmovesend += 1;
				break;
			}
		
		}
	}
	return pmovesend;		
}
int genLegalKnightMoves(struct position *pos, int square, struct move *Nmoves) {
	assert(pos);
	assert(square >= 0 && square <= 63);
	assert(Nmoves);
	int newfile, newrank;
	int newsquare;
	char cappiece;
	int nmovesend = 0;
	int i;
	int file = getfile(square);
	int rank = getrank(square);
	int startsquare = fileranktosquareidx(file,rank);
	char piece = pos->board[startsquare];
	
	if ((pos->tomove == WHITE) && (piece == 'N')) {
		for (i = 0;i < 8;i++) {
			newfile = file + Ndirs[i][0];
			newrank = rank + Ndirs[i][1];
			newsquare = fileranktosquareidx(newfile,newrank);
			cappiece = pos->board[newsquare];
			if ((newfile < 0) || (newfile > 7) || (newrank < 0) || (newrank > 7)) {
				continue;
			}
			if (isWhitePiece(cappiece)) {
				// piece is white piece
				continue;
			}
			Nmoves[nmovesend].from = startsquare;
			Nmoves[nmovesend].to = newsquare;
			Nmoves[nmovesend].prom = 0;
			nmovesend += 1;
		}
	}
	
	if ((pos->tomove == BLACK) && (piece == 'n')) {
		for (i = 0;i < 8;i++) {
			newfile = file + Ndirs[i][0];
			newrank = rank + Ndirs[i][1];
			newsquare = fileranktosquareidx(newfile,newrank);
			cappiece = pos->board[newsquare];
			if ((newfile < 0) || (newfile > 7) || (newrank < 0) || (newrank > 7)) {
				continue;
			}
			if (isBlackPiece(cappiece)) {
				// piece is black piece
				continue;
			}
			Nmoves[nmovesend].from = startsquare;
			Nmoves[nmovesend].to = newsquare;
			Nmoves[nmovesend].prom = 0;
			nmovesend += 1;
		}
	}
	
	return nmovesend;
}
int genLegalBishopMoves(struct position *pos, int square, struct move *Bmoves) {
	assert(pos);
	assert(square >= 0 && square <= 63);
	assert(Bmoves);
	int newfile, newrank;
	int outofbounds;
	int newsquare;
	int bmovesend = 0;
	int i;
	int file = getfile(square);
	int rank = getrank(square);
	int startsquare = fileranktosquareidx(file,rank);
	char piece = pos->board[startsquare];
	
	// gen bishop moves
	for (i = 0;i < 4;i++) {
		outofbounds = 0;
		newfile = file;
		newrank = rank;
		
		while (outofbounds == 0) {
			newfile = newfile + Bdirs[i][0];
			newrank = newrank + Bdirs[i][1];
			newsquare = fileranktosquareidx(newfile,newrank);
			if ((piece == 'b') && (pos->tomove == WHITE)) {
				outofbounds = 1;
				break;
			}
			if ((piece == 'B') && (pos->tomove == BLACK)) {
				outofbounds = 1;
				break;
			}
			if ((newfile < 0) || (newfile > 7) || (newrank < 0) || (newrank > 7)) {
				outofbounds = 1;
				break;
			}
			
			if (pos->tomove == WHITE) {
				if (isWhitePiece(pos->board[newsquare])) {
					// char is uppercase so is white piece
					// line of sight is blocked by friendly piece so not check
					outofbounds = 1;
					break;
				}
				if (isBlackPiece(pos->board[newsquare])) {
					// char is lowercase so is black piece
					// bishop captures enemy piece
					Bmoves[bmovesend].from = startsquare;
					Bmoves[bmovesend].to = newsquare;
					Bmoves[bmovesend].prom = 0;
					bmovesend += 1;
					outofbounds = 1;
					break;
				}
				Bmoves[bmovesend].from = startsquare;
				Bmoves[bmovesend].to = newsquare;
				Bmoves[bmovesend].prom = 0;
				bmovesend += 1;
			}
			
			if (pos->tomove == BLACK) {
				if (isBlackPiece(pos->board[newsquare])) {
					// char is lowercase so is black piece
					// line of sight is blocked by friendly piece so not check
					outofbounds = 1;
					break;
				}
				if (isWhitePiece(pos->board[newsquare])) {
					// char is uppercase so white piece
					// bishop captures white piece
					Bmoves[bmovesend].from = startsquare;
					Bmoves[bmovesend].to = newsquare;
					Bmoves[bmovesend].prom = 0;
					bmovesend += 1;
					outofbounds = 1;
					break;
				}
				Bmoves[bmovesend].from = startsquare;
				Bmoves[bmovesend].to = newsquare;
				Bmoves[bmovesend].prom = 0;
				bmovesend += 1;
			}
			
		}
		
	}
	return bmovesend;
}
int genLegalRookMoves(struct position *pos, int square, struct move *Rmoves) {
	assert(pos);
	assert(square >= 0 && square <= 63);
	assert(Rmoves);
	int newfile, newrank;
	int outofbounds;
	int newsquare;
	int rmovesend = 0;
	int i;
	int file = getfile(square);
	int rank = getrank(square);
	int startsquare = fileranktosquareidx(file,rank);
	char piece = pos->board[startsquare];
	for (i = 0;i < 4;i++) {
		outofbounds = 0;
		file = getfile(square);
		rank = getrank(square);
		newfile = file;
		newrank = rank;
		
		while (outofbounds == 0) {
			newfile = newfile + Rdirs[i][0];
			newrank = newrank + Rdirs[i][1];
			newsquare = fileranktosquareidx(newfile,newrank);
			if ((piece == 'r') && (pos->tomove == WHITE)) {
				outofbounds = 1;
				break;
			}
			if ((piece == 'R') && (pos->tomove == BLACK)) {
				outofbounds = 1;
				break;
			}
			if ((newfile < 0) || (newfile > 7) || (newrank < 0) || (newrank > 7)) {
				outofbounds = 1;
				break;
			}
			
			if (pos->tomove == WHITE) {
				if (isWhitePiece(pos->board[newsquare])) {
					// char is uppercase so is white piece
					// line of sight is blocked by friendly piece so not check
					outofbounds = 1;
					break;
				}
				if (isBlackPiece(pos->board[newsquare])) {
					//char is lowercase so black piece
					//rook captures black piece
					Rmoves[rmovesend].from = startsquare;
					Rmoves[rmovesend].to = newsquare;
					Rmoves[rmovesend].prom = 0;
					rmovesend += 1;
					outofbounds = 1;
					break;
				}
				Rmoves[rmovesend].from = startsquare;
				Rmoves[rmovesend].to = newsquare;
				Rmoves[rmovesend].prom = 0;
				rmovesend += 1;
			}
			
			if (pos->tomove == BLACK) {
				if (isBlackPiece(pos->board[newsquare])) {
					// char is lowercase so is black piece
					// line of sight is blocked by friendly piece so not check
					outofbounds = 1;
					break;
				}
				if (isWhitePiece(pos->board[newsquare])) {
					//char is uppercase so white piece
					//rook captures white piece
					Rmoves[rmovesend].from = startsquare;
					Rmoves[rmovesend].to = newsquare;
					Rmoves[rmovesend].prom = 0;
					rmovesend += 1;
					outofbounds = 1;
					break;
				}
				Rmoves[rmovesend].from = startsquare;
				Rmoves[rmovesend].to = newsquare;
				Rmoves[rmovesend].prom = 0;
				rmovesend += 1;
			}
		}
	}
	return rmovesend;
}

int genLegalQueenMoves(struct position *pos, int square, struct move *Qmoves) {
	assert(pos);
	assert(square >= 0 && square <= 63);
	assert(Qmoves);
	int newfile, newrank;
	int outofbounds;
	int newsquare;
	int qmovesend = 0;
	int i;
	int file = getfile(square);
	int rank = getrank(square);
	int startsquare = fileranktosquareidx(file,rank);
	char piece = pos->board[startsquare];
	
	for (i = 0;i < 8;i++) {
		outofbounds = 0;
		file = getfile(square);
		rank = getrank(square);
		newfile = file;
		newrank = rank;
		
		while (outofbounds == 0) {
			newfile = newfile + Qdirs[i][0];
			newrank = newrank + Qdirs[i][1];
			newsquare = fileranktosquareidx(newfile,newrank);
			if ((piece == 'q') && (pos->tomove == WHITE)) {
				outofbounds = 1;
				break;
			}
			if ((piece == 'Q') && (pos->tomove == BLACK)) {
				outofbounds = 1;
				break;
			}
			if ((newfile < 0) || (newfile > 7) || (newrank < 0) || (newrank > 7)) {
				outofbounds = 1;
				break;
			}
			
			if (pos->tomove == WHITE) {
				if (isWhitePiece(pos->board[newsquare])) {
					// char is uppercase so is white piece
					// line of sight is blocked by friendly piece so not check
					outofbounds = 1;
					break;
				}
				if (isBlackPiece(pos->board[newsquare])) {
					//char is lowercase so black piece
					//rook captures black piece
					Qmoves[qmovesend].from = startsquare;
					Qmoves[qmovesend].to = newsquare;
					Qmoves[qmovesend].prom = 0;
					qmovesend += 1;
					outofbounds = 1;
					break;
				}
				Qmoves[qmovesend].from = startsquare;
				Qmoves[qmovesend].to = newsquare;
				Qmoves[qmovesend].prom = 0;
				qmovesend += 1;
			}
			
			if (pos->tomove == BLACK) {
				if (isBlackPiece(pos->board[newsquare])) {
					// char is lowercase so is black piece
					// line of sight is blocked by friendly piece so not check
					outofbounds = 1;
					break;
				}
				if (isWhitePiece(pos->board[newsquare])) {
					//char is uppercase so white piece
					//rook captures white piece
					Qmoves[qmovesend].from = startsquare;
					Qmoves[qmovesend].to = newsquare;
					Qmoves[qmovesend].prom = 0;
					qmovesend += 1;
					outofbounds = 1;
					break;
				}
				Qmoves[qmovesend].from = startsquare;
				Qmoves[qmovesend].to = newsquare;
				Qmoves[qmovesend].prom = 0;
				qmovesend += 1;
			}
			
		}
	}
	return qmovesend;
}
int genLegalKingMoves(struct position *pos, int square, struct move *Kmoves) {
	assert(pos);
	assert(square >= 0 && square <= 63);
	assert(Kmoves);
	int newfile, newrank;
	int outofbounds;
	int newsquare;
	int kmovesend = 0;
	int i;
	int file = getfile(square);
	int rank = getrank(square);
	int startsquare = fileranktosquareidx(file,rank);
	char piece = pos->board[startsquare];
	
	if ((pos->tomove == WHITE) && (piece == 'K')) {
		for (i = 0;i < 8;i++) {
			outofbounds = 0;
			newfile = file + Kdirs[i][0];
			newrank = rank + Kdirs[i][1];
			newsquare = fileranktosquareidx(newfile,newrank);
			char cappiece = pos->board[newsquare];
			if ((newfile < 0) || (newfile > 7) || (newrank < 0) || (newrank > 7)) {
				continue;
				outofbounds = 1;
			}
			if (isWhitePiece(cappiece)) {
				// piece is white piece
				continue;
				outofbounds = 1;
			}
			
			if (outofbounds == 0) {
				Kmoves[kmovesend].from = startsquare;
				Kmoves[kmovesend].to = newsquare;
				Kmoves[kmovesend].prom = 0;
				kmovesend += 1;
			}
			
		}
		// White castling QS
		outofbounds = 0;
		if ((pos->board[B1] != '0') || (pos->board[C1] != '0') || (pos->board[D1] != '0')) {
			outofbounds = 1;
		}
		//if (isCheck(pos,57) == 1) outofbounds = 1;
		if (isCheck(pos,C1) == 1) outofbounds = 1;
		if (isCheck(pos,D1) == 1) outofbounds = 1;
		if (isCheck(pos,E1) == 1) outofbounds = 1;
		if (pos->WcastleQS == 0) outofbounds = 1;
		
		if (outofbounds == 0) {
			Kmoves[kmovesend].from = E1;
			Kmoves[kmovesend].to = C1;
			Kmoves[kmovesend].prom = 0;
			kmovesend += 1;
		}
		
		// White castling KS
		outofbounds = 0;
		if ((pos->board[F1] != '0') || (pos->board[G1] != '0')) {
			outofbounds = 1;
		}
		if (isCheck(pos,F1) == 1) outofbounds = 1;
		if (isCheck(pos,G1) == 1) outofbounds = 1;
		if (isCheck(pos,E1) == 1) outofbounds = 1;
		if (pos->WcastleKS == 0) outofbounds = 1;
		if (outofbounds == 0) {
			Kmoves[kmovesend].from = E1;
			Kmoves[kmovesend].to = G1;
			Kmoves[kmovesend].prom = 0;
			kmovesend += 1;
		}
	}
	
	if ((pos->tomove == BLACK) && (piece == 'k')) {
		for (i = 0;i < 8;i++) {
			outofbounds = 0;
			newfile = file + Kdirs[i][0];
			newrank = rank + Kdirs[i][1];
			newsquare = fileranktosquareidx(newfile,newrank);
			char cappiece = pos->board[newsquare];
			if ((newfile < 0) || (newfile > 7) || (newrank < 0) || (newrank > 7)) {
				continue;
				outofbounds = 1;
			}
			if (isBlackPiece(cappiece)) {
				// piece is black piece
				continue;
				outofbounds = 1;
			}
			
			if (outofbounds == 0) {
				Kmoves[kmovesend].from = startsquare;
				Kmoves[kmovesend].to = newsquare;
				Kmoves[kmovesend].prom = 0;
				kmovesend += 1;
			}
			
		}
		// Black castling QS
		outofbounds = 0;
		if ((pos->board[B8] != '0') || (pos->board[C8] != '0') || (pos->board[D8] != '0')) {
			outofbounds = 1;
		}
		//if (isCheck(pos,1) == 1) outofbounds = 1;
		if (isCheck(pos,C8) == 1) outofbounds = 1;
		if (isCheck(pos,D8) == 1) outofbounds = 1;
		if (isCheck(pos,E8) == 1) outofbounds = 1;
		if (pos->BcastleQS == 0) outofbounds = 1;
		if (outofbounds == 0) {
			Kmoves[kmovesend].from = E8;
			Kmoves[kmovesend].to = C8;
			Kmoves[kmovesend].prom = 0;
			kmovesend += 1;
		}
		// Black castling KS
		outofbounds = 0;
		if ((pos->board[F8] != '0') || (pos->board[G8] != '0')) {
			outofbounds = 1;
		}
		if (isCheck(pos,E8) == 1) outofbounds = 1;
		if (isCheck(pos,F8) == 1) outofbounds = 1;
		if (isCheck(pos,G8) == 1) outofbounds = 1;
		if (pos->BcastleKS == 0) outofbounds = 1;
		if (outofbounds == 0) {
			Kmoves[kmovesend].from = E8;
			Kmoves[kmovesend].to = G8;
			Kmoves[kmovesend].prom = 0;
			kmovesend += 1;
		}
	}
	return kmovesend;
}
int genLegalMoves(struct position *pos, struct move *moves) {
	assert(pos);
	assert(moves);
	int num_moves = 0;
	
	for (int i = 0;i < 64;i++) {
		char piece = pos->board[i]; 
		if ( ((piece == 'P') && (pos->tomove == WHITE)) || ((piece == 'p') && (pos->tomove == BLACK)) ) {
			num_moves += genLegalPawnMoves(pos,i,moves + num_moves);
		}
		else if ((piece == 'N') || (piece == 'n')) {
			num_moves += genLegalKnightMoves(pos,i,moves + num_moves);
		}
		else if ((piece == 'B') || (piece == 'b')) {
			num_moves += genLegalBishopMoves(pos,i,moves + num_moves);
		}
		else if ((piece == 'R') || (piece == 'r')) {
			num_moves += genLegalRookMoves(pos,i,moves + num_moves);
		}
		else if ((piece == 'Q') || (piece == 'q')) {
			num_moves += genLegalQueenMoves(pos,i,moves + num_moves);
		}
		else if ((piece == 'K') || (piece == 'k')) {
			num_moves += genLegalKingMoves(pos,i,moves + num_moves);
		}
	}
	
	// remove moves which result in check
	/*
	int kingpos;
	int a = 0;
	int newmovesend = 0;
	for (int i = 0;i < num_moves;i++) {
		makeMove(&moves[i],pos);
		pos->tomove = !pos->tomove;
		if (pos->tomove == WHITE) kingpos = pos->Wkingpos;
		if (pos->tomove == BLACK) kingpos = pos->Bkingpos;
		int incheck = isCheck(pos,kingpos);
		if (!incheck) {
			newmoves[a] = moves[i];
			newmovesend++;
			a++;
		}
		pos->tomove = !pos->tomove;
		unmakeMove(pos);
	}
	movesend = 0;
	//moves = newmoves;
	for (int i = 0;i < newmovesend;i++) {
		moves[i] = newmoves[i];
		movesend++;
	}
	 */
	 
	return num_moves;
}
#endif