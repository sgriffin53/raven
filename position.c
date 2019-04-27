#include "position.h"
#include "attacks.h"
#include "draw.h"

int getrank(int square) {
	assert(square >= 0 && square <= 63);
	if (!(square >= 0 && square <= 63))  printf("%d\n",square);
	return (int)(square / 8);
}

int getfile(int square) {
	assert(square >= 0 && square <= 63);
	return square % 8;
}

int strsquaretoidx(char square[]) {
	int file = (int)square[0] - 97;
	int rank = (int)square[1] - 49;
	rank = 7 - rank;
	return (((rank) << 3) | (file));
}

int isBlackPiece(char piece) {
	if ((piece >= 'a') && (piece <= 'z')) {
		return 1;
	}
	return 0;
}

int isWhitePiece(char piece) {
	if ((piece >= 'A') && (piece <= 'Z')) {
		return 1;
	}
	return 0;
}

char* squareidxtostr(int square) {
	assert(square >= 0 && square <= 63);
	char returnstring[3];
	char squarefile = (char)(getfile(square) + 97);
	char squarerank = (char)(7 - getrank(square) + 49);
	returnstring[0] = squarefile;
	returnstring[1] = squarerank;
	returnstring[2] = 0;
	return strdup(returnstring);
}

int fileranktosquareidx(int file,int rank) {
	return (rank) * 8 + file;
}

void parsefen(struct position *pos, const char *ofen) {
	assert(pos);
	assert(ofen);

	// Handle "startpos"
	if (strncmp(ofen, "startpos", 8) == 0) {
		parsefen(pos, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
		return;
	}

	char fen[strlen(ofen)];
	strcpy(fen, ofen);

	char splitstr[100][8192];
	char * token;

	// set blank position
	memset(pos, 0, sizeof(struct position));
	for (int i = 0; i < 64; ++i) pos->board[i] = '0';
	pos->epsquare = -1;

	int n = 0;
	int j = 0;

	token = strtok(fen," ");
	while (token != NULL) {
		strcpy(splitstr[n],token);
		n++;
		token = strtok(NULL, " ");
	}

	//int splitstrend = i; // position of end of splitstr array
	for (size_t i = 0;i<strlen(splitstr[0]);i++) {
		char letter = splitstr[0][i];
		switch (letter) {
			case 'p': pos->board[j] = 'p'; break;
			case 'n': pos->board[j] = 'n'; break;
			case 'b': pos->board[j] = 'b'; break;
			case 'r': pos->board[j] = 'r'; break;
			case 'q': pos->board[j] = 'q'; break;
			case 'k': pos->board[j] = 'k'; pos->Bkingpos = j; break;
			case 'P': pos->board[j] = 'P'; break;
			case 'N': pos->board[j] = 'N'; break;
			case 'B': pos->board[j] = 'B'; break;
			case 'R': pos->board[j] = 'R'; break;
			case 'Q': pos->board[j] = 'Q'; break;
			case 'K': pos->board[j] = 'K'; pos->Wkingpos = j; break;
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

	if (strcmp(splitstr[1],"w") == 0) pos->tomove = WHITE;
	if (strcmp(splitstr[1],"b") == 0) pos->tomove = BLACK;

	for (size_t i = 0;i < strlen(splitstr[2]);i++) {
		if (splitstr[2][i] == 'K') pos->WcastleKS = 1;
		else if (splitstr[2][i] == 'Q') pos->WcastleQS = 1;
		else if (splitstr[2][i] == 'k') pos->BcastleKS = 1;
		else if (splitstr[2][i] == 'q') pos->BcastleQS = 1;
	}

	if (strcmp(splitstr[3],"-") != 0) {
		//en passant square given
		pos->epsquare = strsquaretoidx(splitstr[3]);
	}

	pos->halfmoves = atoi(splitstr[4]);
	if (splitstr[4][0] == '-') pos->halfmoves = 0;
}

void dspboard(const struct position *pos) {
	assert(pos);

	printf("\n");
	printf("  +---+---+---+---+---+---+---+---+\n");
	printf("8 |");

	for (int i = 0;i<64;i++) {
		if ( ((i%8) == 0) && (i != 0) ) {
			printf("\n");
			printf("  +---+---+---+---+---+---+---+---+\n");
			printf("%d",(8 - i/8));
			printf(" |");
		}

		char piece = pos->board[i];
		if (piece == '0') piece = ' ';
		printf(" %c |", piece);
	}

	printf("\n  +---+---+---+---+---+---+---+---+");
	printf("\n    A   B   C   D   E   F   G   H  \n");

	printf("Side to move: ");
	if (pos->tomove == 0) printf("Black");
	else printf("White");
	printf("\n");
	printf("Is Check: %d",isCheck(pos));
	printf("\n");
	printf("EP Square: %d",pos->epsquare);
	printf("\n");
	printf("White King pos: %d",pos->Wkingpos);
	printf("\n");
	printf("Black King pos: %d",pos->Bkingpos);
	printf("\n");
	printf("White castling QS: %d",pos->WcastleQS);
	printf("\n");
	printf("White castling KS: %d",pos->WcastleKS);
	printf("\n");
	printf("Black castling QS: %d",pos->BcastleQS);
	printf("\n");
	printf("Black castling KS: %d",pos->BcastleKS);
	printf("\n");
	printf("Half moves: %d",pos->halfmoves);
	printf("\n");
	printf("Is threefold: %d",isThreefold(pos));
	printf("\n");
}
