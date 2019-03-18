

int strsquaretoidx(char square[]) {
	int file, rank;
	file = (int)square[0] - 97;
	rank = (int)square[1] - 49;
	rank = 7 - rank;
	return (((rank) << 3) | (file));
}
int getrank(int square) {
	return (int)(square / 8);
}
int getfile(int square) {
	return square % 8;
}
int fileranktosquareidx(int file,int rank) {
	return (rank) * 8 + file;
}
struct position parsefen(char fen[]) {
	char splitstr[100][8192];
	char * token;
	// set blank position
	struct position pos = {.epsquare=-1,.board={'0','0','0','0','0','0','0','0',
												'0','0','0','0','0','0','0','0',
												'0','0','0','0','0','0','0','0',
												'0','0','0','0','0','0','0','0',
												'0','0','0','0','0','0','0','0',
												'0','0','0','0','0','0','0','0',
												'0','0','0','0','0','0','0','0',
												'0','0','0','0','0','0','0','0'},.WcastleQS=0,.WcastleKS=0,.BcastleQS=0,.BcastleKS=0,
				.tomove=1,.Wkingpos=0,.Bkingpos=0};
	int i = 0;
	int j = 0;
	token = strtok(fen," ");
	while (token != NULL) {
		strcpy(splitstr[i],token);
		i++;
		token = strtok(NULL, " ");
	}
	//int splitstrend = i; // position of end of splitstr array
	j = 0;
	for (i = 0;i<strlen(splitstr[0]);i++) {
		char letter = splitstr[0][i];
		switch (letter) {
			case 'p': pos.board[j] = 'p'; break;
			case 'n': pos.board[j] = 'n'; break;
			case 'b': pos.board[j] = 'b'; break;
			case 'r': pos.board[j] = 'r'; break;
			case 'q': pos.board[j] = 'q'; break;
			case 'k': pos.board[j] = 'k'; pos.Bkingpos = j; break;
			case 'P': pos.board[j] = 'P'; break;
			case 'N': pos.board[j] = 'N'; break;
			case 'B': pos.board[j] = 'B'; break;
			case 'R': pos.board[j] = 'R'; break;
			case 'Q': pos.board[j] = 'Q'; break;
			case 'K': pos.board[j] = 'K'; pos.Wkingpos = j; break;
			case '/': j--; break;
			case '1' : break;
			case '2' : j++; break;
			case '3' : j+=2; break;
			case '4' : j+=3; break;
			case '5' : j+=4; break;
			case '6' : j+=5; break;
			case '7' : j+=6; break;
			case '8' : j+=7; break;
		
		}
		j++;
	}
	if (strcmp(splitstr[1],"w") == 0) pos.tomove = WHITE;
	if (strcmp(splitstr[1],"b") == 0) pos.tomove = BLACK;
	for (i = 0;i < strlen(splitstr[2]);i++) {
		if (splitstr[2][i] == 'K') pos.WcastleKS = 1;
		else if (splitstr[2][i] == 'Q') pos.WcastleQS = 1;
		else if (splitstr[2][i] == 'k') pos.BcastleKS = 1;
		else if (splitstr[2][i] == 'q') pos.BcastleQS = 1;
	}
	if (strcmp(splitstr[3],"-") != 0) {
		//en passant square given
		pos.epsquare = strsquaretoidx(splitstr[3]);
	}
	return pos;
}
int isThreefold(struct position pos) {
	return 0;
}
int isCheck(struct position *pos, int kingpos) {
	int kingdisty;
	int kingdistx;
	int file, rank;
	int newsquare;
	int outofbounds;
	char enemyknight, enemypawn, enemyrook, enemybishop, enemyqueen;
	int i;
	if (pos->tomove == WHITE) {
		enemyknight = 'n';
		enemypawn = 'p';
		enemyrook = 'r';
		enemybishop = 'b';
		enemyqueen = 'q';
	}
	if (pos->tomove == BLACK) {
		enemyknight = 'N';
		enemypawn = 'P';
		enemyrook = 'R';
		enemybishop = 'B';
		enemyqueen = 'Q';
	}
	int Ndirs[8][2] = {{-2,-1},{-1,-2},{+1,-2},{+2,-1},{+2,+1},{+1,+2},{-1,+2},{-2,+1}};
	int Bdirs[4][2] = {{-1,-1},{-1,+1},{+1,-1},{+1,+1}};
	int Rdirs[4][2] = {{0,1},{0,-1},{-1,0},{1,0}};
	int WPdirs[2][2] = {{-1,-1},{+1,-1}};
	int BPdirs[2][2] = {{-1,+1},{+1,+1}};
	//int WPdirs[2][2] = {{-1,+1},{+1,+1}};
	//int BPdirs[2][2] = {{-1,-1},{+1,-1}};
	
	int Pdirs[2][2];
	if (pos->tomove == WHITE) {
		memcpy(Pdirs,WPdirs,sizeof(WPdirs));
	}
	if (pos->tomove == BLACK) {
		memcpy(Pdirs,BPdirs,sizeof(BPdirs));
	}
	//int Kdirs[8][2] = {{-1,-1},{-1,+1},{+1,-1},{+1,+1},{0,1},{0,-1},{-1,0},{1,0}};
	// check for knight attacks
	for (i = 0;i < 8;i++) {
		file = getfile(kingpos) + Ndirs[i][0];
		rank = getrank(kingpos) + Ndirs[i][1];
		newsquare = fileranktosquareidx(file,rank);
		if ((file < 0) || (file > 7) || (rank < 0) || (rank > 7)) {
			continue;
		}
		if (pos->board[newsquare] == enemyknight) {
			return 1;
		}
	}
	// check for pawn attacks
	for (i = 0;i < 2;i++) {
		file = getfile(kingpos) + Pdirs[i][0];
		rank = getrank(kingpos) + Pdirs[i][1];
		newsquare = fileranktosquareidx(file,rank);
		if ((file < 0) || (file > 7) || (rank < 0) || (rank > 7)) {
			continue;
		}
		if (pos->board[newsquare] == enemypawn) {
			return 1;
		}
	}
	// check for bishop and queen attacks on diagonals
	for (i = 0;i < 4;i++) {
		outofbounds = 0;
		file = getfile(kingpos);
		rank = getrank(kingpos);
		while (outofbounds == 0) {
			file = file + Bdirs[i][0];
			rank = rank + Bdirs[i][1];
			newsquare = fileranktosquareidx(file,rank);
			if ((file < 0) || (file > 7) || (rank < 0) || (rank > 7)) {
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
					if ((pos->board[newsquare] != enemybishop) && (pos->board[newsquare] != enemyqueen)) {
						// line of sight is blocked by enemy piece
						outofbounds = 1;
						break;
					}
				}
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
					if ((pos->board[newsquare] != enemybishop) && (pos->board[newsquare] != enemyqueen)) {
						//line of sight is blocked by enemy piece
						outofbounds = 1;
						break;
					}
				}
			}
			if ((pos->board[newsquare] == enemybishop) || (pos->board[newsquare] == enemyqueen)) {
				return 1;
			}
		}
	}
	// check for rook and queen attacks along laterals
	for (i = 0;i < 4;i++) {
		outofbounds = 0;
		file = getfile(kingpos);
		rank = getrank(kingpos);
		while (outofbounds == 0) {
			file = file + Rdirs[i][0];
			rank = rank + Rdirs[i][1];
			newsquare = fileranktosquareidx(file,rank);
			if ((file < 0) || (file > 7) || (rank < 0) || (rank > 7)) {
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
					if ((pos->board[newsquare] != enemyrook) && (pos->board[newsquare] != enemyqueen)) {
						//line of sight is blocked by enemy piece
						outofbounds = 1;
						break;
					}
				}
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
					if ((pos->board[newsquare] != enemyrook) && (pos->board[newsquare] != enemyqueen)) {
						//line of sight is blocked by enemy piece
						outofbounds = 1;
						break;
					}
				}
			}
			if ((pos->board[newsquare] == enemyrook) || (pos->board[newsquare] == enemyqueen)) {
				return 1;
			}
		}
	}
	// check for king checks
	kingdisty = abs(getrank(pos->Bkingpos) - getrank(pos->Wkingpos));
	kingdistx = abs(getfile(pos->Wkingpos) - getfile(pos->Bkingpos));
	if ((kingdisty <= 1) && (kingdistx <= 1)) {
		// kings are next to each other so it's check
		return 1;
	}
	return 0;
}

char* movetostr(struct move move) {
	char returnstring[6];
	char startsquarefile = (char)(getfile(move.from) + 97);
	char startsquarerank = (char)(7 - getrank(move.from) + 49);
	char endsquarefile = (char)(getfile(move.to) + 97);
	char endsquarerank = (char)(7 - getrank(move.to) + 49);
	returnstring[0] = startsquarefile;
	returnstring[1] = startsquarerank;
	returnstring[2] = endsquarefile;
	returnstring[3] = endsquarerank;
	returnstring[4] = move.prom;
	returnstring[5] = 0;
	return strdup(returnstring);
}