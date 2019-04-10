#ifndef MOVEGENW_H
#define MOVEGENW_H

#include "functions.h"

int genPawnMoves_W(struct position *pos, int square, struct move *moves) {
	assert(pos);
	assert(square >= 0 && square <= 63);
	assert(moves);
	
    int num_moves = 0;
    const int x = getfile(square);
    const int y = getrank(square);
	
    // Singles
    if (pos->board[fileranktosquareidx(x, y - 1)] == '0') {
        const int nx = x;
        const int ny = y - 1;
		int newsquare = fileranktosquareidx(nx,ny);
		if (ny == 0) {
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
			moves[num_moves].cappiece = pos->board[newsquare];
			num_moves += 1;
		}
	}
	// two forward
	if (A2 <= square && square <= H2 &&
        pos->board[fileranktosquareidx(x, y - 1)] == '0' &&
        pos->board[fileranktosquareidx(x, y - 2)] == '0') {
		
        // Add move
		moves[num_moves].from = square;
		moves[num_moves].to = fileranktosquareidx(x, y - 2);
		moves[num_moves].prom = 0;
		moves[num_moves].cappiece = '0';
		num_moves += 1;
    }
	// captures
	for (int i = 0;i < 2;i++) {
		const int nx = x + WPdirs[i][0];
        const int ny = y + WPdirs[i][1];
        const int newsquare = fileranktosquareidx(nx, ny);
        const char cappiece = pos->board[newsquare];
		
		// Borders
        if (nx < 0 || nx > 7) {
            continue;
        }

        if (cappiece == '0') {
            continue;
        }
		if (isBlackPiece(cappiece)) {
			if (ny == 0) {
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
	// en passant
	int newfile = x - 1;
	int newrank = y - 1;
	int newsquare = fileranktosquareidx(newfile,newrank);
	while (1) {
		if ((newfile < 0) || (newfile > 7) || (newrank < 0) || (newrank > 7)) {
			break;
		}
		if (newsquare == pos->epsquare) {
			moves[num_moves].from = square;
			moves[num_moves].to = newsquare;
			moves[num_moves].prom = 0;
			moves[num_moves].cappiece = pos->board[newsquare];
			num_moves += 1;
		}
		break;
	}
	
	newfile = x + 1;
	newrank = y - 1;
	newsquare = fileranktosquareidx(newfile,newrank);
	while (1) {
		if ((newfile < 0) || (newfile > 7) || (newrank < 0) || (newrank > 7)) {
			break;
		}
		if (newsquare == pos->epsquare) {
			moves[num_moves].from = square;
			moves[num_moves].to = newsquare;
			moves[num_moves].prom = 0;
			moves[num_moves].cappiece = pos->board[newsquare];
			num_moves += 1;
		}
		break;
	}
	return num_moves;
}
int genKnightMoves_W(struct position *pos, int square, struct move *moves) {
	assert(pos);
	assert(square >= 0 && square <= 63);
	assert(moves);
    int num_moves = 0;
    const int x = getfile(square);
    const int y = getrank(square);
	for (int i = 0;i < 8;i++) {
		const int nx = x + Ndirs[i][0];
        const int ny = y + Ndirs[i][1];
        const int newsquare = fileranktosquareidx(nx, ny);
		const char cappiece = pos->board[newsquare];
		
		// Borders
        if (nx < 0 || nx > 7 || ny < 0 || ny > 7) {
            continue;
        }

        if (isWhitePiece(cappiece)) {
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
int genBishopMoves_W(struct position *pos, int square, struct move *moves) {
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
			
			if (isWhitePiece(cappiece)) {
				break;
			}
			else if (isBlackPiece(cappiece)) {
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
int genRookMoves_W(struct position *pos, int square, struct move *moves) {
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
			
			if (isWhitePiece(cappiece)) {
				break;
			}
			else if (isBlackPiece(cappiece)) {
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
int genQueenMoves_W(struct position *pos, int square, struct move *moves) {
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
			
			if (isWhitePiece(cappiece)) {
				break;
			}
			else if (isBlackPiece(cappiece)) {
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
int genKingMoves_W(struct position *pos, int square, struct move *moves) {
	assert(pos);
	assert(square >= 0 && square <= 63);
	assert(moves);
    int num_moves = 0;
    const int x = getfile(square);
    const int y = getrank(square);
	for (int i = 0;i < 8;i++) {
		const int nx = x + Kdirs[i][0];
        const int ny = y + Kdirs[i][1];
        const int newsquare = fileranktosquareidx(nx, ny);
		const char cappiece = pos->board[newsquare];
		
		// Borders
        if (nx < 0 || nx > 7 || ny < 0 || ny > 7) {
            continue;
        }
		if (isWhitePiece(cappiece)) {
			continue;
		}
		else if (isBlackPiece(cappiece)) {
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
	    // King side castling
	int kingpos = pos->Wkingpos;
    if ((pos->WcastleKS == 1) &&
        pos->board[F1] == '0' &&
        pos->board[G1] == '0' &&
        !isCheck(pos, E1) &&
        !isCheck(pos, F1) &&
        !isCheck(pos, G1)) {
			// Add move
			moves[num_moves].from = E1;
			moves[num_moves].to = G1;
			moves[num_moves].prom = 0;
			moves[num_moves].cappiece = '0';
			num_moves += 1;
    }
	// Queenside castling
    if ((pos->WcastleQS == 1) &&
        pos->board[D1] == '0' &&
        pos->board[C1] == '0' &&
		pos->board[B1] == '0' &&
        !isCheck(pos, E1) &&
        !isCheck(pos, D1) &&
        !isCheck(pos, C1)) {
			// Add move
			moves[num_moves].from = E1;
			moves[num_moves].to = C1;
			moves[num_moves].prom = 0;
			moves[num_moves].cappiece = '0';
			num_moves += 1;
    }
	return num_moves;
}
int genMoves_W(struct position *pos, struct move *moves) {
	assert(pos);
	assert(moves);
	int num_moves = 0;
	for (int i = 0;i < 64;i++) {
		char piece = pos->board[i];
		if (piece == 'P') {
			num_moves += genPawnMoves_W(pos,i,&moves[num_moves]);
		}
		if (piece == 'N') {
			num_moves += genKnightMoves_W(pos,i,&moves[num_moves]);
		}
		if (piece == 'B') {
			num_moves += genBishopMoves_W(pos,i,&moves[num_moves]);
		}
		if (piece == 'R') {
			num_moves += genRookMoves_W(pos,i,&moves[num_moves]);
		}
		if (piece == 'Q') {
			num_moves += genQueenMoves_W(pos,i,&moves[num_moves]);
		}
		if (piece == 'K') {
			num_moves += genKingMoves_W(pos,i,&moves[num_moves]);
		}
	}
	return num_moves;
}
#endif