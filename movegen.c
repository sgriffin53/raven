#include <stdio.h>
#include <ctype.h>
#include "movegen.h"
#include "attacks.h"
#include "position.h"
#include "move.h"
#include "magicmoves.h"
#include "globals.h"
#include "bitboards.h"

const int BPdirs[2][2] = {{-1,-1},{+1,-1}};
const int WPdirs[2][2] = {{-1,+1},{+1,+1}};

int genKingMoves(struct position *pos, int square, struct move *moves, int forqsearch) {
	U64 BBattacks = 0ULL;
	int num_moves = 0;
	char piece = 'K';
	if (pos->tomove == BLACK) piece = 'k';
	if (pos->tomove == WHITE) {
		U64 BBking = (pos->BBkings & pos->BBwhitepieces);
		BBattacks = BBkingLookup[square];
		BBattacks = BBattacks & ~pos->BBwhitepieces;
		if (forqsearch) BBattacks = BBattacks & pos->BBblackpieces;
	}
	else if (pos->tomove == BLACK) {
		U64 BBking = (pos->BBkings & pos->BBblackpieces);
		BBattacks = BBkingLookup[square];
		BBattacks = BBattacks & ~pos->BBblackpieces;
		if (forqsearch) BBattacks = BBattacks & pos->BBwhitepieces;
	}
	//dspBB(BBattacks);
	while (BBattacks != 0) {
		int movesquare = __builtin_ctzll(BBattacks);
		BBattacks &= ~(1ULL << movesquare);
		char cappiece = getPiece(pos,movesquare);
		moves[num_moves].from = square;
		moves[num_moves].to = movesquare;
		moves[num_moves].prom = 0;
		moves[num_moves].cappiece = cappiece;
		moves[num_moves].piece = piece;
		num_moves++;
	}
	// castling moves
	// white castling
	if (!forqsearch) {
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
				moves[num_moves].cappiece = '0';
				moves[num_moves].piece = piece;
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
				moves[num_moves].cappiece = '0';
				moves[num_moves].piece = piece;
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
					if (!forqsearch) {
						moves[num_moves].from = E8;
						moves[num_moves].to = G8;
						moves[num_moves].prom = 0;
						moves[num_moves].cappiece = '0';
						moves[num_moves].piece = piece;
						num_moves += 1;
					}
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
				moves[num_moves].cappiece = '0';
				moves[num_moves].piece = piece;
				num_moves += 1;
			}
		}
	}
	return num_moves;
}
int genKnightMoves(struct position *pos, int square, struct move *moves, int forqsearch) {
	U64 BBattacks = 0ULL;
	int num_moves = 0;
	char piece = 'N';
	if (pos->tomove == BLACK) piece = 'n';
	if (pos->tomove == WHITE) {
		U64 BBknight = (1ULL << square);
		BBattacks = BBknightLookup[square];
		BBattacks = BBattacks & ~pos->BBwhitepieces;
		if (forqsearch) BBattacks = BBattacks & pos->BBblackpieces;
	}
	else if (pos->tomove == BLACK) {
		U64 BBknight = (1ULL << square);
		BBattacks = BBknightLookup[square];
		BBattacks = BBattacks & ~pos->BBblackpieces;
		if (forqsearch) BBattacks = BBattacks & pos->BBwhitepieces;
	}
	//dspBB(BBattacks);
	while (BBattacks != 0) {
		int movesquare = __builtin_ctzll(BBattacks);
		BBattacks &= BBattacks - 1;
		char cappiece = getPiece(pos,movesquare);
		moves[num_moves].from = square;
		moves[num_moves].to = movesquare;
		moves[num_moves].prom = 0;
		moves[num_moves].cappiece = cappiece;
		moves[num_moves].piece = piece;
		num_moves++;
	}
	return num_moves;
}
int genBishopMoves(struct position *pos, int square, struct move *moves, int forqsearch) {
	U64 BBattacks = 0ULL;
	int num_moves = 0;
	char piece = 'B';
	if (pos->tomove == BLACK) piece = 'b';
	if (pos->tomove == WHITE) {
		U64 BBoccupancy = (pos->BBwhitepieces | pos->BBblackpieces);
		BBattacks = Bmagic(square,BBoccupancy);
		BBattacks = BBattacks & ~pos->BBwhitepieces;
		if (forqsearch) BBattacks = BBattacks & pos->BBblackpieces;
	}
	else if (pos->tomove == BLACK) {
		U64 BBoccupancy = (pos->BBwhitepieces | pos->BBblackpieces);
		BBattacks = Bmagic(square,BBoccupancy);
		BBattacks = BBattacks & ~pos->BBblackpieces;
		if (forqsearch) BBattacks = BBattacks & pos->BBwhitepieces;
	}
	while (BBattacks != 0) {
		int movesquare = __builtin_ctzll(BBattacks);
		BBattacks &= BBattacks - 1;
		char cappiece = getPiece(pos,movesquare);
		moves[num_moves].from = square;
		moves[num_moves].to = movesquare;
		moves[num_moves].prom = 0;
		moves[num_moves].cappiece = cappiece;
		moves[num_moves].piece = piece;
		num_moves++;
	}
	return num_moves;
}
int genRookMoves(struct position *pos, int square, struct move *moves, int forqsearch) {
	U64 BBattacks = 0ULL;
	int num_moves = 0;
	char piece = 'R';
	if (pos->tomove == BLACK) piece = 'r';
	if (pos->tomove == WHITE) {
		U64 BBoccupancy = (pos->BBwhitepieces | pos->BBblackpieces);
		BBattacks = Rmagic(square,BBoccupancy);
		BBattacks = BBattacks & ~pos->BBwhitepieces;
		if (forqsearch) BBattacks = BBattacks & pos->BBblackpieces;
	}
	else if (pos->tomove == BLACK) {
		U64 BBoccupancy = (pos->BBwhitepieces | pos->BBblackpieces);
		BBattacks = Rmagic(square,BBoccupancy);
		BBattacks = BBattacks & ~pos->BBblackpieces;
		if (forqsearch) BBattacks = BBattacks & pos->BBwhitepieces;
	}
	while (BBattacks != 0) {
		int movesquare = __builtin_ctzll(BBattacks);
		BBattacks &= BBattacks - 1;
		char cappiece = getPiece(pos,movesquare);
		moves[num_moves].from = square;
		moves[num_moves].to = movesquare;
		moves[num_moves].prom = 0;
		moves[num_moves].cappiece = cappiece;
		moves[num_moves].piece = piece;
		num_moves++;
	}
	return num_moves;
}
int genQueenMoves(struct position *pos, int square, struct move *moves, int forqsearch) {
	U64 BBattacks = 0ULL;
	int num_moves = 0;
	char piece = 'Q';
	if (pos->tomove == BLACK) piece = 'q';
	if (pos->tomove == WHITE) {
		U64 BBoccupancy = (pos->BBwhitepieces | pos->BBblackpieces);
		BBattacks = Rmagic(square,BBoccupancy) | Bmagic(square,BBoccupancy);
		BBattacks = BBattacks & ~pos->BBwhitepieces;
		if (forqsearch) BBattacks = BBattacks & pos->BBblackpieces;
	}
	else if (pos->tomove == BLACK) {
		U64 BBoccupancy = (pos->BBwhitepieces | pos->BBblackpieces);
		BBattacks = Rmagic(square,BBoccupancy) | Bmagic(square,BBoccupancy);
		BBattacks = BBattacks & ~pos->BBblackpieces;
		if (forqsearch) BBattacks = BBattacks & pos->BBwhitepieces;
	}
	while (BBattacks != 0) {
		int movesquare = __builtin_ctzll(BBattacks);
		BBattacks &= BBattacks - 1;
		char cappiece = getPiece(pos,movesquare);
		moves[num_moves].from = square;
		moves[num_moves].to = movesquare;
		moves[num_moves].prom = 0;
		moves[num_moves].cappiece = cappiece;
		moves[num_moves].piece = piece;
		num_moves++;
	}
	return num_moves;
}
int genPawnMoves(struct position *pos, int square, struct move *moves, int forqsearch) {
	assert(pos);
	assert(moves);
	int num_moves = 0;
	U64 BBpawn = (1ULL << square);
	U64 BBmove;
	
	char piece = 'P';
	if (pos->tomove == BLACK) piece = 'p';
	// white pawns
	if (pos->tomove == WHITE) {
		// double pawn pushe
		if (getrank(square) == 1) {
			// generate double pawn moves for pawns on 2nd rank
			BBmove = BBpawnDoublePushW(BBpawn,~(pos->BBblackpieces | pos->BBwhitepieces));
			if (BBmove != 0) {
				int movesquare = __builtin_ctzll(BBmove);
				if (!forqsearch) {
					moves[num_moves].from = square;
					moves[num_moves].to = movesquare;
					moves[num_moves].prom = 0;
					moves[num_moves].cappiece = '0';
					moves[num_moves].piece = piece;
					num_moves++;
				}
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
					moves[num_moves].cappiece = '0';
					moves[num_moves].piece = piece;
					num_moves++;
					moves[num_moves].from = square;
					moves[num_moves].to = movesquare;
					moves[num_moves].prom = 'r';
					moves[num_moves].cappiece = '0';
					moves[num_moves].piece = piece;
					num_moves++;
					moves[num_moves].from = square;
					moves[num_moves].to = movesquare;
					moves[num_moves].prom = 'b';
					moves[num_moves].cappiece = '0';
					moves[num_moves].piece = piece;
					num_moves++;
					moves[num_moves].from = square;
					moves[num_moves].to = movesquare;
					moves[num_moves].prom = 'n';
					moves[num_moves].cappiece = '0';
					moves[num_moves].piece = piece;
					num_moves++;
			}
			else {
				if (!forqsearch) {
					moves[num_moves].from = square;
					moves[num_moves].to = movesquare;
					moves[num_moves].prom = 0;
					moves[num_moves].cappiece = '0';
					moves[num_moves].piece = piece;
					num_moves++;
				}
			}
		}
		// attacks
		BBmove = BBpawnEastAttacksW(BBpawn) | BBpawnWestAttacksW(BBpawn);
		BBmove = (BBmove & pos->BBblackpieces);
		while (BBmove != 0) {
			int movesquare = __builtin_ctzll(BBmove);
			if (getrank(movesquare) == 7) {
				char cappiece = getPiece(pos,movesquare);
				moves[num_moves].from = square;
				moves[num_moves].to = movesquare;
				moves[num_moves].prom = 'q';
				moves[num_moves].cappiece = cappiece;
				moves[num_moves].piece = piece;
				num_moves++;
				moves[num_moves].from = square;
				moves[num_moves].to = movesquare;
				moves[num_moves].prom = 'r';
				moves[num_moves].cappiece = cappiece;
				moves[num_moves].piece = piece;
				num_moves++;
				moves[num_moves].from = square;
				moves[num_moves].to = movesquare;
				moves[num_moves].prom = 'b';
				moves[num_moves].cappiece = cappiece;
				moves[num_moves].piece = piece;
				num_moves++;
				moves[num_moves].from = square;
				moves[num_moves].to = movesquare;
				moves[num_moves].prom = 'n';
				moves[num_moves].cappiece = cappiece;
				moves[num_moves].piece = piece;
				num_moves++;
			}
			else {
				char cappiece = getPiece(pos,movesquare);
				moves[num_moves].from = square;
				moves[num_moves].to = movesquare;
				moves[num_moves].prom = 0;
				moves[num_moves].cappiece = cappiece;
				moves[num_moves].piece = piece;
				num_moves++;
			}
			//BBmove &= ~(1ULL << movesquare);
			BBmove &= BBmove - 1;
		}
	}
	// black pawns
	else if (pos->tomove == BLACK) {
		if (getrank(square) == 6) {
			// generate double pawn moves for pawns on 7nd rank
			U64 BBmove = BBpawnDoublePushB(BBpawn,~(pos->BBblackpieces | pos->BBwhitepieces));
			if (BBmove != 0) {
				int movesquare = __builtin_ctzll(BBmove);
				if (!forqsearch) {
					moves[num_moves].from = square;
					moves[num_moves].to = movesquare;
					moves[num_moves].prom = 0;
					moves[num_moves].cappiece = '0';
					moves[num_moves].piece = piece;
					num_moves++;
				}
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
					moves[num_moves].cappiece = '0';
					moves[num_moves].piece = piece;
					num_moves++;
					moves[num_moves].from = square;
					moves[num_moves].to = movesquare;
					moves[num_moves].prom = 'r';
					moves[num_moves].cappiece = '0';
					moves[num_moves].piece = piece;
					num_moves++;
					moves[num_moves].from = square;
					moves[num_moves].to = movesquare;
					moves[num_moves].prom = 'b';
					moves[num_moves].cappiece = '0';
					moves[num_moves].piece = piece;
					num_moves++;
					moves[num_moves].from = square;
					moves[num_moves].to = movesquare;
					moves[num_moves].prom = 'n';
					moves[num_moves].cappiece = '0';
					moves[num_moves].piece = piece;
					num_moves++;
			}
			else {
				if (!forqsearch) {
					moves[num_moves].from = square;
					moves[num_moves].to = movesquare;
					moves[num_moves].prom = 0;
					moves[num_moves].cappiece = '0';
					moves[num_moves].piece = piece;
					num_moves++;
				}
			}
		}
		// attacks
		BBmove = BBpawnEastAttacksB(BBpawn) | BBpawnWestAttacksB(BBpawn);
		BBmove = (BBmove & pos->BBwhitepieces);
		while (BBmove != 0) {
			int movesquare = __builtin_ctzll(BBmove);
			if (getrank(movesquare) == 0) {
				char cappiece = getPiece(pos,movesquare);
				moves[num_moves].from = square;
				moves[num_moves].to = movesquare;
				moves[num_moves].prom = 'q';
				moves[num_moves].cappiece = cappiece;
				moves[num_moves].piece = piece;
				num_moves++;
				moves[num_moves].from = square;
				moves[num_moves].to = movesquare;
				moves[num_moves].prom = 'r';
				moves[num_moves].cappiece = cappiece;
				moves[num_moves].piece = piece;
				num_moves++;
				moves[num_moves].from = square;
				moves[num_moves].to = movesquare;
				moves[num_moves].prom = 'b';
				moves[num_moves].cappiece = cappiece;
				moves[num_moves].piece = piece;
				num_moves++;
				moves[num_moves].from = square;
				moves[num_moves].to = movesquare;
				moves[num_moves].prom = 'n';
				moves[num_moves].cappiece = cappiece;
				moves[num_moves].piece = piece;
				num_moves++;
			}
			else {
				char cappiece = getPiece(pos,movesquare);
				moves[num_moves].from = square;
				moves[num_moves].to = movesquare;
				moves[num_moves].prom = 0;
				moves[num_moves].cappiece = cappiece;
				moves[num_moves].piece = piece;
				num_moves++;
			}
			BBmove &= BBmove - 1;
		}
	}
	return num_moves;
}
int genMoves(struct position *pos, struct move *moves, int forqsearch) {
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
					if (!forqsearch) {
						moves[num_moves].from = idx;
						moves[num_moves].to = pos->epsquare;
						moves[num_moves].prom = 0;
						moves[num_moves].cappiece = '0';
						moves[num_moves].piece = piece;
						num_moves++;
					}
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
					if (!forqsearch) {
						moves[num_moves].from = idx;
						moves[num_moves].to = pos->epsquare;
						moves[num_moves].prom = 0;
						moves[num_moves].cappiece = '0';
						moves[num_moves].piece = piece;
						num_moves++;
					}
				}
			}
		}
	}
	
	
	while (BBallpieces != 0) {
		int square = __builtin_ctzll(BBallpieces);
		char piece = getPiece(pos,square);
		
		switch (piece) {
			case 'k':
			case 'K': num_moves += genKingMoves(pos,square,&moves[num_moves], forqsearch); break;
			case 'P':
			case 'p': num_moves += genPawnMoves(pos,square,&moves[num_moves], forqsearch); break;
			case 'N':
			case 'n': num_moves += genKnightMoves(pos,square,&moves[num_moves], forqsearch); break;
			case 'B':
			case 'b': num_moves += genBishopMoves(pos,square,&moves[num_moves], forqsearch); break;
			case 'R':
			case 'r': num_moves += genRookMoves(pos,square,&moves[num_moves], forqsearch); break;
			case 'Q':
			case 'q': num_moves += genQueenMoves(pos,square,&moves[num_moves], forqsearch); break;
			
		}
		BBallpieces &= BBallpieces - 1;
	}
	return num_moves;
}