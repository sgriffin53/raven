void dspboard(struct position pos) {
	char piece;
	char board[65];
	strcpy(board,pos.board);
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
		piece = board[i];
		if (piece == '0') piece = ' ';
		printf(" %c |", piece);
	}
	printf("\n  +---+---+---+---+---+---+---+---+");
	printf("\n    A   B   C   D   E   F   G   H  \n");
	printf("Side to move: ");
	if (pos.tomove == 0) printf("Black");
	else printf("White");
	printf("\n");
	/*
	int kingpos;
	if (pos.tomove == WHITE) kingpos = pos.Wkingpos;
	if (pos.tomove == BLACK) kingpos = pos.Bkingpos;
	printf("Is Check: %d",isCheck(&pos,kingpos));
	printf("\n");
	printf("EP Square: %d",pos.epsquare);
	printf("\n");
	printf("White King pos: %d",pos.Wkingpos);
	printf("\n");
	printf("Black King pos: %d",pos.Bkingpos);
	printf("\n");
	printf("White castling QS: %d",pos.WcastleQS);
	printf("\n");
	printf("White castling KS: %d",pos.WcastleKS);
	printf("\n");
	printf("Black castling QS: %d",pos.BcastleQS);
	printf("\n");
	printf("Black castling KS: %d",pos.BcastleKS);
	printf("\n");
	printf("Half moves: %d",pos.halfmoves);
	printf("\n");
	printf("Is threefold: %d",isThreefold(&pos));
	 */
	printf("\n");
}