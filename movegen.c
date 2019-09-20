#include <stdio.h>
#include <ctype.h>
#include "movegen.h"
#include "attacks.h"
#include "position.h"
#include "move.h"
#include "magicmoves.h"
#include "globals.h"
#include "bitboards.h"
#include "makemove.h"
#include "misc.h"

const int BPdirs[2][2] = {{-1,-1},{+1,-1}};
const int WPdirs[2][2] = {{-1,+1},{+1,+1}};

const U64 pawnAttacksLookup[2][64] = {
		{0, 0, 0, 0, 0, 0, 0, 0, 0x2, 0x5, 0xa, 0x14, 0x28, 0x50, 0xa0, 0x40, 0x200, 0x500, 0xa00, 0x1400, 0x2800, 0x5000, 0xa000, 0x4000, 0x20000, 0x50000, 0xa0000, 0x140000, 0x280000, 0x500000, 0xa00000, 0x400000, 0x2000000, 0x5000000, 0xa000000, 0x14000000, 0x28000000, 0x50000000, 0xa0000000, 0x40000000, 0x200000000, 0x500000000, 0xa00000000, 0x1400000000, 0x2800000000, 0x5000000000, 0xa000000000, 0x4000000000, 0x20000000000, 0x50000000000, 0xa0000000000, 0x140000000000, 0x280000000000, 0x500000000000, 0xa00000000000, 0x400000000000, 0x2000000000000, 0x5000000000000, 0xa000000000000, 0x14000000000000, 0x28000000000000, 0x50000000000000, 0xa0000000000000, 0x40000000000000},
		{0x200, 0x500, 0xa00, 0x1400, 0x2800, 0x5000, 0xa000, 0x4000, 0x20000, 0x50000, 0xa0000, 0x140000, 0x280000, 0x500000, 0xa00000, 0x400000, 0x2000000, 0x5000000, 0xa000000, 0x14000000, 0x28000000, 0x50000000, 0xa0000000, 0x40000000, 0x200000000, 0x500000000, 0xa00000000, 0x1400000000, 0x2800000000, 0x5000000000, 0xa000000000, 0x4000000000, 0x20000000000, 0x50000000000, 0xa0000000000, 0x140000000000, 0x280000000000, 0x500000000000, 0xa00000000000, 0x400000000000, 0x2000000000000, 0x5000000000000, 0xa000000000000, 0x14000000000000, 0x28000000000000, 0x50000000000000, 0xa0000000000000, 0x40000000000000, 0x200000000000000, 0x500000000000000, 0xa00000000000000, 0x1400000000000000, 0x2800000000000000, 0x5000000000000000, 0xa000000000000000, 0x4000000000000000, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}
	};

// pseudo legal movegen

