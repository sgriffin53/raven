#include <assert.h>
#include "attacks.h"

int Qdirs[8][2] = {{-1,-1},{-1,+1},{+1,-1},{+1,+1},{0,1},{0,-1},{-1,0},{1,0}};
int Kdirs[8][2] = {{-1,-1},{-1,+1},{+1,-1},{+1,+1},{0,1},{0,-1},{-1,0},{1,0}};
int Ndirs[8][2] = {{-2,-1},{-1,-2},{+1,-2},{+2,-1},{+2,+1},{+1,+2},{-1,+2},{-2,+1}};
int Bdirs[4][2] = {{-1,-1},{-1,+1},{+1,-1},{+1,+1}};
int Rdirs[4][2] = {{0,1},{0,-1},{-1,0},{1,0}};
int WPdirs[2][2] = {{-1,-1},{+1,-1}};
int BPdirs[2][2] = {{-1,+1},{+1,+1}};

// colour is colour of attacking side
int isAttacked(const struct position *pos, int square, int colour) {
	assert(pos);
	assert(square >= 0 && square <= 63);

	const int x = getfile(square);
	const int y = getrank(square);

	if (colour == WHITE) {
		// white pawn attacks
		for (int i = 0;i < 2;i++) {
			const int nx = x + BPdirs[i][0];
			const int ny = y + BPdirs[i][1];
			const int idx = fileranktosquareidx(nx,ny);
			const char piece = pos->board[idx];

			if (nx < 0 || nx > 7 || ny < 0 || ny > 7) {
				continue;
			}
			if (piece == 'P') {
				return 1;
			}
		}

		// white knight attacks
		for (int i = 0;i < 8;i++) {
			const int nx = x + Ndirs[i][0];
			const int ny = y + Ndirs[i][1];
			const int idx = fileranktosquareidx(nx,ny);
			const char piece = pos->board[idx];

			if (nx < 0 || nx > 7 || ny < 0 || ny > 7) {
				continue;
			}
			if (piece == 'N') {
				return 1;
			}
		}

		//white bishop and queen attacks
		for (int i = 0;i < 4;i++) {
			for (int j = 1;j<=7;j++) {
				const int nx = x + j * Bdirs[i][0];
				const int ny = y + j * Bdirs[i][1];
				const int idx = fileranktosquareidx(nx,ny);
				const char piece = pos->board[idx];

				if (nx < 0 || nx > 7 || ny < 0 || ny > 7) {
					continue;
				}

				if (piece == 'B' || piece == 'Q') {
					return 1;
				}
				else if (piece != '0') {
					break;
				}
			}
		}

		//white rook and queen attacks
		for (int i = 0;i < 4;i++) {
			for (int j = 1;j<=7;j++) {
				const int nx = x + j * Rdirs[i][0];
				const int ny = y + j * Rdirs[i][1];
				const int idx = fileranktosquareidx(nx,ny);
				const char piece = pos->board[idx];

				if (nx < 0 || nx > 7 || ny < 0 || ny > 7) {
					continue;
				}

				if (piece == 'R' || piece == 'Q') {
					return 1;
				}
				else if (piece != '0') {
					break;
				}
			}
		}

		//white king attacks
		for (int i = 0;i < 8;i++) {
			const int nx = x + Kdirs[i][0];
			const int ny = y + Kdirs[i][1];
			const int idx = fileranktosquareidx(nx,ny);
			const char piece = pos->board[idx];

			if (nx < 0 || nx > 7 || ny < 0 || ny > 7) {
				continue;
			}
			if (piece == 'K') {
				return 1;
			}
		}
	}

	else if (colour == BLACK) {
		// black pawn attacks
		for (int i = 0;i < 2;i++) {
			const int nx = x + WPdirs[i][0];
			const int ny = y + WPdirs[i][1];
			const int idx = fileranktosquareidx(nx,ny);
			const char piece = pos->board[idx];

			if (nx < 0 || nx > 7 || ny < 0 || ny > 7) {
				continue;
			}
			if (piece == 'p') {
				return 1;
			}
		}

		// black knight attacks
		for (int i = 0;i < 8;i++) {
			const int nx = x + Ndirs[i][0];
			const int ny = y + Ndirs[i][1];
			const int idx = fileranktosquareidx(nx,ny);
			const char piece = pos->board[idx];

			if (nx < 0 || nx > 7 || ny < 0 || ny > 7) {
				continue;
			}
			if (piece == 'n') {
				return 1;
			}
		}

		//black bishop and queen attacks
		for (int i = 0;i < 4;i++) {
			for (int j = 1;j<=7;j++) {
				const int nx = x + j * Bdirs[i][0];
				const int ny = y + j * Bdirs[i][1];
				const int idx = fileranktosquareidx(nx,ny);
				const char piece = pos->board[idx];

				if (nx < 0 || nx > 7 || ny < 0 || ny > 7) {
					continue;
				}

				if (piece == 'b' || piece == 'q') {
					return 1;
				}
				else if (piece != '0') {
					break;
				}
			}
		}

		//black rook and queen attacks
		for (int i = 0;i < 4;i++) {
			for (int j = 1;j<=7;j++) {
				const int nx = x + j * Rdirs[i][0];
				const int ny = y + j * Rdirs[i][1];
				const int idx = fileranktosquareidx(nx,ny);
				const char piece = pos->board[idx];

				if (nx < 0 || nx > 7 || ny < 0 || ny > 7) {
					continue;
				}

				if (piece == 'r' || piece == 'q') {
					return 1;
				}
				else if (piece != '0') {
					break;
				}
			}
		}

		//black king attacks
		for (int i = 0;i < 8;i++) {
			const int nx = x + Kdirs[i][0];
			const int ny = y + Kdirs[i][1];
			const int idx = fileranktosquareidx(nx,ny);
			const char piece = pos->board[idx];

			if (nx < 0 || nx > 7 || ny < 0 || ny > 7) {
				continue;
			}
			if (piece == 'k') {
				return 1;
			}
		}
	}
	return 0;
}

int isCheck(const struct position *pos) {
	assert(pos);
	int kingpos;
	if (pos->tomove == WHITE) kingpos = pos->Wkingpos;
	else kingpos = pos->Bkingpos;
	return isAttacked(pos,kingpos,!pos->tomove);
}
