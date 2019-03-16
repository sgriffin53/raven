int genLegalPawnMoves(struct position *pos, int square, struct move *Pmoves) {
	int file, rank;
	int newfile, newrank;
	int oneforward;
	int outofbounds;
	int startsquare;
	int newsquare;
	char cappiece;
	int pmovesend = 0;
	char piece;
	file = getfile(square);
	rank = getrank(square);
	startsquare = fileranktosquareidx(file,rank);
	piece = pos->board[startsquare];
	if ((pos->tomove == WHITE) && (piece == 'P')) {
		// one square forward
		outofbounds = 0;
		newfile = file;
		newrank = rank - 1;
		newsquare = fileranktosquareidx(newfile,newrank);
		if ((newfile < 0) || (newfile > 7) || (newrank < 0) || (newrank > 7)) {
			outofbounds = 1;
		}
		if (rank == 1) {
			//move is promotion, handled later
			outofbounds = 1;
		}
		if (pos->board[newsquare] != '0') outofbounds = 1;
		if (outofbounds == 0) {
			Pmoves[pmovesend].from = startsquare;
			Pmoves[pmovesend].to = newsquare;
			Pmoves[pmovesend].prom = 0;
			pmovesend += 1;
		}
		// two squares forward
		if ((rank == 6) && (pos->tomove == WHITE)) {
			newfile = file;
			newrank = rank - 1;
			newsquare = fileranktosquareidx(newfile,newrank);
			if ((newfile < 0) || (newfile > 7) || (newrank < 0) || (newrank > 7)) {
				outofbounds = 1;
			}
			//check if square one forward is blocked
			if (pos->board[newsquare] != '0') outofbounds = 1;
			// check if square two forward is blocked
			newfile = file;
			newrank = rank - 2;
			newsquare = fileranktosquareidx(newfile,newrank);
			if ((newfile < 0) || (newfile > 7) || (newrank < 0) || (newrank > 7)) {
				outofbounds = 1;
			}
			if (pos->board[newsquare] != '0') outofbounds = 1;
			if (outofbounds == 0) {
				// add pawn move
				Pmoves[pmovesend].from = startsquare;
				Pmoves[pmovesend].to = newsquare;
				Pmoves[pmovesend].prom = 0;
				pmovesend += 1;
			}
		}
		// captures
		outofbounds = 0;
		newfile = file - 1;
		newrank = rank - 1;
		newsquare = fileranktosquareidx(newfile,newrank);
		if ((newfile < 0) || (newfile > 7) || (newrank < 0) || (newrank > 7)) {
			outofbounds = 1;
		}
		if (rank == 1) {
			//move is promotion, handled later
			outofbounds = 1;
		}
		cappiece = pos->board[newsquare];
		if (! ((cappiece >= 'a') && (cappiece <= 'z')) ) {
			// square is not black piece
			outofbounds = 1;
		}

		if (outofbounds == 0) {
			Pmoves[pmovesend].from = startsquare;
			Pmoves[pmovesend].to = newsquare;
			Pmoves[pmovesend].prom = 0;
			pmovesend += 1;
		}
		outofbounds = 0;
		newfile = file + 1;
		newrank = rank - 1;
		newsquare = fileranktosquareidx(newfile,newrank);
		if ((newfile < 0) || (newfile > 7) || (newrank < 0) || (newrank > 7)) {
			outofbounds = 1;
		}
		if (rank == 1) {
			//move is promotion, handled later
			outofbounds = 1;
		}
		cappiece = pos->board[newsquare];
		if (! ((cappiece >= 'a') && (cappiece <= 'z')) ) {
			// square is not black piece
			outofbounds = 1;
		}
		if (outofbounds == 0) {
			Pmoves[pmovesend].from = startsquare;
			Pmoves[pmovesend].to = newsquare;
			Pmoves[pmovesend].prom = 0;
			pmovesend += 1;
		}
		// en passant
		outofbounds = 0;
		newfile = file - 1;
		newrank = rank - 1;
		newsquare = fileranktosquareidx(newfile,newrank);
		if ((newfile < 0) || (newfile > 7) || (newrank < 0) || (newrank > 7)) {
			outofbounds = 1;
		}
		if (outofbounds == 0) {
			if (newsquare == pos->epsquare) {
				Pmoves[pmovesend].from = startsquare;
				Pmoves[pmovesend].to = newsquare;
				Pmoves[pmovesend].prom = 0;
				pmovesend += 1;
			}
		}
		outofbounds = 0;
		newfile = file + 1;
		newrank = rank - 1;
		newsquare = fileranktosquareidx(newfile,newrank);
		if ((newfile < 0) || (newfile > 7) || (newrank < 0) || (newrank > 7)) {
			outofbounds = 1;
		}
		if (outofbounds == 0) {
			if (newsquare == pos->epsquare) {
				Pmoves[pmovesend].from = startsquare;
				Pmoves[pmovesend].to = newsquare;
				Pmoves[pmovesend].prom = 0;
				pmovesend += 1;
			}
		}
		// promotion
		if ((rank == 1)) {
			// capture up and left
			outofbounds = 0;
			newfile = file - 1;
			newrank = rank - 1;
			newsquare = fileranktosquareidx(newfile,newrank);
			cappiece = pos->board[newsquare];
			if (! ((cappiece >= 'a') && (cappiece <= 'z')) ) {
				// piece is not black piece
				outofbounds = 1;
			}
			if (outofbounds == 0) {
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
			}
			// capture up and right
			outofbounds = 0;
			newfile = file + 1;
			newrank = rank - 1;
			newsquare = fileranktosquareidx(newfile,newrank);
			cappiece = pos->board[newsquare];
			if (! ((cappiece >= 'a') && (cappiece <= 'z')) ) {
				// piece is not black piece
				outofbounds = 1;
			}
			if (outofbounds == 0) {
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
			}
			// move forward one square
			outofbounds = 0;
			newfile = file;
			newrank = rank - 1;
			newsquare = fileranktosquareidx(newfile,newrank);
			cappiece = pos->board[newsquare];
			if (cappiece != '0' ) {
				// promotion square is not empty
				outofbounds = 1;
			}
			if (outofbounds == 0) {
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
			}
		
		}
			
		
	}
	if ((pos->tomove == BLACK) && (piece == 'p')) {
		// one square forward
		outofbounds = 0;
		newfile = file;
		newrank = rank + 1;
		newsquare = fileranktosquareidx(newfile,newrank);
		if ((newfile < 0) || (newfile > 7) || (newrank < 0) || (newrank > 7)) {
			outofbounds = 1;
		}
		if (rank == 6) {
			//move is promotion, handled later
			outofbounds = 1;
		}
		if (pos->board[newsquare] != '0') outofbounds = 1;
		if (outofbounds == 0) {
			Pmoves[pmovesend].from = startsquare;
			Pmoves[pmovesend].to = newsquare;
			Pmoves[pmovesend].prom = 0;
			pmovesend += 1;
		}
		// two squares forward
		if ((rank == 1) && (pos->tomove == BLACK)) {
			//check if square one forward is blocked
			newfile = file;
			newrank = rank + 1;
			newsquare = fileranktosquareidx(newfile,newrank);
			if ((newfile < 0) || (newfile > 7) || (newrank < 0) || (newrank > 7)) {
				outofbounds = 1;
			}
			if (pos->board[newsquare] != '0') outofbounds = 1;
			// check if square two forward is blocked
			newfile = file;
			newrank = rank + 2;
			newsquare = fileranktosquareidx(newfile,newrank);
			if ((newfile < 0) || (newfile > 7) || (newrank < 0) || (newrank > 7)) {
				outofbounds = 1;
			}
			if (pos->board[newsquare] != '0') outofbounds = 1;
			if (outofbounds == 0) {
				// add pawn move
				Pmoves[pmovesend].from = startsquare;
				Pmoves[pmovesend].to = newsquare;
				Pmoves[pmovesend].prom = 0;
				pmovesend += 1;
			}
		}
		// captures
		outofbounds = 0;
		newfile = file - 1;
		newrank = rank + 1;
		newsquare = fileranktosquareidx(newfile,newrank);
		if ((newfile < 0) || (newfile > 7) || (newrank < 0) || (newrank > 7)) {
			outofbounds = 1;
		}
		if (rank == 6) {
			//move is promotion, handled later
			outofbounds = 1;
		}
		cappiece = pos->board[newsquare];
		if (! ((cappiece >= 'A') && (cappiece <= 'Z')) ) {
			// square is not white piece
			outofbounds = 1;
		}
		if (outofbounds == 0) {
			Pmoves[pmovesend].from = startsquare;
			Pmoves[pmovesend].to = newsquare;
			Pmoves[pmovesend].prom = 0;
			pmovesend += 1;
		}
		outofbounds = 0;
		newfile = file + 1;
		newrank = rank + 1;
		newsquare = fileranktosquareidx(newfile,newrank);
		if ((newfile < 0) || (newfile > 7) || (newrank < 0) || (newrank > 7)) {
			outofbounds = 1;
		}
		if (rank == 6) {
			//move is promotion, handled later
			outofbounds = 1;
		}
		cappiece = pos->board[newsquare];
		if (! ((cappiece >= 'A') && (cappiece <= 'Z')) ) {
			// square is not white piece
			outofbounds = 1;
		}
		if (outofbounds == 0) {
			Pmoves[pmovesend].from = startsquare;
			Pmoves[pmovesend].to = newsquare;
			Pmoves[pmovesend].prom = 0;
			pmovesend += 1;
		}
		// en passant
		outofbounds = 0;
		newfile = file - 1;
		newrank = rank + 1;
		newsquare = fileranktosquareidx(newfile,newrank);
		if ((newfile < 0) || (newfile > 7) || (newrank < 0) || (newrank > 7)) {
			outofbounds = 1;
		}
		if (outofbounds == 0) {
			if (newsquare == pos->epsquare) {
				Pmoves[pmovesend].from = startsquare;
				Pmoves[pmovesend].to = newsquare;
				Pmoves[pmovesend].prom = 0;
				pmovesend += 1;
			}
		}
		outofbounds = 0;
		newfile = file + 1;
		newrank = rank + 1;
		newsquare = fileranktosquareidx(newfile,newrank);
		if ((newfile < 0) || (newfile > 7) || (newrank < 0) || (newrank > 7)) {
			outofbounds = 1;
		}
		if (outofbounds == 0) {
			if (newsquare == pos->epsquare) {
				Pmoves[pmovesend].from = startsquare;
				Pmoves[pmovesend].to = newsquare;
				Pmoves[pmovesend].prom = 0;
				pmovesend += 1;
			}
		}

		// promotion
		if ((rank == 6)) {
			// capture down and left
			outofbounds = 0;
			newfile = file - 1;
			newrank = rank + 1;
			newsquare = fileranktosquareidx(newfile,newrank);
			cappiece = pos->board[newsquare];
			if (! ((cappiece >= 'A') && (cappiece <= 'Z')) ) {
				// piece is not white piece
				outofbounds = 1;
			}
			if (outofbounds == 0) {
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
			}
			// capture down and right
			outofbounds = 0;
			newfile = file + 1;
			newrank = rank + 1;
			newsquare = fileranktosquareidx(newfile,newrank);
			cappiece = pos->board[newsquare];
			if (! ((cappiece >= 'A') && (cappiece <= 'Z')) ) {
				// piece is not white piece
				outofbounds = 1;
			}
			if (outofbounds == 0) {
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
			}
			// move forward one square
			outofbounds = 0;
			newfile = file;
			newrank = rank + 1;
			newsquare = fileranktosquareidx(newfile,newrank);
			cappiece = pos->board[newsquare];
			if (cappiece != '0' ) {
				// promotion square is not empty
				outofbounds = 1;
			}
			if (outofbounds == 0) {
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
			}
		
		}
	}
	return pmovesend;		
}
int genLegalKnightMoves(struct position *pos, int square, struct move *Nmoves) {
	int file, rank;
	int newfile, newrank;
	int outofbounds;
	int startsquare;
	int newsquare;
	char cappiece;
	int nmovesend = 0;
	char piece;
	int i;
	file = getfile(square);
	rank = getrank(square);
	startsquare = fileranktosquareidx(file,rank);
	piece = pos->board[startsquare];
	if ((pos->tomove == WHITE) && (piece == 'N')) {
		int Ndirs[8][2] = {{-2,-1},{-1,-2},{+1,-2},{+2,-1},{+2,+1},{+1,+2},{-1,+2},{-2,+1}};
		for (i = 0;i < 8;i++) {
			outofbounds = 0;
			newfile = file + Ndirs[i][0];
			newrank = rank + Ndirs[i][1];
			newsquare = fileranktosquareidx(newfile,newrank);
			cappiece = pos->board[newsquare];
			if ((newfile < 0) || (newfile > 7) || (newrank < 0) || (newrank > 7)) {
				outofbounds = 1;
			}
			if ((cappiece >= 'A') && (cappiece <= 'Z')) {
				// piece is white piece
				outofbounds = 1;
			}
			if (outofbounds == 0) {
				Nmoves[nmovesend].from = startsquare;
				Nmoves[nmovesend].to = newsquare;
				Nmoves[nmovesend].prom = 0;
				nmovesend += 1;
			}
		}
	}
	if ((pos->tomove == BLACK) && (piece == 'n')) {
		int Ndirs[8][2] = {{-2,-1},{-1,-2},{+1,-2},{+2,-1},{+2,+1},{+1,+2},{-1,+2},{-2,+1}};
		for (i = 0;i < 8;i++) {
			outofbounds = 0;
			newfile = file + Ndirs[i][0];
			newrank = rank + Ndirs[i][1];
			newsquare = fileranktosquareidx(newfile,newrank);
			cappiece = pos->board[newsquare];
			if ((newfile < 0) || (newfile > 7) || (newrank < 0) || (newrank > 7)) {
				outofbounds = 1;
			}
			if ((cappiece >= 'a') && (cappiece <= 'z')) {
				// piece is black piece
				outofbounds = 1;
			}
			if (outofbounds == 0) {
				Nmoves[nmovesend].from = startsquare;
				Nmoves[nmovesend].to = newsquare;
				Nmoves[nmovesend].prom = 0;
				nmovesend += 1;
			}
		}
	}
	return nmovesend;
}
int genLegalBishopMoves(struct position *pos, int square, struct move *Bmoves) {
	int file, rank;
	int newfile, newrank;
	int outofbounds;
	int startsquare;
	int newsquare;
	char cappiece;
	int bmovesend = 0;
	char piece;
	int i;
	int Bdirs[4][2] = {{-1,-1},{-1,+1},{+1,-1},{+1,+1}};
	file = getfile(square);
	rank = getrank(square);
	startsquare = fileranktosquareidx(file,rank);
	piece = pos->board[startsquare];
	// gen bishop moves
	for (i = 0;i < 4;i++) {
		outofbounds = 0;
		file = getfile(square);
		rank = getrank(square);
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
				if ((pos->board[newsquare] >= 'A') && (pos->board[newsquare] <= 'Z')) {
					// char is uppercase so is white piece
					// line of sight is blocked by friendly piece so not check
					outofbounds = 1;
					break;
				}
				if ((pos->board[newsquare] >= 'a') && (pos->board[newsquare] <= 'z')) {
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
				if ((pos->board[newsquare] >= 'a') && (pos->board[newsquare] <= 'z')) {
					// char is lowercase so is black piece
					// line of sight is blocked by friendly piece so not check
					outofbounds = 1;
					break;
				}
				if ((pos->board[newsquare] >= 'A') && (pos->board[newsquare] <= 'Z')) {
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
	int file, rank;
	int newfile, newrank;
	int outofbounds;
	int startsquare;
	int newsquare;
	char cappiece;
	int rmovesend = 0;
	char piece;
	int i;
	int Rdirs[4][2] = {{0,1},{0,-1},{-1,0},{1,0}};
	file = getfile(square);
	rank = getrank(square);
	startsquare = fileranktosquareidx(file,rank);
	piece = pos->board[startsquare];
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
				if ((pos->board[newsquare] >= 'A') && (pos->board[newsquare] <= 'Z')) {
					// char is uppercase so is white piece
					// line of sight is blocked by friendly piece so not check
					outofbounds = 1;
					break;
				}
				if ((pos->board[newsquare] >= 'a') && (pos->board[newsquare] <= 'z')) {
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
				if ((pos->board[newsquare] >= 'a') && (pos->board[newsquare] <= 'z')) {
					// char is lowercase so is black piece
					// line of sight is blocked by friendly piece so not check
					outofbounds = 1;
					break;
				}
				if ((pos->board[newsquare] >= 'A') && (pos->board[newsquare] <= 'Z')) {
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
	int file, rank;
	int newfile, newrank;
	int outofbounds;
	int startsquare;
	int newsquare;
	char cappiece;
	int qmovesend = 0;
	char piece;
	int i;
	int Qdirs[8][2] = {{-1,-1},{-1,+1},{+1,-1},{+1,+1},{0,1},{0,-1},{-1,0},{1,0}};
	file = getfile(square);
	rank = getrank(square);
	startsquare = fileranktosquareidx(file,rank);
	piece = pos->board[startsquare];
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
				if ((pos->board[newsquare] >= 'A') && (pos->board[newsquare] <= 'Z')) {
					// char is uppercase so is white piece
					// line of sight is blocked by friendly piece so not check
					outofbounds = 1;
					break;
				}
				if ((pos->board[newsquare] >= 'a') && (pos->board[newsquare] <= 'z')) {
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
				if ((pos->board[newsquare] >= 'a') && (pos->board[newsquare] <= 'z')) {
					// char is lowercase so is black piece
					// line of sight is blocked by friendly piece so not check
					outofbounds = 1;
					break;
				}
				if ((pos->board[newsquare] >= 'A') && (pos->board[newsquare] <= 'Z')) {
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
	int file, rank;
	int newfile, newrank;
	int outofbounds;
	int startsquare;
	int newsquare;
	char cappiece;
	int kmovesend = 0;
	char piece;
	int i;
	file = getfile(square);
	rank = getrank(square);
	startsquare = fileranktosquareidx(file,rank);
	piece = pos->board[startsquare];
	if ((pos->tomove == WHITE) && (piece == 'K')) {
		int Kdirs[8][2] = {{-1,-1},{-1,+1},{+1,-1},{+1,+1},{0,1},{0,-1},{-1,0},{1,0}};
		for (i = 0;i < 8;i++) {
			outofbounds = 0;
			newfile = file + Kdirs[i][0];
			newrank = rank + Kdirs[i][1];
			newsquare = fileranktosquareidx(newfile,newrank);
			cappiece = pos->board[newsquare];
			if ((newfile < 0) || (newfile > 7) || (newrank < 0) || (newrank > 7)) {
				outofbounds = 1;
			}
			if ((cappiece >= 'A') && (cappiece <= 'Z')) {
				// piece is white piece
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
		if ((pos->board[56] != 'R') || (pos->board[57] != '0') || (pos->board[58] != '0') || (pos->board[59] != '0') || (pos->board[60] != 'K')) {
			outofbounds = 1;
		}
		//if (isCheck(pos,57) == 1) outofbounds = 1;
		if (isCheck(pos,58) == 1) outofbounds = 1;
		if (isCheck(pos,59) == 1) outofbounds = 1;
		if (isCheck(pos,60) == 1) outofbounds = 1;
		if (pos->WcastleQS == 0) outofbounds = 1;
		if (outofbounds == 0) {
			Kmoves[kmovesend].from = 60;
			Kmoves[kmovesend].to = 58;
			Kmoves[kmovesend].prom = 0;
			kmovesend += 1;
		}
		// White castling KS
		outofbounds = 0;
		if ((pos->board[60] != 'K') || (pos->board[61] != '0') || (pos->board[62] != '0') || (pos->board[63] != 'R')) {
			outofbounds = 1;
		}
		if (isCheck(pos,61) == 1) outofbounds = 1;
		if (isCheck(pos,62) == 1) outofbounds = 1;
		if (isCheck(pos,60) == 1) outofbounds = 1;
		if (pos->WcastleKS == 0) outofbounds = 1;
		if (outofbounds == 0) {
			Kmoves[kmovesend].from = 60;
			Kmoves[kmovesend].to = 62;
			Kmoves[kmovesend].prom = 0;
			kmovesend += 1;
		}
	}
	if ((pos->tomove == BLACK) && (piece == 'k')) {
		int Kdirs[8][2] = {{-1,-1},{-1,+1},{+1,-1},{+1,+1},{0,1},{0,-1},{-1,0},{1,0}};
		for (i = 0;i < 8;i++) {
			outofbounds = 0;
			newfile = file + Kdirs[i][0];
			newrank = rank + Kdirs[i][1];
			newsquare = fileranktosquareidx(newfile,newrank);
			cappiece = pos->board[newsquare];
			if ((newfile < 0) || (newfile > 7) || (newrank < 0) || (newrank > 7)) {
				outofbounds = 1;
			}
			if ((cappiece >= 'a') && (cappiece <= 'z')) {
				// piece is black piece
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
		if ((pos->board[0] != 'r') || (pos->board[1] != '0') || (pos->board[2] != '0') || (pos->board[3] != '0') || (pos->board[4] != 'k')) {
			outofbounds = 1;
		}
		//if (isCheck(pos,1) == 1) outofbounds = 1;
		if (isCheck(pos,2) == 1) outofbounds = 1;
		if (isCheck(pos,3) == 1) outofbounds = 1;
		if (isCheck(pos,4) == 1) outofbounds = 1;
		if (pos->BcastleQS == 0) outofbounds = 1;
		if (outofbounds == 0) {
			Kmoves[kmovesend].from = 4;
			Kmoves[kmovesend].to = 2;
			Kmoves[kmovesend].prom = 0;
			kmovesend += 1;
		}
		// White castling KS
		outofbounds = 0;
		if ((pos->board[4] != 'k') || (pos->board[5] != '0') || (pos->board[6] != '0') || (pos->board[7] != 'r')) {
			outofbounds = 1;
		}
		if (isCheck(pos,4) == 1) outofbounds = 1;
		if (isCheck(pos,5) == 1) outofbounds = 1;
		if (isCheck(pos,6) == 1) outofbounds = 1;
		if (pos->BcastleKS == 0) outofbounds = 1;
		if (outofbounds == 0) {
			Kmoves[kmovesend].from = 4;
			Kmoves[kmovesend].to = 6;
			Kmoves[kmovesend].prom = 0;
			kmovesend += 1;
		}
	}
	return kmovesend;
}
int genLegalMoves(struct position *pos, struct move *moves) {
	struct move newmoves[MAX_MOVES];
	char piece;
	int movesend = 0; // end of moves array
	int num_moves = 0;
	for (int i = 0;i < 64;i++) {
		piece = pos->board[i]; 
		if ((piece == 'P') || (piece == 'p')) {
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
	return movesend;
}
U64 perft(struct position *pos, int depth) {
	if (depth == 0) return 1;
	U64 nodes = 0;
	struct move moves[MAX_MOVES];
	int n_moves = genLegalMoves(pos,moves);
	for (int i = 0; i < n_moves;i++) {
		makeMove(&moves[i], pos);
		nodes += perft(pos,depth - 1);
		unmakeMove(pos);
	}
	return nodes;
}
int splitperft(struct position *pos, int depth) {
	int n_moves, i;
	struct move moves[2048];
	int nodes = 0;
	if (depth == 0) return 1;
	n_moves = genLegalMoves(pos,moves);
	for (i = 0; i < n_moves;i++) {
		makeMove(&moves[i], pos);
		printf("%s - %d\n",movetostr(moves[i]), perft(pos,depth - 1));
		nodes += perft(pos,depth - 1);
		unmakeMove(pos);
	}
	return nodes;
}
/*
u64 Perft(int depth)
{
    MOVE move_list[256];
    int n_moves, i;
    u64 nodes = 0;

    if (depth == 0) return 1;

    n_moves = GenerateLegalMoves(move_list);
    for (i = 0; i < n_moves; i++) {
        MakeMove(move_list[i]);
        nodes += Perft(depth - 1);
        UndoMove(move_list[i]);
    }
    return nodes;
}
*/