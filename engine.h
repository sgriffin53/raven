int pieceval(char inpiece) {
	if (inpiece == 'p') return 100;
	if (inpiece == 'n') return 300;
	if (inpiece == 'b') return 300;
	if (inpiece == 'r') return 500;
	if (inpiece == 'q') return 900;
	if (inpiece == 'k') return 9999;
	if (inpiece == 'P') return 100;
	if (inpiece == 'N') return 300;
	if (inpiece == 'B') return 300;
	if (inpiece == 'R') return 500;
	if (inpiece == 'Q') return 900;
	if (inpiece == 'K') return 9999;
	assert(0);
	printf("inpiece: %d\n",inpiece);
	return 0;
}
int evalBoard(struct position pos) {
	int score = 0;
	for (int i = 0;i<64;i++) {
		char piece = pos.board[i];
		if (piece != '0') {
			int pval = pieceval(piece);
			if ((piece >= 'a') && (piece <= 'z')) {
				pval = -pval;
			}
			/*
			int pstscore = PSTval(piece,i);
			score += pstscore;
			 */
			score += pval;
		}
	}
	if (pos.tomove == BLACK) return -score;
	return score;
}
int negaMax(struct position pos,int depth) {
	assert(depth >= 0);
	nodesSearched++;
	if (depth == 0) {
		return evalBoard(pos);
	}
	struct move moves[MAX_MOVES];
	int maxScore = -9999;
	int num_moves = genLegalMoves(&pos,moves);
	for (int i = 0;i < num_moves;i++) {
		pos = makeMove(moves[i],pos);
		int score = -negaMax(pos,depth - 1);
		pos = unmakeMove();
		if (score > maxScore) {
			maxScore = score;
		}
	}
	return maxScore;
}
struct move search(struct position pos, int searchdepth) {
	int bestScore = -9999;
	nodesSearched = 0;
	struct move moves[MAX_MOVES];
	int num_moves = genLegalMoves(&pos,moves);
	struct move bestmove = moves[0];
	for (int i = 0;i < num_moves;i++) {
		pos = makeMove(moves[i],pos);
		int curscore = -negaMax(pos,searchdepth-1);
		//printf("%s - %d\n",movetostr(moves[i]),curscore);
		if (curscore > bestScore) {
			bestScore = curscore;
			bestmove = moves[i];
		}
		pos = unmakeMove();
	}
	return bestmove;
}
/*
def negaMax(depth):
    globals.nodesSearched += 1
    if depth == 0:
        return evalBoard(globals.board)
    maxScore = -9999
    for move in globals.board.legal_moves:
        move = str(move)
        globals.board.push_uci(move)
        score = -negaMax(depth - 1)
        globals.board.pop()
        if score > maxScore:
            maxScore = score
    return maxScore
	*/