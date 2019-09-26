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

enum {NORT, NOEA, EAST, SOEA, SOUT, SOWE, WEST, NOWE};

int genKingMoves(struct position *pos, int square, struct move *moves, int forqsearch) {
	U64 BBattacks = 0ULL;
	int num_moves = 0;
	int piece = KING;
	int piececol = pos->tomove;
	if (pos->tomove == WHITE) {
		U64 BBking = (pos->pieces[KING] & pos->colours[WHITE]);
		//BBattacks = BBkingattacks(BBking);
		BBattacks = BBkingLookup[square];
		BBattacks = BBattacks & ~pos->colours[WHITE];
	}
	else if (pos->tomove == BLACK) {
		U64 BBking = (pos->pieces[KING] & pos->colours[BLACK]);
		//BBattacks = BBkingattacks(BBking);
		BBattacks = BBkingLookup[square];
		BBattacks = BBattacks & ~pos->colours[BLACK];
	}
	//dspBB(BBattacks);
	while (BBattacks != 0) {
		int movesquare = __builtin_ctzll(BBattacks);
		BBattacks &= ~(1ULL << movesquare);
		int cappiece = getPiece(pos,movesquare);
		if (!forqsearch || (forqsearch && cappiece !=  -1)) {
			moves[num_moves].from = square;
			moves[num_moves].to = movesquare;
			moves[num_moves].prom = -1;
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
			getPiece(pos,F1) == -1 &&
			getPiece(pos,G1) == -1 &&
			!isAttacked(pos, E1, BLACK) &&
			!isAttacked(pos, F1, BLACK) &&
			!isAttacked(pos, G1, BLACK)) {
				// Add move
				if (!forqsearch) {
					moves[num_moves].from = E1;
					moves[num_moves].to = G1;
					moves[num_moves].prom = -1;
					moves[num_moves].cappiece = -1;
					moves[num_moves].piece = piece;
					num_moves += 1;
				}
		}
		// Queenside castling
		if ((pos->WcastleQS == 1) &&
			getPiece(pos,D1)  == -1 &&
			getPiece(pos,C1)  == -1 &&
			getPiece(pos,B1)  == -1 &&
			!isAttacked(pos, E1, BLACK) &&
			!isAttacked(pos, D1, BLACK) &&
			!isAttacked(pos, C1, BLACK)) {
				// Add move
				if (!forqsearch) {
					moves[num_moves].from = E1;
					moves[num_moves].to = C1;
					moves[num_moves].prom = -1;
					moves[num_moves].cappiece = -1;
					moves[num_moves].piece = piece;
					num_moves += 1;
				}
		}
	}
	// black castling
	else if (pos->tomove == BLACK) {
		// Kingside castling
		if ((pos->BcastleKS == 1) &&
			getPiece(pos,F8) == -1 &&
			getPiece(pos,G8) == -1 &&
			!isAttacked(pos, E8, WHITE) &&
			!isAttacked(pos, F8, WHITE) &&
			!isAttacked(pos, G8, WHITE)) {
				// Add move
				if (!forqsearch) {
					moves[num_moves].from = E8;
					moves[num_moves].to = G8;
					moves[num_moves].prom = -1;
					moves[num_moves].cappiece = -1;
					moves[num_moves].piece = piece;
					num_moves += 1;
				}
		}
		// Queenside castling
		if ((pos->BcastleQS == 1) &&
			getPiece(pos,D8) == -1 &&
			getPiece(pos,C8) == -1 &&
			getPiece(pos,B8) == -1 &&
			!isAttacked(pos, E8, WHITE) &&
			!isAttacked(pos, D8, WHITE) &&
			!isAttacked(pos, C8, WHITE)) {
				// Add move
				if (!forqsearch) {
					moves[num_moves].from = E8;
					moves[num_moves].to = C8;
					moves[num_moves].prom = -1;
					moves[num_moves].cappiece = -1;
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
	int piece = KNIGHT;
	int piececol = pos->tomove;
	if (pos->tomove == WHITE) {
		U64 BBknight = (1ULL << square);
		//BBattacks = BBknightattacks(BBknight);
		BBattacks = BBknightLookup[square];
		BBattacks = BBattacks & ~pos->colours[WHITE];
	}
	else if (pos->tomove == BLACK) {
		U64 BBknight = (1ULL << square);
		//BBattacks = BBknightattacks(BBknight);
		BBattacks = BBknightLookup[square];
		BBattacks = BBattacks & ~pos->colours[BLACK];
	}
	//dspBB(BBattacks);
	while (BBattacks != 0) {
		int movesquare = __builtin_ctzll(BBattacks);
		//BBattacks &= ~(1ULL << movesquare);
		BBattacks &= BBattacks - 1;
		int cappiece = getPiece(pos,movesquare);
		if (!forqsearch || (forqsearch && cappiece != -1)) {
			moves[num_moves].from = square;
			moves[num_moves].to = movesquare;
			moves[num_moves].prom = -1;
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
	int piece = BISHOP;
	if (pos->tomove == WHITE) {
		U64 BBoccupancy = (pos->colours[WHITE] | pos->colours[BLACK]);
		BBattacks = Bmagic(square,BBoccupancy);
		BBattacks = BBattacks & ~pos->colours[WHITE];
	}
	else if (pos->tomove == BLACK) {
		U64 BBoccupancy = (pos->colours[WHITE] | pos->colours[BLACK]);
		BBattacks = Bmagic(square,BBoccupancy);
		BBattacks = BBattacks & ~pos->colours[BLACK];
	}
	while (BBattacks != 0) {
		int movesquare = __builtin_ctzll(BBattacks);
		//BBattacks &= ~(1ULL << movesquare);
		BBattacks &= BBattacks - 1;
		int cappiece = getPiece(pos,movesquare);
		if (!forqsearch || (forqsearch && cappiece != -1)) {
			moves[num_moves].from = square;
			moves[num_moves].to = movesquare;
			moves[num_moves].prom = -1;
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
	int piece = ROOK;
	if (pos->tomove == WHITE) {
		U64 BBoccupancy = (pos->colours[WHITE] | pos->colours[BLACK]);
		BBattacks = Rmagic(square,BBoccupancy);
		BBattacks = BBattacks & ~pos->colours[WHITE];
	}
	else if (pos->tomove == BLACK) {
		U64 BBoccupancy = (pos->colours[WHITE] | pos->colours[BLACK]);
		BBattacks = Rmagic(square,BBoccupancy);
		BBattacks = BBattacks & ~pos->colours[BLACK];
	}
	while (BBattacks != 0) {
		int movesquare = __builtin_ctzll(BBattacks);
		//BBattacks &= ~(1ULL << movesquare);
		BBattacks &= BBattacks - 1;
		int cappiece = getPiece(pos,movesquare);
		if (!forqsearch || (forqsearch && cappiece != -1)) {
			moves[num_moves].from = square;
			moves[num_moves].to = movesquare;
			moves[num_moves].prom = -1;
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
	int piece = QUEEN;
	if (pos->tomove == WHITE) {
		U64 BBoccupancy = (pos->colours[WHITE] | pos->colours[BLACK]);
		BBattacks = Rmagic(square,BBoccupancy) | Bmagic(square,BBoccupancy);
		BBattacks = BBattacks & ~pos->colours[WHITE];
	}
	else if (pos->tomove == BLACK) {
		U64 BBoccupancy = (pos->colours[WHITE] | pos->colours[BLACK]);
		BBattacks = Rmagic(square,BBoccupancy) | Bmagic(square,BBoccupancy);
		BBattacks = BBattacks & ~pos->colours[BLACK];
	}
	while (BBattacks != 0) {
		int movesquare = __builtin_ctzll(BBattacks);
		//BBattacks &= ~(1ULL << movesquare);
		BBattacks &= BBattacks - 1;
		int cappiece = getPiece(pos,movesquare);
		if (!forqsearch || (forqsearch && cappiece != -1)) {
			moves[num_moves].from = square;
			moves[num_moves].to = movesquare;
			moves[num_moves].prom = -1;
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
	
	int piece = PAWN;
	
	// white pawns
	if (pos->tomove == WHITE) {
		// double pawn pushe
		if (getrank(square) == 1) {
			// generate double pawn moves for pawns on 2nd rank
			BBmove = BBpawnDoublePushW(BBpawn,~(pos->colours[BLACK] | pos->colours[WHITE]));
			if (BBmove != 0) {
				int movesquare = __builtin_ctzll(BBmove);
				if (!forqsearch) {
					moves[num_moves].from = square;
					moves[num_moves].to = movesquare;
					moves[num_moves].prom = -1;
					moves[num_moves].cappiece = -1;
					moves[num_moves].piece = piece;
					num_moves++;
				}
			}
		}
		// single pawn push
		BBmove = BBpawnSinglePushW(BBpawn,~(pos->colours[BLACK] | pos->colours[WHITE]));
		if (BBmove != 0) {
			int movesquare = __builtin_ctzll(BBmove);
			if (getrank(movesquare) == 7) {
					moves[num_moves].from = square;
					moves[num_moves].to = movesquare;
					moves[num_moves].prom = QUEEN;
					moves[num_moves].cappiece = -1;
					moves[num_moves].piece = piece;
					num_moves++;
					moves[num_moves].from = square;
					moves[num_moves].to = movesquare;
					moves[num_moves].prom = ROOK;
					moves[num_moves].cappiece = -1;
					moves[num_moves].piece = piece;
					num_moves++;
					moves[num_moves].from = square;
					moves[num_moves].to = movesquare;
					moves[num_moves].prom = BISHOP;
					moves[num_moves].cappiece = -1;
					moves[num_moves].piece = piece;
					num_moves++;
					moves[num_moves].from = square;
					moves[num_moves].to = movesquare;
					moves[num_moves].prom = KNIGHT;
					moves[num_moves].cappiece = -1;
					moves[num_moves].piece = piece;
					num_moves++;
			}
			else {
				if (!forqsearch) {
					moves[num_moves].from = square;
					moves[num_moves].to = movesquare;
					moves[num_moves].prom = -1;
					moves[num_moves].cappiece = -1;
					moves[num_moves].piece = piece;
					num_moves++;
				}
			}
		}
		// attacks
		BBmove = BBpawnEastAttacksW(BBpawn) | BBpawnWestAttacksW(BBpawn);
		BBmove = (BBmove & pos->colours[BLACK]);
		while (BBmove != 0) {
			int movesquare = __builtin_ctzll(BBmove);
			if (getrank(movesquare) == 7) {
				int cappiece = getPiece(pos,movesquare);
				moves[num_moves].from = square;
				moves[num_moves].to = movesquare;
				moves[num_moves].prom = QUEEN;
				moves[num_moves].cappiece = cappiece;
				moves[num_moves].piece = piece;
				num_moves++;
				moves[num_moves].from = square;
				moves[num_moves].to = movesquare;
				moves[num_moves].prom = ROOK;
				moves[num_moves].cappiece = cappiece;
				moves[num_moves].piece = piece;
				num_moves++;
				moves[num_moves].from = square;
				moves[num_moves].to = movesquare;
				moves[num_moves].prom = BISHOP;
				moves[num_moves].cappiece = cappiece;
				moves[num_moves].piece = piece;
				num_moves++;
				moves[num_moves].from = square;
				moves[num_moves].to = movesquare;
				moves[num_moves].prom = KNIGHT;
				moves[num_moves].cappiece = cappiece;
				moves[num_moves].piece = piece;
				num_moves++;
			}
			else {
				int cappiece = getPiece(pos,movesquare);
				moves[num_moves].from = square;
				moves[num_moves].to = movesquare;
				moves[num_moves].prom = -1;
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
			U64 BBmove = BBpawnDoublePushB(BBpawn,~(pos->colours[BLACK] | pos->colours[WHITE]));
			if (BBmove != 0) {
				int movesquare = __builtin_ctzll(BBmove);
				if (!forqsearch) {
					moves[num_moves].from = square;
					moves[num_moves].to = movesquare;
					moves[num_moves].prom = -1;
					moves[num_moves].cappiece = -1;
					moves[num_moves].piece = piece;
					num_moves++;
				}
			}
		}
		// single pawn push
		U64 BBmove = BBpawnSinglePushB(BBpawn,~(pos->colours[BLACK] | pos->colours[WHITE]));
		if (BBmove != 0) {
			int movesquare = __builtin_ctzll(BBmove);
			if (getrank(movesquare) == 0) {
					moves[num_moves].from = square;
					moves[num_moves].to = movesquare;
					moves[num_moves].prom = QUEEN;
					moves[num_moves].cappiece = -1;
					moves[num_moves].piece = piece;
					num_moves++;
					moves[num_moves].from = square;
					moves[num_moves].to = movesquare;
					moves[num_moves].prom = ROOK;
					moves[num_moves].cappiece = -1;
					moves[num_moves].piece = piece;
					num_moves++;
					moves[num_moves].from = square;
					moves[num_moves].to = movesquare;
					moves[num_moves].prom = BISHOP;
					moves[num_moves].cappiece = -1;
					moves[num_moves].piece = piece;
					num_moves++;
					moves[num_moves].from = square;
					moves[num_moves].to = movesquare;
					moves[num_moves].prom = KNIGHT;
					moves[num_moves].cappiece = -1;
					moves[num_moves].piece = piece;
					num_moves++;
			}
			else {
				if (!forqsearch) {
					moves[num_moves].from = square;
					moves[num_moves].to = movesquare;
					moves[num_moves].prom = -1;
					moves[num_moves].cappiece = -1;
					moves[num_moves].piece = piece;
					num_moves++;
				}
			}
		}
		// attacks
		BBmove = BBpawnEastAttacksB(BBpawn) | BBpawnWestAttacksB(BBpawn);
		BBmove = (BBmove & pos->colours[WHITE]);
		while (BBmove != 0) {
			int movesquare = __builtin_ctzll(BBmove);
			if (getrank(movesquare) == 0) {
				int cappiece = getPiece(pos,movesquare);
				moves[num_moves].from = square;
				moves[num_moves].to = movesquare;
				moves[num_moves].prom = QUEEN;
				moves[num_moves].cappiece = cappiece;
				moves[num_moves].piece = piece;
				num_moves++;
				moves[num_moves].from = square;
				moves[num_moves].to = movesquare;
				moves[num_moves].prom = ROOK;
				moves[num_moves].cappiece = cappiece;
				moves[num_moves].piece = piece;
				num_moves++;
				moves[num_moves].from = square;
				moves[num_moves].to = movesquare;
				moves[num_moves].prom = BISHOP;
				moves[num_moves].cappiece = cappiece;
				moves[num_moves].piece = piece;
				num_moves++;
				moves[num_moves].from = square;
				moves[num_moves].to = movesquare;
				moves[num_moves].prom = KNIGHT;
				moves[num_moves].cappiece = cappiece;
				moves[num_moves].piece = piece;
				num_moves++;
			}
			else {
				int cappiece = getPiece(pos,movesquare);
				moves[num_moves].from = square;
				moves[num_moves].to = movesquare;
				moves[num_moves].prom = -1;
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
	if (pos->tomove == WHITE) BBallpieces = pos->colours[WHITE];
	else BBallpieces = pos->colours[BLACK];
	
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
				const int piece = getPiece(pos,idx);
				const int piececol = getPieceCol(pos, idx);
				if (piece == PAWN && piececol == BLACK) {
					// Add move
					if (!forqsearch) {
						moves[num_moves].from = idx;
						moves[num_moves].to = pos->epsquare;
						moves[num_moves].prom = -1;
						moves[num_moves].cappiece = -1;
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
				const int piece = getPiece(pos,idx);
				const int piececol = getPieceCol(pos, idx);
				if (piece == PAWN && piececol == WHITE) {
					// Add move
					if (!forqsearch) {
						moves[num_moves].from = idx;
						moves[num_moves].to = pos->epsquare;
						moves[num_moves].prom = -1;
						moves[num_moves].cappiece = -1;
						moves[num_moves].piece = piece;
						num_moves++;
					}
				}
			}
		}
	}
	
	
	while (BBallpieces != 0) {
		int square = __builtin_ctzll(BBallpieces);
		int piece = getPiece(pos,square);
		
		switch (piece) {
			case KING: num_moves += genKingMoves(pos,square,&moves[num_moves], forqsearch); break;
			case PAWN: num_moves += genPawnMoves(pos,square,&moves[num_moves], forqsearch); break;
			case KNIGHT: num_moves += genKnightMoves(pos,square,&moves[num_moves], forqsearch); break;
			case BISHOP: num_moves += genBishopMoves(pos,square,&moves[num_moves], forqsearch); break;
			case ROOK: num_moves += genRookMoves(pos,square,&moves[num_moves], forqsearch); break;
			case QUEEN: num_moves += genQueenMoves(pos,square,&moves[num_moves], forqsearch); break;
			
		}
		BBallpieces &= BBallpieces - 1;
	}
	return num_moves;
}
U64 pinnedPieces(struct position *pos) {
	U64 kingpos;
	U64 BBoppBQ, BBoppRQ;
	U64 BBpossiblePinned;
	if (pos->tomove == WHITE) {
		kingpos = pos->Wkingpos;
		BBoppBQ = pos->colours[BLACK] & (pos->pieces[QUEEN] | pos->pieces[BISHOP]);
		BBoppRQ = pos->colours[BLACK] & (pos->pieces[QUEEN] | pos->pieces[ROOK]);
		BBpossiblePinned = pos->colours[WHITE];
	}
	else {
		kingpos = pos->Bkingpos;
		BBoppBQ = pos->colours[WHITE] & (pos->pieces[QUEEN] | pos->pieces[BISHOP]);
		BBoppRQ = pos->colours[WHITE] & (pos->pieces[QUEEN] | pos->pieces[ROOK]);
		BBpossiblePinned = pos->colours[BLACK];
	}
	U64 BBoccupied = pos->colours[WHITE] | pos->colours[BLACK];
	
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

	// Lookup table for all eight ray-directions: NORT, NOEA, EAST, SOEA, SOUT, SOWE, WEST, NOWE
	static const U64 rayLookup[8][64] = {
		{0x101010101010100, 0x202020202020200, 0x404040404040400, 0x808080808080800, 0x1010101010101000, 0x2020202020202000, 0x4040404040404000, 0x8080808080808000, 0x101010101010000, 0x202020202020000, 0x404040404040000, 0x808080808080000, 0x1010101010100000, 0x2020202020200000, 0x4040404040400000, 0x8080808080800000, 0x101010101000000, 0x202020202000000, 0x404040404000000, 0x808080808000000, 0x1010101010000000, 0x2020202020000000, 0x4040404040000000, 0x8080808080000000, 0x101010100000000, 0x202020200000000, 0x404040400000000, 0x808080800000000, 0x1010101000000000, 0x2020202000000000, 0x4040404000000000, 0x8080808000000000, 0x101010000000000, 0x202020000000000, 0x404040000000000, 0x808080000000000, 0x1010100000000000, 0x2020200000000000, 0x4040400000000000, 0x8080800000000000, 0x101000000000000, 0x202000000000000, 0x404000000000000, 0x808000000000000, 0x1010000000000000, 0x2020000000000000, 0x4040000000000000, 0x8080000000000000, 0x100000000000000, 0x200000000000000, 0x400000000000000, 0x800000000000000, 0x1000000000000000, 0x2000000000000000, 0x4000000000000000, 0x8000000000000000, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
		{0x8040201008040200, 0x80402010080400, 0x804020100800, 0x8040201000, 0x80402000, 0x804000, 0x8000, 0x0, 0x4020100804020000, 0x8040201008040000, 0x80402010080000, 0x804020100000, 0x8040200000, 0x80400000, 0x800000, 0x0, 0x2010080402000000, 0x4020100804000000, 0x8040201008000000, 0x80402010000000, 0x804020000000, 0x8040000000, 0x80000000, 0x0, 0x1008040200000000, 0x2010080400000000, 0x4020100800000000, 0x8040201000000000, 0x80402000000000, 0x804000000000, 0x8000000000, 0x0, 0x804020000000000, 0x1008040000000000, 0x2010080000000000, 0x4020100000000000, 0x8040200000000000, 0x80400000000000, 0x800000000000, 0x0, 0x402000000000000, 0x804000000000000, 0x1008000000000000, 0x2010000000000000, 0x4020000000000000, 0x8040000000000000, 0x80000000000000, 0x0, 0x200000000000000, 0x400000000000000, 0x800000000000000, 0x1000000000000000, 0x2000000000000000, 0x4000000000000000, 0x8000000000000000, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
		{0xfe, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0, 0x80, 0x0, 0xfe00, 0xfc00, 0xf800, 0xf000, 0xe000, 0xc000, 0x8000, 0x0, 0xfe0000, 0xfc0000, 0xf80000, 0xf00000, 0xe00000, 0xc00000, 0x800000, 0x0, 0xfe000000, 0xfc000000, 0xf8000000, 0xf0000000, 0xe0000000, 0xc0000000, 0x80000000, 0x0, 0xfe00000000, 0xfc00000000, 0xf800000000, 0xf000000000, 0xe000000000, 0xc000000000, 0x8000000000, 0x0, 0xfe0000000000, 0xfc0000000000, 0xf80000000000, 0xf00000000000, 0xe00000000000, 0xc00000000000, 0x800000000000, 0x0, 0xfe000000000000, 0xfc000000000000, 0xf8000000000000, 0xf0000000000000, 0xe0000000000000, 0xc0000000000000, 0x80000000000000, 0x0, 0xfe00000000000000, 0xfc00000000000000, 0xf800000000000000, 0xf000000000000000, 0xe000000000000000, 0xc000000000000000, 0x8000000000000000, 0x0},
		{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80, 0x0, 0x204, 0x408, 0x810, 0x1020, 0x2040, 0x4080, 0x8000, 0x0, 0x20408, 0x40810, 0x81020, 0x102040, 0x204080, 0x408000, 0x800000, 0x0, 0x2040810, 0x4081020, 0x8102040, 0x10204080, 0x20408000, 0x40800000, 0x80000000, 0x0, 0x204081020, 0x408102040, 0x810204080, 0x1020408000, 0x2040800000, 0x4080000000, 0x8000000000, 0x0, 0x20408102040, 0x40810204080, 0x81020408000, 0x102040800000, 0x204080000000, 0x408000000000, 0x800000000000, 0x0, 0x2040810204080, 0x4081020408000, 0x8102040800000, 0x10204080000000, 0x20408000000000, 0x40800000000000, 0x80000000000000, 0x0},
		{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80, 0x101, 0x202, 0x404, 0x808, 0x1010, 0x2020, 0x4040, 0x8080, 0x10101, 0x20202, 0x40404, 0x80808, 0x101010, 0x202020, 0x404040, 0x808080, 0x1010101, 0x2020202, 0x4040404, 0x8080808, 0x10101010, 0x20202020, 0x40404040, 0x80808080, 0x101010101, 0x202020202, 0x404040404, 0x808080808, 0x1010101010, 0x2020202020, 0x4040404040, 0x8080808080, 0x10101010101, 0x20202020202, 0x40404040404, 0x80808080808, 0x101010101010, 0x202020202020, 0x404040404040, 0x808080808080, 0x1010101010101, 0x2020202020202, 0x4040404040404, 0x8080808080808, 0x10101010101010, 0x20202020202020, 0x40404040404040, 0x80808080808080},
		{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x0, 0x100, 0x201, 0x402, 0x804, 0x1008, 0x2010, 0x4020, 0x0, 0x10000, 0x20100, 0x40201, 0x80402, 0x100804, 0x201008, 0x402010, 0x0, 0x1000000, 0x2010000, 0x4020100, 0x8040201, 0x10080402, 0x20100804, 0x40201008, 0x0, 0x100000000, 0x201000000, 0x402010000, 0x804020100, 0x1008040201, 0x2010080402, 0x4020100804, 0x0, 0x10000000000, 0x20100000000, 0x40201000000, 0x80402010000, 0x100804020100, 0x201008040201, 0x402010080402, 0x0, 0x1000000000000, 0x2010000000000, 0x4020100000000, 0x8040201000000, 0x10080402010000, 0x20100804020100, 0x40201008040201},
		{0x0, 0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f, 0x0, 0x100, 0x300, 0x700, 0xf00, 0x1f00, 0x3f00, 0x7f00, 0x0, 0x10000, 0x30000, 0x70000, 0xf0000, 0x1f0000, 0x3f0000, 0x7f0000, 0x0, 0x1000000, 0x3000000, 0x7000000, 0xf000000, 0x1f000000, 0x3f000000, 0x7f000000, 0x0, 0x100000000, 0x300000000, 0x700000000, 0xf00000000, 0x1f00000000, 0x3f00000000, 0x7f00000000, 0x0, 0x10000000000, 0x30000000000, 0x70000000000, 0xf0000000000, 0x1f0000000000, 0x3f0000000000, 0x7f0000000000, 0x0, 0x1000000000000, 0x3000000000000, 0x7000000000000, 0xf000000000000, 0x1f000000000000, 0x3f000000000000, 0x7f000000000000, 0x0, 0x100000000000000, 0x300000000000000, 0x700000000000000, 0xf00000000000000, 0x1f00000000000000, 0x3f00000000000000, 0x7f00000000000000},
		{0x0, 0x100, 0x10200, 0x1020400, 0x102040800, 0x10204081000, 0x1020408102000, 0x102040810204000, 0x0, 0x10000, 0x1020000, 0x102040000, 0x10204080000, 0x1020408100000, 0x102040810200000, 0x204081020400000, 0x0, 0x1000000, 0x102000000, 0x10204000000, 0x1020408000000, 0x102040810000000, 0x204081020000000, 0x408102040000000, 0x0, 0x100000000, 0x10200000000, 0x1020400000000, 0x102040800000000, 0x204081000000000, 0x408102000000000, 0x810204000000000, 0x0, 0x10000000000, 0x1020000000000, 0x102040000000000, 0x204080000000000, 0x408100000000000, 0x810200000000000, 0x1020400000000000, 0x0, 0x1000000000000, 0x102000000000000, 0x204000000000000, 0x408000000000000, 0x810000000000000, 0x1020000000000000, 0x2040000000000000, 0x0, 0x100000000000000, 0x200000000000000, 0x400000000000000, 0x800000000000000, 0x1000000000000000, 0x2000000000000000, 0x4000000000000000, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}
	};

	const int aFile = getfile(a), bFile = getfile(b);

	if (aFile == bFile)
		return rayLookup[NORT][a] | rayLookup[SOUT][a] | 1ULL << a;

	const int aRank = getrank(a), bRank = getrank(b);

	if (aRank == bRank)
		return rayLookup[EAST][a] | rayLookup[WEST][a] | 1ULL << a;

	// Right diagonal
	if (aRank - aFile == bRank - bFile)
	   return rayLookup[NOEA][a] | rayLookup[SOWE][a] | 1ULL << a;

	// Left diagonal
	if (aRank + aFile == bRank + bFile)
		return rayLookup[NOWE][a] | rayLookup[SOEA][a] | 1ULL << a;

	return 0;
}