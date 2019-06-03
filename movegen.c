#include <stdio.h>
#include <ctype.h>
#include "movegen.h"
#include "attacks.h"
#include "position.h"
#include "move.h"
#include "magicmoves.h"

const int BPdirs[2][2] = {{-1,-1},{+1,-1}};
const int WPdirs[2][2] = {{-1,+1},{+1,+1}};

int genKingMoves(struct position *pos, int square, struct move *moves) {
	U64 BBattacks = 0ULL;
	int num_moves = 0;
	if (pos->tomove == WHITE) {
		U64 BBking = (pos->BBkings & pos->BBwhitepieces);
		BBattacks = BBkingattacks(BBking);
		BBattacks = BBattacks & ~pos->BBwhitepieces;
	}
	else if (pos->tomove == BLACK) {
		U64 BBking = (pos->BBkings & pos->BBblackpieces);
		BBattacks = BBkingattacks(BBking);
		BBattacks = BBattacks & ~pos->BBblackpieces;
	}
	//dspBB(BBattacks);
	while (BBattacks != 0) {
		int movesquare = __builtin_ctzll(BBattacks);
		BBattacks &= ~(1ULL << movesquare);
		moves[num_moves].from = square;
		moves[num_moves].to = movesquare;
		moves[num_moves].prom = 0;
		num_moves++;
	}
	// castling moves
	// white castling
	if (pos->tomove == WHITE) {
		// King side castling
		if ((pos->WcastleKS == 1) &&
			getPiece(pos,F1) == '0' &&
			getPiece(pos,G1) == '0' &&
			!isAttacked(pos, E1, BLACK) &&
			!isAttacked(pos, F1, BLACK) &&
			!isAttacked(pos, G1, BLACK)) {
				// Add move
				moves[num_moves].from = E1;
				moves[num_moves].to = G1;
				moves[num_moves].prom = 0;
				num_moves += 1;
		}
		// Queenside castling
		if ((pos->WcastleQS == 1) &&
			getPiece(pos,D1)  == '0' &&
			getPiece(pos,C1)  == '0' &&
			getPiece(pos,B1)  == '0' &&
			!isAttacked(pos, E1, BLACK) &&
			!isAttacked(pos, D1, BLACK) &&
			!isAttacked(pos, C1, BLACK)) {
				// Add move
				moves[num_moves].from = E1;
				moves[num_moves].to = C1;
				moves[num_moves].prom = 0;
				num_moves += 1;
		}
	}
	// black castling
	else if (pos->tomove == BLACK) {
		// Kingside castling
		if ((pos->BcastleKS == 1) &&
			getPiece(pos,F8) == '0' &&
			getPiece(pos,G8) == '0' &&
			!isAttacked(pos, E8, WHITE) &&
			!isAttacked(pos, F8, WHITE) &&
			!isAttacked(pos, G8, WHITE)) {
				// Add move
				moves[num_moves].from = E8;
				moves[num_moves].to = G8;
				moves[num_moves].prom = 0;
				num_moves += 1;
		}
		// Queenside castling
		if ((pos->BcastleQS == 1) &&
			getPiece(pos,D8) == '0' &&
			getPiece(pos,C8) == '0' &&
			getPiece(pos,B8) == '0' &&
			!isAttacked(pos, E8, WHITE) &&
			!isAttacked(pos, D8, WHITE) &&
			!isAttacked(pos, C8, WHITE)) {
				// Add move
				moves[num_moves].from = E8;
				moves[num_moves].to = C8;
				moves[num_moves].prom = 0;
				num_moves += 1;
		}
	}
	return num_moves;
}
int genKnightMoves(struct position *pos, int square, struct move *moves) {
	U64 BBattacks = 0ULL;
	int num_moves = 0;
	if (pos->tomove == WHITE) {
		U64 BBknight = (1ULL << square);
		BBattacks = BBknightattacks(BBknight);
		BBattacks = BBattacks & ~pos->BBwhitepieces;
	}
	else if (pos->tomove == BLACK) {
		U64 BBknight = (1ULL << square);
		BBattacks = BBknightattacks(BBknight);
		BBattacks = BBattacks & ~pos->BBblackpieces;
	}
	//dspBB(BBattacks);
	while (BBattacks != 0) {
		int movesquare = __builtin_ctzll(BBattacks);
		BBattacks &= ~(1ULL << movesquare);
		moves[num_moves].from = square;
		moves[num_moves].to = movesquare;
		moves[num_moves].prom = 0;
		num_moves++;
	}
	return num_moves;
}
int genBishopMoves(struct position *pos, int square, struct move *moves) {
	U64 BBattacks = 0ULL;
	int num_moves = 0;
	if (pos->tomove == WHITE) {
		U64 BBoccupancy = (pos->BBwhitepieces | pos->BBblackpieces);
		BBattacks = Bmagic(square,BBoccupancy);
		BBattacks = BBattacks & ~pos->BBwhitepieces;
	}
	else if (pos->tomove == BLACK) {
		U64 BBoccupancy = (pos->BBwhitepieces | pos->BBblackpieces);
		BBattacks = Bmagic(square,BBoccupancy);
		BBattacks = BBattacks & ~pos->BBblackpieces;
	}
	while (BBattacks != 0) {
		int movesquare = __builtin_ctzll(BBattacks);
		BBattacks &= ~(1ULL << movesquare);
		moves[num_moves].from = square;
		moves[num_moves].to = movesquare;
		moves[num_moves].prom = 0;
		num_moves++;
	}
	return num_moves;
}
int genRookMoves(struct position *pos, int square, struct move *moves) {
	U64 BBattacks = 0ULL;
	int num_moves = 0;
	if (pos->tomove == WHITE) {
		U64 BBoccupancy = (pos->BBwhitepieces | pos->BBblackpieces);
		BBattacks = Rmagic(square,BBoccupancy);
		BBattacks = BBattacks & ~pos->BBwhitepieces;
	}
	else if (pos->tomove == BLACK) {
		U64 BBoccupancy = (pos->BBwhitepieces | pos->BBblackpieces);
		BBattacks = Rmagic(square,BBoccupancy);
		BBattacks = BBattacks & ~pos->BBblackpieces;
	}
	while (BBattacks != 0) {
		int movesquare = __builtin_ctzll(BBattacks);
		BBattacks &= ~(1ULL << movesquare);
		moves[num_moves].from = square;
		moves[num_moves].to = movesquare;
		moves[num_moves].prom = 0;
		num_moves++;
	}
	return num_moves;
}
int genQueenMoves(struct position *pos, int square, struct move *moves) {
	U64 BBattacks = 0ULL;
	int num_moves = 0;
	if (pos->tomove == WHITE) {
		U64 BBoccupancy = (pos->BBwhitepieces | pos->BBblackpieces);
		BBattacks = Rmagic(square,BBoccupancy) | Bmagic(square,BBoccupancy);
		BBattacks = BBattacks & ~pos->BBwhitepieces;
	}
	else if (pos->tomove == BLACK) {
		U64 BBoccupancy = (pos->BBwhitepieces | pos->BBblackpieces);
		BBattacks = Rmagic(square,BBoccupancy) | Bmagic(square,BBoccupancy);
		BBattacks = BBattacks & ~pos->BBblackpieces;
	}
	while (BBattacks != 0) {
		int movesquare = __builtin_ctzll(BBattacks);
		BBattacks &= ~(1ULL << movesquare);
		moves[num_moves].from = square;
		moves[num_moves].to = movesquare;
		moves[num_moves].prom = 0;
		num_moves++;
	}
	return num_moves;
}
int genPawnMoves(struct position *pos, int square, struct move *moves) {
	assert(pos);
	assert(moves);
	int num_moves = 0;
	U64 BBpawn = (1ULL << square);
	U64 BBmove;
	// white pawns
	if (pos->tomove == WHITE) {
		// double pawn pushe
		if (getrank(square) == 1) {
			// generate double pawn moves for pawns on 2nd rank
			BBmove = BBpawnDoublePushW(BBpawn,~(pos->BBblackpieces | pos->BBwhitepieces));
			if (BBmove != 0) {
				int movesquare = __builtin_ctzll(BBmove);
				moves[num_moves].from = square;
				moves[num_moves].to = movesquare;
				moves[num_moves].prom = 0;
				num_moves++;
			}
		}
		// single pawn push
		BBmove = BBpawnSinglePushW(BBpawn,~(pos->BBblackpieces | pos->BBwhitepieces));
		if (BBmove != 0) {
			int movesquare = __builtin_ctzll(BBmove);
			if (getrank(movesquare) == 7) {
				moves[num_moves].from = square;
				moves[num_moves].to = movesquare;
				moves[num_moves].prom = 'q';
				num_moves++;
				moves[num_moves].from = square;
				moves[num_moves].to = movesquare;
				moves[num_moves].prom = 'r';
				num_moves++;
				moves[num_moves].from = square;
				moves[num_moves].to = movesquare;
				moves[num_moves].prom = 'b';
				num_moves++;
				moves[num_moves].from = square;
				moves[num_moves].to = movesquare;
				moves[num_moves].prom = 'n';
				num_moves++;
			}
			else {
				moves[num_moves].from = square;
				moves[num_moves].to = movesquare;
				moves[num_moves].prom = 0;
				num_moves++;
			}
		}
		// attacks
		BBmove = BBpawnEastAttacksW(BBpawn) | BBpawnWestAttacksW(BBpawn);
		BBmove = (BBmove & pos->BBblackpieces);
		while (BBmove != 0) {
			int movesquare = __builtin_ctzll(BBmove);
			if (getrank(movesquare) == 7) {
				moves[num_moves].from = square;
				moves[num_moves].to = movesquare;
				moves[num_moves].prom = 'q';
				num_moves++;
				moves[num_moves].from = square;
				moves[num_moves].to = movesquare;
				moves[num_moves].prom = 'r';
				num_moves++;
				moves[num_moves].from = square;
				moves[num_moves].to = movesquare;
				moves[num_moves].prom = 'b';
				num_moves++;
				moves[num_moves].from = square;
				moves[num_moves].to = movesquare;
				moves[num_moves].prom = 'n';
				num_moves++;
			}
			else {
				moves[num_moves].from = square;
				moves[num_moves].to = movesquare;
				moves[num_moves].prom = 0;
				num_moves++;
			}
			BBmove &= ~(1ULL << movesquare);
		}
	}
	// black pawns
	else if (pos->tomove == BLACK) {
		if (getrank(square) == 6) {
			// generate double pawn moves for pawns on 7nd rank
			U64 BBmove = BBpawnDoublePushB(BBpawn,~(pos->BBblackpieces | pos->BBwhitepieces));
			if (BBmove != 0) {
				int movesquare = __builtin_ctzll(BBmove);
				moves[num_moves].from = square;
				moves[num_moves].to = movesquare;
				moves[num_moves].prom = 0;
				num_moves++;
			}
		}
		// single pawn push
		U64 BBmove = BBpawnSinglePushB(BBpawn,~(pos->BBblackpieces | pos->BBwhitepieces));
		if (BBmove != 0) {
			int movesquare = __builtin_ctzll(BBmove);
			if (getrank(movesquare) == 0) {
				moves[num_moves].from = square;
				moves[num_moves].to = movesquare;
				moves[num_moves].prom = 'q';
				num_moves++;
				moves[num_moves].from = square;
				moves[num_moves].to = movesquare;
				moves[num_moves].prom = 'r';
				num_moves++;
				moves[num_moves].from = square;
				moves[num_moves].to = movesquare;
				moves[num_moves].prom = 'b';
				num_moves++;
				moves[num_moves].from = square;
				moves[num_moves].to = movesquare;
				moves[num_moves].prom = 'n';
				num_moves++;
			}
			else {
				moves[num_moves].from = square;
				moves[num_moves].to = movesquare;
				moves[num_moves].prom = 0;
				num_moves++;
			}
		}
		// attacks
		BBmove = BBpawnEastAttacksB(BBpawn) | BBpawnWestAttacksB(BBpawn);
		BBmove = (BBmove & pos->BBwhitepieces);
		while (BBmove != 0) {
			int movesquare = __builtin_ctzll(BBmove);
			if (getrank(movesquare) == 0) {
				moves[num_moves].from = square;
				moves[num_moves].to = movesquare;
				moves[num_moves].prom = 'q';
				num_moves++;
				moves[num_moves].from = square;
				moves[num_moves].to = movesquare;
				moves[num_moves].prom = 'r';
				num_moves++;
				moves[num_moves].from = square;
				moves[num_moves].to = movesquare;
				moves[num_moves].prom = 'b';
				num_moves++;
				moves[num_moves].from = square;
				moves[num_moves].to = movesquare;
				moves[num_moves].prom = 'n';
				num_moves++;
			}
			else {
				moves[num_moves].from = square;
				moves[num_moves].to = movesquare;
				moves[num_moves].prom = 0;
				num_moves++;
			}
			BBmove &= ~(1ULL << movesquare);
		}
	}
	return num_moves;
}
int genMoves(struct position *pos, struct move *moves) {
	assert(pos);
	assert(moves);
	int num_moves = 0;
	U64 BBallpieces;
	if (pos->tomove == WHITE) BBallpieces = pos->BBwhitepieces;
	else BBallpieces = pos->BBblackpieces;
	
	// en passant moves
	
	if (pos->epsquare != -1) {
		const int x = getfile(pos->epsquare);
		const int y = getrank(pos->epsquare);
		if (pos->tomove == BLACK) {
			for (int i = 0;i < 2;i++) {
				const int nx = x + BPdirs[i][0];
				const int ny = y - BPdirs[i][1];
				// Borders
				if (nx < 0 || nx > 7) {
					continue;
				}
				const int idx = fileranktosquareidx(nx, ny);
				const char piece = getPiece(pos,idx);

				if (piece == 'p') {
					// Add move
					moves[num_moves].from = idx;
					moves[num_moves].to = pos->epsquare;
					moves[num_moves].prom = 0;
					num_moves++;
				}
			}
		}
		else if (pos->tomove == WHITE) {
			for (int i = 0;i < 2;i++) {
				const int nx = x + WPdirs[i][0];
				const int ny = y - WPdirs[i][1];
				// Borders
				if (nx < 0 || nx > 7) {
					continue;
				}
				const int idx = fileranktosquareidx(nx, ny);
				const char piece = getPiece(pos,idx);
				if (piece == 'P') {
					// Add move
					moves[num_moves].from = idx;
					moves[num_moves].to = pos->epsquare;
					moves[num_moves].prom = 0;
					num_moves++;
				}
			}
		}
	}
	
	
	while (BBallpieces != 0) {
		int square = __builtin_ctzll(BBallpieces);
		char piece = getPiece(pos,square);
		if ((piece == 'K') || (piece == 'k')) {
			num_moves += genKingMoves(pos,square,&moves[num_moves]);
		}
		else if ((piece == 'P') || (piece == 'p')) {
			num_moves += genPawnMoves(pos,square,&moves[num_moves]);
		}
		else if ((piece == 'N') || (piece == 'n')) {
			num_moves += genKnightMoves(pos,square,&moves[num_moves]);
		}
		else if ((piece == 'B') || (piece == 'b')) {
			num_moves += genBishopMoves(pos,square,&moves[num_moves]);
		}
		else if ((piece == 'R') || (piece == 'r')) {
			num_moves += genRookMoves(pos,square,&moves[num_moves]);
		}
		else if ((piece == 'Q') || (piece == 'q')) {
			num_moves += genQueenMoves(pos,square,&moves[num_moves]);
		}

		BBallpieces &= ~(1ULL << square);
	}
	return num_moves;
}