int genKingMoves(struct position *pos, int square, struct move *moves, int forqsearch) {
	U64 BBattacks = 0ULL;
	int num_moves = 0;
	char piece = 'K';
	if (pos->tomove == BLACK) piece = 'k';
	if (pos->tomove == WHITE) {
		U64 BBking = (pos->BBkings & pos->BBwhitepieces);
		//BBattacks = BBkingattacks(BBking);
		BBattacks = BBkingLookup[square];
		BBattacks = BBattacks & ~pos->BBwhitepieces;
	}
	else if (pos->tomove == BLACK) {
		U64 BBking = (pos->BBkings & pos->BBblackpieces);
		//BBattacks = BBkingattacks(BBking);
		BBattacks = BBkingLookup[square];
		BBattacks = BBattacks & ~pos->BBblackpieces;
	}
	//dspBB(BBattacks);
	while (BBattacks != 0) {
		int movesquare = __builtin_ctzll(BBattacks);
		BBattacks &= ~(1ULL << movesquare);
		char cappiece = getPiece(pos,movesquare);
		if (!forqsearch || (forqsearch && cappiece != '0')) {
			moves[num_moves].from = square;
			moves[num_moves].to = movesquare;
			moves[num_moves].prom = 0;
			moves[num_moves].cappiece = cappiece;
			moves[num_moves].piece = piece;
			num_moves++;
		}
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
				if (!forqsearch) {
					moves[num_moves].from = E1;
					moves[num_moves].to = G1;
					moves[num_moves].prom = 0;
					moves[num_moves].cappiece = '0';
					moves[num_moves].piece = piece;
					num_moves += 1;
				}
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
				if (!forqsearch) {
					moves[num_moves].from = E1;
					moves[num_moves].to = C1;
					moves[num_moves].prom = 0;
					moves[num_moves].cappiece = '0';
					moves[num_moves].piece = piece;
					num_moves += 1;
				}
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
				if (!forqsearch) {
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
		//BBattacks = BBknightattacks(BBknight);
		BBattacks = BBknightLookup[square];
		BBattacks = BBattacks & ~pos->BBwhitepieces;
	}
	else if (pos->tomove == BLACK) {
		U64 BBknight = (1ULL << square);
		//BBattacks = BBknightattacks(BBknight);
		BBattacks = BBknightLookup[square];
		BBattacks = BBattacks & ~pos->BBblackpieces;
	}
	//dspBB(BBattacks);
	while (BBattacks != 0) {
		int movesquare = __builtin_ctzll(BBattacks);
		//BBattacks &= ~(1ULL << movesquare);
		BBattacks &= BBattacks - 1;
		char cappiece = getPiece(pos,movesquare);
		if (!forqsearch || (forqsearch && cappiece != '0')) {
			moves[num_moves].from = square;
			moves[num_moves].to = movesquare;
			moves[num_moves].prom = 0;
			moves[num_moves].cappiece = cappiece;
			moves[num_moves].piece = piece;
			num_moves++;
		}
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
	}
	else if (pos->tomove == BLACK) {
		U64 BBoccupancy = (pos->BBwhitepieces | pos->BBblackpieces);
		BBattacks = Bmagic(square,BBoccupancy);
		BBattacks = BBattacks & ~pos->BBblackpieces;
	}
	while (BBattacks != 0) {
		int movesquare = __builtin_ctzll(BBattacks);
		//BBattacks &= ~(1ULL << movesquare);
		BBattacks &= BBattacks - 1;
		char cappiece = getPiece(pos,movesquare);
		if (!forqsearch || (forqsearch && cappiece != '0')) {
			moves[num_moves].from = square;
			moves[num_moves].to = movesquare;
			moves[num_moves].prom = 0;
			moves[num_moves].cappiece = cappiece;
			moves[num_moves].piece = piece;
			num_moves++;
		}
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
	}
	else if (pos->tomove == BLACK) {
		U64 BBoccupancy = (pos->BBwhitepieces | pos->BBblackpieces);
		BBattacks = Rmagic(square,BBoccupancy);
		BBattacks = BBattacks & ~pos->BBblackpieces;
	}
	while (BBattacks != 0) {
		int movesquare = __builtin_ctzll(BBattacks);
		//BBattacks &= ~(1ULL << movesquare);
		BBattacks &= BBattacks - 1;
		char cappiece = getPiece(pos,movesquare);
		if (!forqsearch || (forqsearch && cappiece != '0')) {
			moves[num_moves].from = square;
			moves[num_moves].to = movesquare;
			moves[num_moves].prom = 0;
			moves[num_moves].cappiece = cappiece;
			moves[num_moves].piece = piece;
			num_moves++;
		}
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
	}
	else if (pos->tomove == BLACK) {
		U64 BBoccupancy = (pos->BBwhitepieces | pos->BBblackpieces);
		BBattacks = Rmagic(square,BBoccupancy) | Bmagic(square,BBoccupancy);
		BBattacks = BBattacks & ~pos->BBblackpieces;
	}
	while (BBattacks != 0) {
		int movesquare = __builtin_ctzll(BBattacks);
		//BBattacks &= ~(1ULL << movesquare);
		BBattacks &= BBattacks - 1;
		char cappiece = getPiece(pos,movesquare);
		if (!forqsearch || (forqsearch && cappiece != '0')) {
			moves[num_moves].from = square;
			moves[num_moves].to = movesquare;
			moves[num_moves].prom = 0;
			moves[num_moves].cappiece = cappiece;
			moves[num_moves].piece = piece;
			num_moves++;
		}
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
			//BBmove &= ~(1ULL << movesquare);
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
		 
		/*
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
		*/
		//BBallpieces &= ~(1ULL << square);
		BBallpieces &= BBallpieces - 1;
	}
	return num_moves;
}

// legal move gen

int legalMoves(struct position *pos, struct move *moves) {
	U64 BBmyking;
	
	
	if (pos->tomove == WHITE) BBmyking = pos->BBwhitepieces & pos->BBkings;
	else BBmyking = pos->BBblackpieces & pos->BBkings;
	
	struct move *ptr = moves;
	
	const U64 attacked = attackedSquares(pos);
	const U64 pinned = pinnedPieces(pos);
	
	const U64 check = attacked & BBmyking;
	
	U64 checkAttack = NO_CHECK;
	
	if (check) {
		if (numberOfChecks(pos) == 1) {
			checkAttack = checkingAttack(pos);
		}
		else {
			kingMoves(pos, &ptr, attacked, check);
			return ptr - moves;
		}
	}
	pawnMoves(pos, &ptr, checkAttack, pinned);
	knightMoves(pos, &ptr, checkAttack, pinned);
	
	slidingMoves(pos, &ptr, BISHOP, pos->tomove, BBbishopAttacks, checkAttack, pinned);
	slidingMoves(pos, &ptr, ROOK, pos->tomove, BBrookAttacks, checkAttack, pinned);
	slidingMoves(pos, &ptr, QUEEN, pos->tomove, BBqueenAttacks, checkAttack, pinned);
	
	kingMoves(pos, &ptr, attacked, check);
	return ptr - moves;
}
void slidingMoves(struct position *pos, struct move **moves, const int piece, const int side, U64 (*movesFunc)(int, U64), const U64 checkAttacks, const U64 pinned) {
	const int oppside = 1 ^ side;
	char mypiece;
	U64 BBmypiece, opppieces, empty, occupied;
	empty = ~(pos->BBwhitepieces | pos->BBblackpieces);
	occupied = ~empty;
	int kingpos;
	if (side == WHITE) {
		opppieces = pos->BBblackpieces;
		if (piece == BISHOP) {
			mypiece = 'B';
			BBmypiece = pos->BBwhitepieces & pos->BBbishops;
		}
		else if (piece == ROOK) {
			mypiece = 'R';
			BBmypiece = pos->BBwhitepieces & pos->BBrooks;
		}
		else if (piece == QUEEN) {
			mypiece = 'Q';
			BBmypiece = pos->BBwhitepieces & pos->BBqueens;
		}
		kingpos = pos->Wkingpos;
	}
	else  {
		opppieces = pos->BBwhitepieces;
		if (piece == BISHOP) {
			mypiece = 'b';
			BBmypiece = pos->BBblackpieces & pos->BBbishops;
		}
		else if (piece == ROOK) {
			mypiece = 'r';
			BBmypiece = pos->BBblackpieces & pos->BBrooks;
		}
		else if (piece == QUEEN) {
			mypiece = 'q';
			BBmypiece = pos->BBblackpieces & pos->BBqueens;
		}
		kingpos = pos->Bkingpos;
	}
	U64 pinnedSliders = BBmypiece & pinned;
	U64 bb = BBmypiece ^ pinnedSliders;
	while (bb) {
		const int from = __builtin_ctzll(bb);
		bb &= bb - 1;
		const U64 movesBB = movesFunc(from, occupied) & checkAttacks;
		
		saveMoves(pos, moves, mypiece, movesBB, from, side, opppieces);
		saveMoves(pos, moves, mypiece, movesBB, from, side, empty);
	}
	while (checkAttacks == NO_CHECK && pinnedSliders) {
		const int from = __builtin_ctzll(pinnedSliders);
		pinnedSliders &= pinnedSliders - 1;
		const U64 movesBB = movesFunc(from, occupied) & line(from, kingpos);
		
		// If the piece is pinned it can only possibly capture the pinning piece.
		U64 attacker = movesBB & opppieces;
		
		if (attacker) {
			struct move addmove;
			addmove.from = from;
			addmove.to = __builtin_ctzll(attacker);
			addmove.piece = mypiece;
			addmove.cappiece = getPiece(pos, addmove.to);
			addmove.prom = 0;
			**moves = addmove;
			(*moves)++;
		}
		saveMoves(pos, moves, mypiece, movesBB, from, side, empty);
	}
}
void knightMoves(struct position *pos, struct move **moves, U64 checkAttacks, U64 pinned) {
	// Knights are always absolutely pinned, so their moves don't have to be considered.
	U64 myknights, empty, opppieces;
	char piece;
	empty = ~(pos->BBwhitepieces | pos->BBblackpieces);
	if (pos->tomove == WHITE) {
		myknights = pos->BBwhitepieces & pos->BBknights;
		opppieces = pos->BBblackpieces;
		piece = 'N';
	}
	else {
		myknights = pos->BBblackpieces & pos->BBknights;
		opppieces = pos->BBwhitepieces;
		piece = 'n';
	}
	U64 bb = myknights & ~pinned;
	while (bb) {
		const int from = __builtin_ctzll(bb);
		bb &= bb - 1;
		
		const U64 movesBB = BBknightLookup[from] & checkAttacks;
		
		saveMoves(pos, moves, piece, movesBB, from, pos->tomove, opppieces);
		saveMoves(pos, moves, piece, movesBB, from, pos->tomove, empty);
	}
}
void pawnMoves(struct position *pos, struct move **moves, U64 checkAttack, U64 pinned) {
	U64 opppieces;
	U64 mypawns;
	int kingpos;
	char piece;
	if (pos->tomove == WHITE) {
		piece = 'P';
		kingpos = pos->Wkingpos;
		mypawns = pos->BBwhitepieces & pos->BBpawns;
		opppieces = pos->BBblackpieces;
	}
	else {
		piece = 'p';
		kingpos = pos->Bkingpos;
		mypawns = pos->BBblackpieces & pos->BBpawns;
		opppieces = pos->BBwhitepieces;
	}
	U64 pinnedPawns = mypawns & pinned;
	const U64 bb = mypawns ^ pinnedPawns;
	if (pos->epsquare != -1) {
		U64 attackers = pawnAttacksLookup[!pos->tomove][pos->epsquare] & mypawns;
		while (attackers) {
			const int from = __builtin_ctzll(attackers);
			attackers &= attackers - 1;
			struct move addmove;
			addmove.from = from;
			addmove.to = pos->epsquare;
			addmove.piece = piece;
			addmove.prom = 0;
			addmove.cappiece = '0';
			makeMove(&addmove, pos);
			pos->tomove = !pos->tomove;
			if (!isCheck(pos)) {
				**moves = addmove;
				(*moves)++;
			}
			pos->tomove = !pos->tomove;
			unmakeMove(pos);
		}
	}
	if (pos->tomove == WHITE) {
		addPawnMoves(pos, moves, wCaptRightPawn(bb, opppieces) & checkAttack, WHITE, 9);
		addPawnMoves(pos, moves, wCaptLeftPawn(bb, opppieces) & checkAttack, WHITE, 7);
		
		wPawnPushMoves(pos, moves, bb, ~(pos->BBblackpieces | pos->BBwhitepieces), checkAttack);
		if (checkAttack == NO_CHECK) {
			wPinnedPawnsMoves(pos, moves, pinnedPawns, opppieces);
		}
	}
	else {
		addPawnMoves(pos, moves, bCaptRightPawn(bb, opppieces) & checkAttack, BLACK, -7);
		addPawnMoves(pos, moves, bCaptLeftPawn(bb, opppieces) & checkAttack, BLACK, -9);
		
		bPawnPushMoves(pos, moves, bb, ~(pos->BBblackpieces | pos->BBwhitepieces), checkAttack);
		if (checkAttack == NO_CHECK) {
			bPinnedPawnsMoves(pos, moves, pinnedPawns, opppieces);
		}
	}
}
void wPinnedPawnsMoves(struct position *pos, struct move **moves, U64 pinnedPawns, const U64 opPieces) {
	int kingpos = pos->Wkingpos;

	while (pinnedPawns) {
		const int pawn = __builtin_ctzll(pinnedPawns);
		pinnedPawns &= pinnedPawns - 1;
		// Pawns that are pinned horizontally can't move
		switch (typeOfPin(kingpos, pawn)) {
		case VERTICAL:
			wPawnPushMoves(pos, moves, bitmask[pawn], ~(pos->BBblackpieces | pos->BBwhitepieces), NO_CHECK);
			break;
		case DIAGRIGHT:
			addPawnMoves(pos, moves, wCaptRightPawn(bitmask[pawn], opPieces), WHITE, 9);
			break;
		case DIAGLEFT:
			addPawnMoves(pos, moves, wCaptLeftPawn(bitmask[pawn], opPieces), WHITE, 7);
			break;
		}
	}
}
void bPinnedPawnsMoves(struct position *pos, struct move **moves, U64 pinnedPawns, const U64 opPieces) {
	int kingpos = pos->Bkingpos;

	while (pinnedPawns) {
		const int pawn = __builtin_ctzll(pinnedPawns);
		pinnedPawns &= pinnedPawns - 1;
		// Pawns that are pinned horizontally can't move
		switch (typeOfPin(kingpos, pawn)) {
		case VERTICAL:
			bPawnPushMoves(pos, moves, bitmask[pawn], ~(pos->BBblackpieces | pos->BBwhitepieces), NO_CHECK);
			break;
		case DIAGRIGHT:
			addPawnMoves(pos, moves, bCaptLeftPawn(bitmask[pawn], opPieces), BLACK, -9);
			break;
		case DIAGLEFT:
			addPawnMoves(pos, moves, bCaptRightPawn(bitmask[pawn], opPieces), BLACK, -7);
			break;
		}
	}
}
void wPawnPushMoves(struct position *pos, struct move **moves, const U64 bb, const U64 empty, const U64 checkAttack) {
	const U64 singlePush = wSinglePushPawn(bb, empty);
	U64 doublePush = wDoublePushPawn(singlePush, empty) & checkAttack;
	
	addPawnMoves(pos, moves, singlePush & checkAttack, WHITE, 8);
	while (doublePush) {
		const int to = __builtin_ctzll(doublePush);
		doublePush &= doublePush - 1;
		struct move addmove;
		addmove.to = to;
		addmove.from = to - 16;
		addmove.piece = 'P';
		addmove.prom = 0;
		addmove.cappiece = '0';
		**moves = addmove;
		(*moves)++;
	}
}
void bPawnPushMoves(struct position *pos, struct move **moves, const U64 bb, const U64 empty, const U64 checkAttack) {
	const U64 singlePush = bSinglePushPawn(bb, empty);
	U64 doublePush = bDoublePushPawn(singlePush, empty) & checkAttack;
	
	addPawnMoves(pos, moves, singlePush & checkAttack, BLACK, -8);
	
	while (doublePush) {
		const int to = __builtin_ctzll(doublePush);
		doublePush &= doublePush - 1;
		struct move addmove;
		addmove.to = to;
		addmove.from = to + 16;
		addmove.piece = 'p';
		addmove.prom = 0;
		addmove.cappiece = '0';
		**moves = addmove;
		(*moves)++;
	}
}
U64 wSinglePushPawn(const U64 bb, const U64 empty) { return northOne(bb) & empty; }
U64 bSinglePushPawn(const U64 bb, const U64 empty) { return southOne(bb) & empty; }

U64 wDoublePushPawn(const U64 bb, const U64 empty) {
	static const U64 rank4 = 0x00000000FF000000;
	return wSinglePushPawn(bb, empty) & rank4;
}
U64 bDoublePushPawn(const U64 bb, const U64 empty) {
	static const U64 rank5 = 0x000000FF00000000;
	return bSinglePushPawn(bb, empty) & rank5;
}
void addPawnMoves(struct position *pos, struct move **moves, U64 bb, const int side, const int shift) {
	static const U64 rank1AndRank8 = 0xff000000000000ff;
	
	char piece;
	
	if (side == WHITE) piece = 'P';
	else piece = 'p';
	
	// Splits the bitboard into promoting pawns and non-promoting pawns
	U64 promoting = bb & rank1AndRank8;
	bb ^= promoting;
	
	while (bb) {
		const int to = __builtin_ctzll(bb);
		bb &= bb - 1;
		char cappiece = getPiece(pos, to);
		struct move addmove;
		addmove.from = to - shift;
		addmove.to = to;
		addmove.prom = 0;
		addmove.cappiece = cappiece;
		addmove.piece = piece;
		**moves = addmove;
		(*moves)++;
	}
	while (promoting) {
		const int to = __builtin_ctzll(promoting);
		promoting &= promoting - 1;
		const int from = to - shift;
		char cappiece = getPiece(pos, to);
		struct move addmove;
		addmove.from = to - shift;
		addmove.to = to;
		addmove.prom = 'q';
		addmove.cappiece = cappiece;
		addmove.piece = piece;
		**moves = addmove;
		(*moves)++;
		addmove.prom = 'r';
		**moves = addmove;
		(*moves)++;
		addmove.prom = 'b';
		**moves = addmove;
		(*moves)++;
		addmove.prom = 'n';
		**moves = addmove;
		(*moves)++;
	}
}
void saveMoves(struct position *pos, struct move **moves, const char piece, const U64 movesBB, const int from, const int side, const U64 toBB) {
	U64 bb = movesBB & toBB;

	while (bb) {
		const int to = __builtin_ctzll(bb);
		bb &= bb - 1;
		char cappiece = getPiece(pos, to);
		**moves = (struct move){.from=from, .to=to, .piece=piece, .prom=0, .cappiece=cappiece};
		(*moves)++;
	} 
}
void kingMoves(struct position *pos, struct move **moves, const U64 attacked, const U64 check) {
	char piece;
	char kingpos;
	U64 opppieces;
	if (pos->tomove == WHITE) {
		opppieces = pos->BBblackpieces;
		kingpos = pos->Wkingpos;
		piece = 'K';
	}
	else {
		piece = 'k';
		kingpos = pos->Bkingpos;
		opppieces = pos->BBwhitepieces;
	}
	
	const int from = kingpos;
	const U64 movesBB = BBkingLookup[from] & ~attacked;

	saveMoves(pos, moves, piece, movesBB, from, pos->tomove, opppieces);
	saveMoves(pos, moves, piece, movesBB, from, pos->tomove, ~(pos->BBwhitepieces | pos->BBblackpieces));
	
	if (pos->tomove == WHITE) {
		// King side castling
		if ((pos->WcastleKS == 1) &&
			getPiece(pos,F1) == '0' &&
			getPiece(pos,G1) == '0' &&
			!isAttacked(pos, E1, BLACK) &&
			!isAttacked(pos, F1, BLACK) &&
			!isAttacked(pos, G1, BLACK)) {
				// Add move
				struct move addmove;
				addmove.from = E1;
				addmove.to = G1;
				addmove.prom = 0;
				addmove.cappiece = '0';
				addmove.piece = piece;
				**moves = addmove;
				(*moves)++;
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
				struct move addmove;
				addmove.from = E1;
				addmove.to = C1;
				addmove.prom = 0;
				addmove.cappiece = '0';
				addmove.piece = piece;
				**moves = addmove;
				(*moves)++;
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
				struct move addmove;
				addmove.from = E8;
				addmove.to = G8;
				addmove.prom = 0;
				addmove.cappiece = '0';
				addmove.piece = piece;
				**moves = addmove;
				(*moves)++;
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
				struct move addmove;
				addmove.from = E8;
				addmove.to = C8;
				addmove.prom = 0;
				addmove.cappiece = '0';
				addmove.piece = piece;
				**moves = addmove;
				(*moves)++;
		}
	}
}
U64 numberOfChecks(struct position *pos) {
	int kingpos;
	U64 opppawns, oppbishops, opprooks, oppqueens, oppkings, oppknights;
	if (pos->tomove == WHITE) {
		opppawns = pos->BBblackpieces & pos->BBpawns;
		oppbishops = pos->BBblackpieces & pos->BBbishops;
		opprooks = pos->BBblackpieces & pos->BBrooks;
		oppqueens = pos->BBblackpieces & pos->BBqueens;
		oppkings = pos->BBblackpieces & pos->BBkings;
		oppknights = pos->BBblackpieces & pos->BBknights;
		kingpos = pos->Wkingpos;
	}
	else {
		opppawns = pos->BBwhitepieces & pos->BBpawns;
		oppbishops = pos->BBwhitepieces & pos->BBbishops;
		opprooks = pos->BBwhitepieces & pos->BBrooks;
		oppqueens = pos->BBwhitepieces & pos->BBqueens;
		oppkings = pos->BBwhitepieces & pos->BBkings;
		oppknights = pos->BBwhitepieces & pos->BBknights;
		kingpos = pos->Bkingpos;
	}
	
	U64 checks = 0;
	
	U64 BBoccupied = pos->BBwhitepieces | pos->BBblackpieces;
	
	checks |= pawnAttacksLookup[pos->tomove][kingpos] & opppawns;
	checks |= BBbishopAttacks(kingpos, BBoccupied) & (oppqueens | oppbishops);
	checks |= BBrookAttacks(kingpos, BBoccupied) & (oppqueens | opprooks);
	checks |= BBknightLookup[kingpos] & oppknights;
	checks |= BBkingLookup[kingpos] & oppkings;
	
	return __builtin_popcountll(checks);
}
U64 checkingAttack(struct position *pos) {
	int kingpos;
	U64 opppawns, oppbishops, opprooks, oppqueens, oppkings, oppknights;
	if (pos->tomove == WHITE) {
		opppawns = pos->BBblackpieces & pos->BBpawns;
		oppbishops = pos->BBblackpieces & pos->BBbishops;
		opprooks = pos->BBblackpieces & pos->BBrooks;
		oppqueens = pos->BBblackpieces & pos->BBqueens;
		oppkings = pos->BBblackpieces & pos->BBkings;
		oppknights = pos->BBblackpieces & pos->BBknights;
		kingpos = pos->Wkingpos;
	}
	else {
		opppawns = pos->BBwhitepieces & pos->BBpawns;
		oppbishops = pos->BBwhitepieces & pos->BBbishops;
		opprooks = pos->BBwhitepieces & pos->BBrooks;
		oppqueens = pos->BBwhitepieces & pos->BBqueens;
		oppkings = pos->BBwhitepieces & pos->BBkings;
		oppknights = pos->BBwhitepieces & pos->BBknights;
		kingpos = pos->Bkingpos;
	}
	U64 attacks = 0;
	U64 BBoccupied = pos->BBwhitepieces | pos->BBblackpieces;
	
	attacks |= pawnAttacksLookup[pos->tomove][kingpos] & opppawns;
	attacks |= BBknightLookup[kingpos] & oppknights;
	
	U64 checkingSliders = 
							(BBbishopAttacks(kingpos, BBoccupied) & (oppqueens | oppbishops)) |
							(BBrookAttacks(kingpos, BBoccupied) & (oppqueens | opprooks));
	
	attacks |= checkingSliders;
	
	while (checkingSliders) {
		int attacker = __builtin_ctzll(checkingSliders);
		checkingSliders &= checkingSliders - 1;
		attacks |= BBinbetweenLookup[kingpos][attacker];
	}
	return attacks;
}
U64 attackedSquares(struct position *pos) {
	U64 BBoppbishops, BBopprooks, BBoppqueens;
	if (pos->tomove == WHITE) {
		pos->BBwhitepieces ^= (1ULL << pos->Wkingpos);
		BBoppbishops = pos->BBblackpieces & pos->BBbishops;
		BBopprooks = pos->BBblackpieces & pos->BBrooks;
		BBoppqueens = pos->BBblackpieces & pos->BBqueens;
		
	}
	else {
		pos->BBblackpieces ^= (1ULL << pos->Bkingpos);
		BBoppbishops = pos->BBwhitepieces & pos->BBbishops;
		BBopprooks = pos->BBwhitepieces & pos->BBrooks;
		BBoppqueens = pos->BBwhitepieces & pos->BBqueens;
	}
	
	U64 BBattacked = BBpawnattacks(pos, !pos->tomove) |
					BBallknightattacks(pos, !pos->tomove) |
					BBallkingattacks(pos, !pos->tomove) |
					BBslidingAttacks(pos, BBbishopAttacks, BBoppbishops) |
					BBslidingAttacks(pos, BBrookAttacks, BBopprooks) |
					BBslidingAttacks(pos, BBqueenAttacks, BBoppqueens);
					
	if (pos->tomove == WHITE) {
		pos->BBwhitepieces ^= (1ULL << pos->Wkingpos);
	}
	else {
		pos->BBblackpieces ^= (1ULL << pos->Bkingpos);
	}
	return BBattacked;
}
int typeOfPin(const int a, const int b) {

	const U64 inBetween = BBinbetweenLookup[a][b] | bitmask[a] | bitmask[b];
	const int c = min(a, b);

	if ((bitmask[c] << 1) & inBetween)
		return HORIZONTAL;

	if ((bitmask[c] << 8) & inBetween)
		return VERTICAL;

	if ((bitmask[c] << 9) & inBetween)
		return DIAGRIGHT;

	if ((bitmask[c] << 7) & inBetween)
		return DIAGLEFT;

	return NONE;
}
U64 pinnedPieces(struct position *pos) {
	U64 kingpos;
	U64 BBoppBQ, BBoppRQ;
	U64 BBpossiblePinned;
	if (pos->tomove == WHITE) {
		kingpos = pos->Wkingpos;
		BBoppBQ = pos->BBblackpieces & (pos->BBqueens | pos->BBbishops);
		BBoppRQ = pos->BBblackpieces & (pos->BBqueens | pos->BBrooks);
		BBpossiblePinned = pos->BBwhitepieces;
	}
	else {
		kingpos = pos->Bkingpos;
		BBoppBQ = pos->BBwhitepieces & (pos->BBqueens | pos->BBbishops);
		BBoppRQ = pos->BBwhitepieces & (pos->BBqueens | pos->BBrooks);
		BBpossiblePinned = pos->BBblackpieces;
	}
	U64 BBoccupied = pos->BBwhitepieces | pos->BBblackpieces;
	
	U64 pinners = (xrayBishopAttacks(kingpos, BBoccupied, BBpossiblePinned) &
			  	  	   BBoppBQ) |

			  	  	   (xrayRookAttacks(kingpos, BBoccupied, BBpossiblePinned) &
			  	  	   BBoppRQ);
	
	U64 pinned = 0;
	
	while (pinners) {
		int square = __builtin_ctzll(pinners);
		pinners &= pinners - 1;
		pinned |= BBinbetweenLookup[kingpos][square] & BBpossiblePinned;
	}
	return pinned;
}

U64 xrayBishopAttacks(const int sqr, const U64 occupied, const U64 myPieces) {
	const U64 attacks = Bmagic(sqr, occupied);
	const U64 blockers = myPieces & attacks;

	return attacks ^ Bmagic(sqr, occupied ^ blockers);
}

U64 xrayRookAttacks(const int sqr, const U64 occupied, const U64 myPieces) {
	const U64 attacks = Rmagic(sqr, occupied);
	const U64 blockers = myPieces & attacks;

	return attacks ^ Rmagic(sqr, occupied ^ blockers);
}
U64 line(const int a, const int b) {


	const int aFile = getfile(a), bFile = getfile(b);

	if (aFile == bFile)
		return rayLookup[NORT][a] | rayLookup[SOUT][a] | bitmask[a];

	const int aRank = getrank(a), bRank = getrank(b);

	if (aRank == bRank)
		return rayLookup[EAST][a] | rayLookup[WEST][a] | bitmask[a];

	// Right diagonal
	if (aRank - aFile == bRank - bFile)
	   return rayLookup[NOEA][a] | rayLookup[SOWE][a] | bitmask[a];

	// Left diagonal
	if (aRank + aFile == bRank + bFile)
		return rayLookup[NOWE][a] | rayLookup[SOEA][a] | bitmask[a];

	return 0;
}


int isEnPassant(struct position *pos, struct move *move) {
	if (pos->epsquare == -1) return 0;
	if (pos->tomove == BLACK && move->piece != 'p') return 0;
	else if (pos->tomove == WHITE && move->piece != 'P') return 0;
	if (move->to == pos->epsquare) return 1;
	return 0;
}
int isCastling(struct position *pos, struct move *move) {
	if (pos->tomove == BLACK && move->piece != 'k') return 0;
	else if (pos->tomove == WHITE && move->piece != 'K') return 0;
	if (pos->tomove == BLACK) {
		if (move->from == E8) {
			if (move->to == C8) return 1;
			else if (move->to == G8) return 1;
		}
	}
	else if (pos->tomove == WHITE) {
		if (move->from == E1) {
			if (move->to == C1) return 1;
			else if (move->to == G1) return 1;
		}
	}
	return 0;
}
int moveCanBeIllegal(struct position *pos, struct move *move) {
	int kingpos;
	if (pos->tomove == WHITE) {
		kingpos = pos->Wkingpos;
	}
	else {
		kingpos = pos->Bkingpos;
	}
	if (move->piece == 'k' || move->piece == 'K') return 1; // king moves
	if (isEnPassant(pos, move)) return 1; // en passant
	if (isCastling(pos, move)) return 1;
	if (isCheck(pos)) {
		struct move lastmove = movestack[movestackend - 2]; // assume this is after move has been played
		//if (lastmove.piece == 'p' || lastmove.piece == 'P' || lastmove.piece == 'N' || lastmove.piece == 'n') {
			if (move->to == lastmove.to) {
				return 1; // capture of checking knight/pawn
			}
		//}
		if ((1ULL << move->to) & checkingAttack(pos)) {
			return 1; // blocking sliding piece check
		}
		if (numberOfChecks(pos) > 1) return 1;
	}
	return 0;
}
int isLegal(struct position *pos, struct move *move) {
	int kingpos;
	if (pos->tomove == WHITE) {
		kingpos = pos->Wkingpos;
	}
	else kingpos = pos->Bkingpos;
	
	if (move->piece == 'k' || move->piece == 'K') {
		return !isAttacked(pos, kingpos, !pos->tomove);
	}
	U64 pinned = pinnedPieces(pos);
	if (!(pinned && (1ULL << move->from))) {
		// piece is not pinned
		return 1;
	}
	if ((1ULL << move->to) & checkingAttack(pos)) {
		return 1;
	}
	return 0;
}