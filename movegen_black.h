#ifndef MOVEGENB_H
#define MOVEGENB_H

#include "functions.h"

int genPawnMoves_B(const struct position *pos, int square, struct move *moves) {
	assert(pos);
	assert(square >= 0 && square <= 63);
	assert(moves);

	int num_moves = 0;
	const int x = getfile(square);
	const int y = getrank(square);

	// Singles
	if (pos->board[fileranktosquareidx(x, y + 1)] == '0') {
		const int nx = x;
		const int ny = y + 1;
		int newsquare = fileranktosquareidx(nx,ny);
		if (ny == 7) {
			// promotion

			// add moves
			moves[num_moves].from = square;
			moves[num_moves].to = newsquare;
			moves[num_moves].prom = 'q';
			moves[num_moves].cappiece = pos->board[newsquare];
			num_moves += 1;

			moves[num_moves].from = square;
			moves[num_moves].to = newsquare;
			moves[num_moves].prom = 'r';
			moves[num_moves].cappiece = pos->board[newsquare];
			num_moves += 1;

			moves[num_moves].from = square;
			moves[num_moves].to = newsquare;
			moves[num_moves].prom = 'b';
			moves[num_moves].cappiece = pos->board[newsquare];
			num_moves += 1;

			moves[num_moves].from = square;
			moves[num_moves].to = newsquare;
			moves[num_moves].prom = 'n';
			moves[num_moves].cappiece = pos->board[newsquare];
			num_moves += 1;
		}
		else {
			//add move
			moves[num_moves].from = square;
			moves[num_moves].to = newsquare;
			moves[num_moves].prom = 0;
			moves[num_moves].cappiece = pos->board[newsquare];;
			num_moves += 1;
		}
	}
	// two forward
	if (A7 <= square && square <= H7 &&
		pos->board[fileranktosquareidx(x, y + 1)] == '0' &&
		pos->board[fileranktosquareidx(x, y + 2)] == '0') {

		// Add move
		moves[num_moves].from = square;
		moves[num_moves].to = fileranktosquareidx(x, y + 2);
		moves[num_moves].prom = 0;
		moves[num_moves].cappiece = '0';
		num_moves += 1;
	}
	// captures
	for (int i = 0;i < 2;i++) {
		const int nx = x + BPdirs[i][0];
		const int ny = y + BPdirs[i][1];
		const int newsquare = fileranktosquareidx(nx, ny);
		const char cappiece = pos->board[newsquare];

		// Borders
		if (nx < 0 || nx > 7) {
			continue;
		}

		if (cappiece == '0') {
			continue;
		}
		if (isWhitePiece(cappiece)) {
			if (ny == 7) {
				// promotion capture

				// add moves
				moves[num_moves].from = square;
				moves[num_moves].to = newsquare;
				moves[num_moves].prom = 'q';
				moves[num_moves].cappiece = cappiece;
				num_moves += 1;

				moves[num_moves].from = square;
				moves[num_moves].to = newsquare;
				moves[num_moves].prom = 'r';
				moves[num_moves].cappiece = cappiece;
				num_moves += 1;

				moves[num_moves].from = square;
				moves[num_moves].to = newsquare;
				moves[num_moves].prom = 'b';
				moves[num_moves].cappiece = cappiece;
				num_moves += 1;

				moves[num_moves].from = square;
				moves[num_moves].to = newsquare;
				moves[num_moves].prom = 'n';
				moves[num_moves].cappiece = cappiece;
				num_moves += 1;
			}
			else {
				//add move
				moves[num_moves].from = square;
				moves[num_moves].to = newsquare;
				moves[num_moves].prom = 0;
				moves[num_moves].cappiece = cappiece;
				num_moves += 1;
			}
		}
	}
	return num_moves;
}
int genKnightMoves_B(const struct position *pos, int square, struct move *moves) {
	assert(pos);
	assert(square >= 0 && square <= 63);
	assert(moves);
	int num_moves = 0;
	const int x = getfile(square);
	const int y = getrank(square);
	for (int i = 0;i < 8;i++) {
		const int nx = x + Ndirs[i][0];
		const int ny = y + Ndirs[i][1];;
		const int newsquare = fileranktosquareidx(nx, ny);
		const char cappiece = pos->board[newsquare];

		// Borders
		if (nx < 0 || nx > 7 || ny < 0 || ny > 7) {
			continue;
		}

		if (isBlackPiece(cappiece)) {
			continue;
		}

		//add move
		moves[num_moves].from = square;
		moves[num_moves].to = newsquare;
		moves[num_moves].prom = 0;
		moves[num_moves].cappiece = cappiece;
		num_moves += 1;

	}
	return num_moves;
}
int genBishopMoves_B(const struct position *pos, int square, struct move *moves) {
	assert(pos);
	assert(square >= 0 && square <= 63);
	assert(moves);
	int num_moves = 0;
	const int x = getfile(square);
	const int y = getrank(square);
	for (int i = 0;i < 4;i++) {
		for (int j = 1;j <= 7;j++) {
			int nx = x + j * Bdirs[i][0];
			int ny = y + j * Bdirs[i][1];
			const int newsquare = fileranktosquareidx(nx, ny);
			const char cappiece = pos->board[newsquare];
			// Borders

			if (nx < 0 || nx > 7 || ny < 0 || ny > 7) {
				break;
			}

			if (isBlackPiece(cappiece)) {
				break;
			}
			else if (isWhitePiece(cappiece)) {
				moves[num_moves].from = square;
				moves[num_moves].to = newsquare;
				moves[num_moves].prom = 0;
				moves[num_moves].cappiece = cappiece;
				num_moves += 1;
				break;
			}
			else if (cappiece == '0') {
				moves[num_moves].from = square;
				moves[num_moves].to = newsquare;
				moves[num_moves].prom = 0;
				moves[num_moves].cappiece = '0';
				num_moves += 1;
			}
			else {
				assert(0);
			}
		}
	}
	return num_moves;
}
int genRookMoves_B(const struct position *pos, int square, struct move *moves) {
	assert(pos);
	assert(square >= 0 && square <= 63);
	assert(moves);
	int num_moves = 0;
	const int x = getfile(square);
	const int y = getrank(square);
	for (int i = 0;i < 4;i++) {
		for (int j = 1;j <= 7;j++) {
			int nx = x + j * Rdirs[i][0];
			int ny = y + j * Rdirs[i][1];
			const int newsquare = fileranktosquareidx(nx, ny);
			const char cappiece = pos->board[newsquare];

			if (nx < 0 || nx > 7 || ny < 0 || ny > 7) {
				break;
			}

			if (isBlackPiece(cappiece)) {
				break;
			}
			else if (isWhitePiece(cappiece)) {
				moves[num_moves].from = square;
				moves[num_moves].to = newsquare;
				moves[num_moves].prom = 0;
				moves[num_moves].cappiece = cappiece;
				num_moves += 1;
				break;
			}
			else if (cappiece == '0') {
				moves[num_moves].from = square;
				moves[num_moves].to = newsquare;
				moves[num_moves].prom = 0;
				moves[num_moves].cappiece = '0';
				num_moves += 1;
			}
			else {
				assert(0);
			}
		}
	}
	return num_moves;
}
int genQueenMoves_B(const struct position *pos, int square, struct move *moves) {
	assert(pos);
	assert(square >= 0 && square <= 63);
	assert(moves);
	int num_moves = 0;
	const int x = getfile(square);
	const int y = getrank(square);
	for (int i = 0;i < 8;i++) {
		for (int j = 1;j <= 7;j++) {
			int nx = x + j * Qdirs[i][0];
			int ny = y + j * Qdirs[i][1];
			const int newsquare = fileranktosquareidx(nx, ny);
			const char cappiece = pos->board[newsquare];

			if (nx < 0 || nx > 7 || ny < 0 || ny > 7) {
				break;
			}

			if (isBlackPiece(cappiece)) {
				break;
			}
			else if (isWhitePiece(cappiece)) {
				moves[num_moves].from = square;
				moves[num_moves].to = newsquare;
				moves[num_moves].prom = 0;
				moves[num_moves].cappiece = cappiece;
				num_moves += 1;
				break;
			}
			else if (cappiece == '0') {
				moves[num_moves].from = square;
				moves[num_moves].to = newsquare;
				moves[num_moves].prom = 0;
				moves[num_moves].cappiece = '0';
				num_moves += 1;
			}
			else {
				assert(0);
			}
		}
	}
	return num_moves;
}
int genKingMoves_B(const struct position *pos, int square, struct move *moves) {
	assert(pos);
	assert(square >= 0 && square <= 63);
	assert(moves);
	int num_moves = 0;
	const int x = getfile(square);
	const int y = getrank(square);
	for (int i = 0;i < 8;i++) {
		const int nx = x + Kdirs[i][0];
		const int ny = y + Kdirs[i][1];;
		const int newsquare = fileranktosquareidx(nx, ny);
		const char cappiece = pos->board[newsquare];

		// Borders
		if (nx < 0 || nx > 7 || ny < 0 || ny > 7) {
			continue;
		}
		if (isBlackPiece(cappiece)) {
			continue;
		}
		else if (isWhitePiece(cappiece)) {
			moves[num_moves].from = square;
			moves[num_moves].to = newsquare;
			moves[num_moves].prom = 0;
			moves[num_moves].cappiece = cappiece;
			num_moves += 1;
		}
		else if (cappiece == '0') {
			moves[num_moves].from = square;
			moves[num_moves].to = newsquare;
			moves[num_moves].prom = 0;
			moves[num_moves].cappiece = '0';
			num_moves += 1;
		}
		else {
			assert(0);
		}
	}
	if ((pos->BcastleKS == 1) &&
		pos->board[F8] == '0' &&
		pos->board[G8] == '0' &&
		!isAttacked(pos, E8, WHITE) &&
		!isAttacked(pos, F8, WHITE) &&
		!isAttacked(pos, G8, WHITE)) {
			// Add move
			moves[num_moves].from = E8;
			moves[num_moves].to = G8;
			moves[num_moves].prom = 0;
			moves[num_moves].cappiece = '0';
			num_moves += 1;
	}
	// Queenside castling
	if ((pos->BcastleQS == 1) &&
		pos->board[D8] == '0' &&
		pos->board[C8] == '0' &&
		pos->board[B8] == '0' &&
		!isAttacked(pos, E8, WHITE) &&
		!isAttacked(pos, D8, WHITE) &&
		!isAttacked(pos, C8, WHITE)) {
			// Add move
			moves[num_moves].from = E8;
			moves[num_moves].to = C8;
			moves[num_moves].prom = 0;
			moves[num_moves].cappiece = '0';
			num_moves += 1;
	}
	return num_moves;
}
int genMoves_B(const struct position *pos, struct move *moves) {
	assert(pos);
	assert(moves);
	int num_moves = 0;
	// en passant
	if (pos->epsquare != -1) {
		const int x = getfile(pos->epsquare);
		const int y = getrank(pos->epsquare);

		for (int i = 0;i < 2;i++) {

			const int nx = x + BPdirs[i][0];
			const int ny = y - BPdirs[i][1];
			const int idx = fileranktosquareidx(nx, ny);
			const char piece = pos->board[idx];
			// Borders
			if (nx < 0 || nx > 7) {
				continue;
			}

			if (piece == 'p') {
				// Add move
				moves[num_moves].from = idx;
				moves[num_moves].to = pos->epsquare;
				moves[num_moves].prom = 0;
				moves[num_moves].cappiece = 'P';
				num_moves++;
			}
		}
	}
	for (int i = 0;i < 64;i++) {
		char piece = pos->board[i];
		if (piece == 'p') {
			num_moves += genPawnMoves_B(pos,i,&moves[num_moves]);
		}
		else if (piece == 'n') {
			num_moves += genKnightMoves_B(pos,i,&moves[num_moves]);
		}
		else if (piece == 'b') {
			num_moves += genBishopMoves_B(pos,i,&moves[num_moves]);
		}
		else if (piece == 'r') {
			num_moves += genRookMoves_B(pos,i,&moves[num_moves]);
		}
		else if (piece == 'q') {
			num_moves += genQueenMoves_B(pos,i,&moves[num_moves]);
		}
		else if (piece == 'k') {
			num_moves += genKingMoves_B(pos,i,&moves[num_moves]);
		}
	}
	return num_moves;
}
#endif
