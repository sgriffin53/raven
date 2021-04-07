#include <stdio.h>
#include <ctype.h>
#include "movegen.h"
#include "attacks.h"
#include "position.h"
#include "move.h"
#include "magicmoves.h"
#include "../globals.h"
#include "bitboards.h"

const int BPdirs[2][2] = {{-1,-1},{+1,-1}};
const int WPdirs[2][2] = {{-1,+1},{+1,+1}};

int genKingMoves(struct position *pos, int square, struct move *moves, int forqsearch) {
	U64 BBattacks = 0ULL;
	int num_moves = 0;
	if (pos->tomove == WHITE) {
		U64 BBking = (pos->pieces[KING] & pos->colours[WHITE]);
		BBattacks = BBkingLookup[square];
		BBattacks = BBattacks & ~pos->colours[WHITE];
		if (forqsearch) BBattacks = BBattacks & pos->colours[BLACK];
	}
	else if (pos->tomove == BLACK) {
		U64 BBking = (pos->pieces[KING] & pos->colours[BLACK]);
		BBattacks = BBkingLookup[square];
		BBattacks = BBattacks & ~pos->colours[BLACK];
		if (forqsearch) BBattacks = BBattacks & pos->colours[WHITE];
	}
	//dspBB(BBattacks);
	while (BBattacks != 0) {
		int movesquare = __builtin_ctzll(BBattacks);
		BBattacks &= BBattacks - 1;
		moves[num_moves].from = square;
		moves[num_moves].to = movesquare;
		moves[num_moves].prom = NONE;
		moves[num_moves].cappiece = getPiece(pos,movesquare);
		moves[num_moves].piece = KING;
		num_moves++;
	}
	// castling moves
	// white castling
	if (!forqsearch) {
		if (pos->tomove == WHITE) {
			// King side castling
			if ((pos->WcastleKS == 1) &&
				getPiece(pos,F1) == NONE &&
				getPiece(pos,G1) == NONE &&
				!isAttacked(pos, E1, BLACK) &&
				!isAttacked(pos, F1, BLACK) &&
				!isAttacked(pos, G1, BLACK)) {
				// Add move
				moves[num_moves].from = E1;
				moves[num_moves].to = G1;
				moves[num_moves].prom = NONE;
				moves[num_moves].cappiece = NONE;
				moves[num_moves].piece = KING;
				num_moves += 1;
			}
			// Queenside castling
			if ((pos->WcastleQS == 1) &&
				getPiece(pos,D1)  == NONE &&
				getPiece(pos,C1)  == NONE &&
				getPiece(pos,B1)  == NONE &&
				!isAttacked(pos, E1, BLACK) &&
				!isAttacked(pos, D1, BLACK) &&
				!isAttacked(pos, C1, BLACK)) {
				// Add move
				moves[num_moves].from = E1;
				moves[num_moves].to = C1;
				moves[num_moves].prom = NONE;
				moves[num_moves].cappiece = NONE;
				moves[num_moves].piece = KING;
				num_moves += 1;
			}
		}
		// black castling
		else if (pos->tomove == BLACK) {
			// Kingside castling
			if ((pos->BcastleKS == 1) &&
				getPiece(pos,F8) == NONE &&
				getPiece(pos,G8) == NONE &&
				!isAttacked(pos, E8, WHITE) &&
				!isAttacked(pos, F8, WHITE) &&
				!isAttacked(pos, G8, WHITE)) {
					// Add move
					if (!forqsearch) {
						moves[num_moves].from = E8;
						moves[num_moves].to = G8;
						moves[num_moves].prom = NONE;
						moves[num_moves].cappiece = NONE;
						moves[num_moves].piece = KING;
						num_moves += 1;
					}
			}
			// Queenside castling
			if ((pos->BcastleQS == 1) &&
				getPiece(pos,D8) == NONE &&
				getPiece(pos,C8) == NONE &&
				getPiece(pos,B8) == NONE &&
				!isAttacked(pos, E8, WHITE) &&
				!isAttacked(pos, D8, WHITE) &&
				!isAttacked(pos, C8, WHITE)) {
				// Add move
				moves[num_moves].from = E8;
				moves[num_moves].to = C8;
				moves[num_moves].prom = NONE;
				moves[num_moves].cappiece = NONE;
				moves[num_moves].piece = KING;
				num_moves += 1;
			}
		}
	}
	return num_moves;
}
int genKnightMoves(struct position *pos, int square, struct move *moves, int forqsearch) {
	U64 BBattacks = 0ULL;
	int num_moves = 0;
	if (pos->tomove == WHITE) {
		U64 BBknight = (1ULL << square);
		BBattacks = BBknightLookup[square];
		BBattacks = BBattacks & ~pos->colours[WHITE];
		if (forqsearch) BBattacks = BBattacks & pos->colours[BLACK];
	}
	else if (pos->tomove == BLACK) {
		U64 BBknight = (1ULL << square);
		BBattacks = BBknightLookup[square];
		BBattacks = BBattacks & ~pos->colours[BLACK];
		if (forqsearch) BBattacks = BBattacks & pos->colours[WHITE];
	}
	//dspBB(BBattacks);
	while (BBattacks != 0) {
		int movesquare = __builtin_ctzll(BBattacks);
		BBattacks &= BBattacks - 1;
		moves[num_moves].from = square;
		moves[num_moves].to = movesquare;
		moves[num_moves].prom = NONE;
		moves[num_moves].cappiece = getPiece(pos,movesquare);
		moves[num_moves].piece = KNIGHT;
		num_moves++;
	}
	return num_moves;
}
int genBishopMoves(struct position *pos, int square, struct move *moves, int forqsearch) {
	U64 BBattacks = 0ULL;
	int num_moves = 0;
	if (pos->tomove == WHITE) {
		U64 BBoccupancy = (pos->colours[WHITE] | pos->colours[BLACK]);
		BBattacks = Bmagic(square,BBoccupancy);
		BBattacks = BBattacks & ~pos->colours[WHITE];
		if (forqsearch) BBattacks = BBattacks & pos->colours[BLACK];
	}
	else if (pos->tomove == BLACK) {
		U64 BBoccupancy = (pos->colours[WHITE] | pos->colours[BLACK]);
		BBattacks = Bmagic(square,BBoccupancy);
		BBattacks = BBattacks & ~pos->colours[BLACK];
		if (forqsearch) BBattacks = BBattacks & pos->colours[WHITE];
	}
	while (BBattacks != 0) {
		int movesquare = __builtin_ctzll(BBattacks);
		BBattacks &= BBattacks - 1;
		moves[num_moves].from = square;
		moves[num_moves].to = movesquare;
		moves[num_moves].prom = NONE;
		moves[num_moves].cappiece = getPiece(pos,movesquare);
		moves[num_moves].piece = BISHOP;
		num_moves++;
	}
	return num_moves;
}
int genRookMoves(struct position *pos, int square, struct move *moves, int forqsearch) {
	U64 BBattacks = 0ULL;
	int num_moves = 0;
	if (pos->tomove == WHITE) {
		U64 BBoccupancy = (pos->colours[WHITE] | pos->colours[BLACK]);
		BBattacks = Rmagic(square,BBoccupancy);
		BBattacks = BBattacks & ~pos->colours[WHITE];
		if (forqsearch) BBattacks = BBattacks & pos->colours[BLACK];
	}
	else if (pos->tomove == BLACK) {
		U64 BBoccupancy = (pos->colours[WHITE] | pos->colours[BLACK]);
		BBattacks = Rmagic(square,BBoccupancy);
		BBattacks = BBattacks & ~pos->colours[BLACK];
		if (forqsearch) BBattacks = BBattacks & pos->colours[WHITE];
	}
	//dspBB(BBattacks);
	while (BBattacks != 0) {
		int movesquare = __builtin_ctzll(BBattacks);
		BBattacks &= BBattacks - 1;
		moves[num_moves].from = square;
		moves[num_moves].to = movesquare;
		moves[num_moves].prom = NONE;
		moves[num_moves].cappiece = getPiece(pos,movesquare);
		moves[num_moves].piece = ROOK;
		num_moves++;
	}
	return num_moves;
}
int genQueenMoves(struct position *pos, int square, struct move *moves, int forqsearch) {
	U64 BBattacks = 0ULL;
	int num_moves = 0;
	if (pos->tomove == WHITE) {
		U64 BBoccupancy = (pos->colours[WHITE] | pos->colours[BLACK]);
		BBattacks = Rmagic(square,BBoccupancy) | Bmagic(square,BBoccupancy);
		BBattacks = BBattacks & ~pos->colours[WHITE];
		if (forqsearch) BBattacks = BBattacks & pos->colours[BLACK];
	}
	else if (pos->tomove == BLACK) {
		U64 BBoccupancy = (pos->colours[WHITE] | pos->colours[BLACK]);
		BBattacks = Rmagic(square,BBoccupancy) | Bmagic(square,BBoccupancy);
		BBattacks = BBattacks & ~pos->colours[BLACK];
		if (forqsearch) BBattacks = BBattacks & pos->colours[WHITE];
	}
	while (BBattacks != 0) {
		int movesquare = __builtin_ctzll(BBattacks);
		BBattacks &= BBattacks - 1;
		moves[num_moves].from = square;
		moves[num_moves].to = movesquare;
		moves[num_moves].prom = NONE;
		moves[num_moves].cappiece = getPiece(pos,movesquare);
		moves[num_moves].piece = QUEEN;
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
					moves[num_moves].prom = NONE;
					moves[num_moves].cappiece = NONE;
					moves[num_moves].piece = PAWN;
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
					moves[num_moves].cappiece = NONE;
					moves[num_moves].piece = PAWN;
					num_moves++;
					moves[num_moves].from = square;
					moves[num_moves].to = movesquare;
					moves[num_moves].prom = ROOK;
					moves[num_moves].cappiece = NONE;
					moves[num_moves].piece = PAWN;
					num_moves++;
					moves[num_moves].from = square;
					moves[num_moves].to = movesquare;
					moves[num_moves].prom = BISHOP;
					moves[num_moves].cappiece = NONE;
					moves[num_moves].piece = PAWN;
					num_moves++;
					moves[num_moves].from = square;
					moves[num_moves].to = movesquare;
					moves[num_moves].prom = KNIGHT;
					moves[num_moves].cappiece = NONE;
					moves[num_moves].piece = PAWN;
					num_moves++;
			}
			else {
				if (!forqsearch) {
					moves[num_moves].from = square;
					moves[num_moves].to = movesquare;
					moves[num_moves].prom = NONE;
					moves[num_moves].cappiece = NONE;
					moves[num_moves].piece = PAWN;
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
				char cappiece = getPiece(pos,movesquare);
				moves[num_moves].from = square;
				moves[num_moves].to = movesquare;
				moves[num_moves].prom = QUEEN;
				moves[num_moves].cappiece = cappiece;
				moves[num_moves].piece = PAWN;
				num_moves++;
				moves[num_moves].from = square;
				moves[num_moves].to = movesquare;
				moves[num_moves].prom = ROOK;
				moves[num_moves].cappiece = cappiece;
				moves[num_moves].piece = PAWN;
				num_moves++;
				moves[num_moves].from = square;
				moves[num_moves].to = movesquare;
				moves[num_moves].prom = BISHOP;
				moves[num_moves].cappiece = cappiece;
				moves[num_moves].piece = PAWN;
				num_moves++;
				moves[num_moves].from = square;
				moves[num_moves].to = movesquare;
				moves[num_moves].prom = KNIGHT;
				moves[num_moves].cappiece = cappiece;
				moves[num_moves].piece = PAWN;
				num_moves++;
			}
			else {
				moves[num_moves].from = square;
				moves[num_moves].to = movesquare;
				moves[num_moves].prom = NONE;
				moves[num_moves].cappiece = getPiece(pos,movesquare);
				moves[num_moves].piece = PAWN;
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
					moves[num_moves].prom = NONE;
					moves[num_moves].cappiece = NONE;
					moves[num_moves].piece = PAWN;
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
					moves[num_moves].cappiece = NONE;
					moves[num_moves].piece = PAWN;
					num_moves++;
					moves[num_moves].from = square;
					moves[num_moves].to = movesquare;
					moves[num_moves].prom = ROOK;
					moves[num_moves].cappiece = NONE;
					moves[num_moves].piece = PAWN;
					num_moves++;
					moves[num_moves].from = square;
					moves[num_moves].to = movesquare;
					moves[num_moves].prom = BISHOP;
					moves[num_moves].cappiece = NONE;
					moves[num_moves].piece = PAWN;
					num_moves++;
					moves[num_moves].from = square;
					moves[num_moves].to = movesquare;
					moves[num_moves].prom = KNIGHT;
					moves[num_moves].cappiece = NONE;
					moves[num_moves].piece = PAWN;
					num_moves++;
			}
			else {
				if (!forqsearch) {
					moves[num_moves].from = square;
					moves[num_moves].to = movesquare;
					moves[num_moves].prom = NONE;
					moves[num_moves].cappiece = NONE;
					moves[num_moves].piece = PAWN;
					num_moves++;
				}
			}
		}
		// attacks
		BBmove = BBpawnEastAttacksB(BBpawn) | BBpawnWestAttacksB(BBpawn);
		BBmove = (BBmove & pos->colours[WHITE]);
		//printf("attacks from %d\n", square);
		//dspBB(BBmove);
		while (BBmove != 0) {
			int movesquare = __builtin_ctzll(BBmove);
			if (getrank(movesquare) == 0) {
				char cappiece = getPiece(pos,movesquare);
				moves[num_moves].from = square;
				moves[num_moves].to = movesquare;
				moves[num_moves].prom = QUEEN;
				moves[num_moves].cappiece = cappiece;
				moves[num_moves].piece = PAWN;
				num_moves++;
				moves[num_moves].from = square;
				moves[num_moves].to = movesquare;
				moves[num_moves].prom = ROOK;
				moves[num_moves].cappiece = cappiece;
				moves[num_moves].piece = PAWN;
				num_moves++;
				moves[num_moves].from = square;
				moves[num_moves].to = movesquare;
				moves[num_moves].prom = BISHOP;
				moves[num_moves].cappiece = cappiece;
				moves[num_moves].piece = PAWN;
				num_moves++;
				moves[num_moves].from = square;
				moves[num_moves].to = movesquare;
				moves[num_moves].prom = KNIGHT;
				moves[num_moves].cappiece = cappiece;
				moves[num_moves].piece = PAWN;
				num_moves++;
			}
			else {
				moves[num_moves].from = square;
				moves[num_moves].to = movesquare;
				moves[num_moves].prom = NONE;
				moves[num_moves].cappiece = getPiece(pos,movesquare);
				moves[num_moves].piece = PAWN;
				num_moves++;
			}
			BBmove &= BBmove - 1;
		}
	}
	return num_moves;
}
int genAllPawnMoves(struct position *pos, int square, struct move *moves, int forqsearch) {
	// generates all pawn moves
	U64 BBoccupied = pos->colours[WHITE] | pos->colours[BLACK];
	U64 BBwhitepawns = pos->colours[WHITE] & pos->pieces[PAWN];
	U64 BBblackpawns = pos->colours[BLACK] & pos->pieces[PAWN];
	int num_moves = 0;
	char piece = PAWN;
	if (pos->tomove == WHITE) {
		// double pushes
		if (!forqsearch) {
			U64 BBstartpawns = BBwhitepawns & BBrank2;
			U64 BBdoublepushes = (BBstartpawns << 16) & ~(BBoccupied);
			while (BBdoublepushes) {
				int targetsquare = __builtin_ctzll(BBdoublepushes);
				BBdoublepushes &= BBdoublepushes - 1;
				U64 BBrank3blocked = ((1ULL << targetsquare) >> 8) & (BBoccupied);
				if (BBrank3blocked) continue;
				int sourcesquare = targetsquare - 16;
				moves[num_moves].from = sourcesquare;
				moves[num_moves].to = targetsquare;
				moves[num_moves].prom = NONE;
				moves[num_moves].cappiece = NONE;
				moves[num_moves].piece = piece;
				num_moves++;
			}
		}
		// single pushes
		
		U64 BBpawns = BBwhitepawns;
		U64 BBsinglepushes = (BBpawns << 8) & ~(BBoccupied);
		while (BBsinglepushes) {
			int targetsquare = __builtin_ctzll(BBsinglepushes);
			BBsinglepushes &= BBsinglepushes - 1;
			int sourcesquare = targetsquare - 8;
			if (getrank(targetsquare) == 7) {
					moves[num_moves].from = sourcesquare;
					moves[num_moves].to = targetsquare;
					moves[num_moves].prom = QUEEN;
					moves[num_moves].cappiece = NONE;
					moves[num_moves].piece = piece;
					num_moves++;
					moves[num_moves].from = sourcesquare;
					moves[num_moves].to = targetsquare;
					moves[num_moves].prom = ROOK;
					moves[num_moves].cappiece = NONE;
					moves[num_moves].piece = piece;
					num_moves++;
					moves[num_moves].from = sourcesquare;
					moves[num_moves].to = targetsquare;
					moves[num_moves].prom = BISHOP;
					moves[num_moves].cappiece = NONE;
					moves[num_moves].piece = piece;
					num_moves++;
					moves[num_moves].from = sourcesquare;
					moves[num_moves].to = targetsquare;
					moves[num_moves].prom = KNIGHT;
					moves[num_moves].cappiece = NONE;
					moves[num_moves].piece = piece;
					num_moves++;
			}
			else {
				if (!forqsearch) {
					moves[num_moves].from = sourcesquare;
					moves[num_moves].to = targetsquare;
					moves[num_moves].prom = NONE;
					moves[num_moves].cappiece = NONE;
					moves[num_moves].piece = piece;
					num_moves++;
				}
			}
		}
		
		// left capture
		
		U64 BBleftcaptures = noWeOne(BBpawns) & pos->colours[BLACK];
		while (BBleftcaptures) {
			int targetsquare = __builtin_ctzll(BBleftcaptures);
			BBleftcaptures &= BBleftcaptures - 1;
			int sourcesquare = targetsquare - 7;
			if (getrank(targetsquare) == 7) {
				char cappiece = getPiece(pos,targetsquare);
				moves[num_moves].from = sourcesquare;
				moves[num_moves].to = targetsquare;
				moves[num_moves].prom = QUEEN;
				moves[num_moves].cappiece = cappiece;
				moves[num_moves].piece = piece;
				num_moves++;
				moves[num_moves].from = sourcesquare;
				moves[num_moves].to = targetsquare;
				moves[num_moves].prom = ROOK;
				moves[num_moves].cappiece = cappiece;
				moves[num_moves].piece = piece;
				num_moves++;
				moves[num_moves].from = sourcesquare;
				moves[num_moves].to = targetsquare;
				moves[num_moves].prom = BISHOP;
				moves[num_moves].cappiece = cappiece;
				moves[num_moves].piece = piece;
				num_moves++;
				moves[num_moves].from = sourcesquare;
				moves[num_moves].to = targetsquare;
				moves[num_moves].prom = KNIGHT;
				moves[num_moves].cappiece = cappiece;
				moves[num_moves].piece = piece;
				num_moves++;
			}
			else {
				char cappiece = getPiece(pos,targetsquare);
				moves[num_moves].from = sourcesquare;
				moves[num_moves].to = targetsquare;
				moves[num_moves].prom = NONE;
				moves[num_moves].cappiece = cappiece;
				moves[num_moves].piece = piece;
				num_moves++;
			}
		}
		
		// right capture
		
		U64 BBrightcaptures = noEaOne(BBpawns) & pos->colours[BLACK];
		while (BBrightcaptures) {
			int targetsquare = __builtin_ctzll(BBrightcaptures);
			BBrightcaptures &= BBrightcaptures - 1;
			int sourcesquare = targetsquare - 9;
			if (getrank(targetsquare) == 7) {
				char cappiece = getPiece(pos,targetsquare);
				moves[num_moves].from = sourcesquare;
				moves[num_moves].to = targetsquare;
				moves[num_moves].prom = QUEEN;
				moves[num_moves].cappiece = cappiece;
				moves[num_moves].piece = piece;
				num_moves++;
				moves[num_moves].from = sourcesquare;
				moves[num_moves].to = targetsquare;
				moves[num_moves].prom = ROOK;
				moves[num_moves].cappiece = cappiece;
				moves[num_moves].piece = piece;
				num_moves++;
				moves[num_moves].from = sourcesquare;
				moves[num_moves].to = targetsquare;
				moves[num_moves].prom = BISHOP;
				moves[num_moves].cappiece = cappiece;
				moves[num_moves].piece = piece;
				num_moves++;
				moves[num_moves].from = sourcesquare;
				moves[num_moves].to = targetsquare;
				moves[num_moves].prom = KNIGHT;
				moves[num_moves].cappiece = cappiece;
				moves[num_moves].piece = piece;
				num_moves++;
			}
			else {
				char cappiece = getPiece(pos,targetsquare);
				moves[num_moves].from = sourcesquare;
				moves[num_moves].to = targetsquare;
				moves[num_moves].prom = NONE;
				moves[num_moves].cappiece = cappiece;
				moves[num_moves].piece = piece;
				num_moves++;
			}
		}
	}
	else {
		// black to move
		
		// double pushes
		U64 BBstartpawns = BBblackpawns & BBrank7;
		U64 BBdoublepushes = (BBstartpawns >> 16) & ~(BBoccupied);
		if (!forqsearch) {
			while (BBdoublepushes) {
				int targetsquare = __builtin_ctzll(BBdoublepushes);
				BBdoublepushes &= BBdoublepushes - 1;
				U64 BBrank3blocked = ((1ULL << targetsquare) << 8) & (BBoccupied);
				if (BBrank3blocked) continue;
				int sourcesquare = targetsquare + 16;
				moves[num_moves].from = sourcesquare;
				moves[num_moves].to = targetsquare;
				moves[num_moves].prom = NONE;
				moves[num_moves].cappiece = NONE;
				moves[num_moves].piece = piece;
				num_moves++;
			}
		}
		// single pushes
		
		U64 BBpawns = BBblackpawns;
		U64 BBsinglepushes = (BBpawns >> 8) & ~(BBoccupied);
		while (BBsinglepushes) {
			int targetsquare = __builtin_ctzll(BBsinglepushes);
			BBsinglepushes &= BBsinglepushes - 1;
			int sourcesquare = targetsquare + 8;
			if (getrank(targetsquare) == 0) {
					moves[num_moves].from = sourcesquare;
					moves[num_moves].to = targetsquare;
					moves[num_moves].prom = QUEEN;
					moves[num_moves].cappiece = NONE;
					moves[num_moves].piece = piece;
					num_moves++;
					moves[num_moves].from = sourcesquare;
					moves[num_moves].to = targetsquare;
					moves[num_moves].prom = ROOK;
					moves[num_moves].cappiece = NONE;
					moves[num_moves].piece = piece;
					num_moves++;
					moves[num_moves].from = sourcesquare;
					moves[num_moves].to = targetsquare;
					moves[num_moves].prom = BISHOP;
					moves[num_moves].cappiece = NONE;
					moves[num_moves].piece = piece;
					num_moves++;
					moves[num_moves].from = sourcesquare;
					moves[num_moves].to = targetsquare;
					moves[num_moves].prom = KNIGHT;
					moves[num_moves].cappiece = NONE;
					moves[num_moves].piece = piece;
					num_moves++;
			}
			else {
				if (!forqsearch) {
					moves[num_moves].from = sourcesquare;
					moves[num_moves].to = targetsquare;
					moves[num_moves].prom = NONE;
					moves[num_moves].cappiece = NONE;
					moves[num_moves].piece = piece;
					num_moves++;
				}
			}
		}
		// left capture
		
		U64 BBleftcaptures = soWeOne(BBpawns) & pos->colours[WHITE];
		while (BBleftcaptures) {
			int targetsquare = __builtin_ctzll(BBleftcaptures);
			BBleftcaptures &= BBleftcaptures - 1;
			int sourcesquare = targetsquare + 9;
			if (getrank(targetsquare) == 0) {
				char cappiece = getPiece(pos,targetsquare);
				moves[num_moves].from = sourcesquare;
				moves[num_moves].to = targetsquare;
				moves[num_moves].prom = QUEEN;
				moves[num_moves].cappiece = cappiece;
				moves[num_moves].piece = piece;
				num_moves++;
				moves[num_moves].from = sourcesquare;
				moves[num_moves].to = targetsquare;
				moves[num_moves].prom = ROOK;
				moves[num_moves].cappiece = cappiece;
				moves[num_moves].piece = piece;
				num_moves++;
				moves[num_moves].from = sourcesquare;
				moves[num_moves].to = targetsquare;
				moves[num_moves].prom = BISHOP;
				moves[num_moves].cappiece = cappiece;
				moves[num_moves].piece = piece;
				num_moves++;
				moves[num_moves].from = sourcesquare;
				moves[num_moves].to = targetsquare;
				moves[num_moves].prom = KNIGHT;
				moves[num_moves].cappiece = cappiece;
				moves[num_moves].piece = piece;
				num_moves++;
			}
			else {
				char cappiece = getPiece(pos,targetsquare);
				moves[num_moves].from = sourcesquare;
				moves[num_moves].to = targetsquare;
				moves[num_moves].prom = NONE;
				moves[num_moves].cappiece = cappiece;
				moves[num_moves].piece = piece;
				num_moves++;
			}
		}
		// right capture
		
		U64 BBrightcaptures = soEaOne(BBpawns) & pos->colours[WHITE];
		while (BBrightcaptures) {
			int targetsquare = __builtin_ctzll(BBrightcaptures);
			BBrightcaptures &= BBrightcaptures - 1;
			int sourcesquare = targetsquare + 7;
			if (getrank(targetsquare) == 0) {
				char cappiece = getPiece(pos,targetsquare);
				moves[num_moves].from = sourcesquare;
				moves[num_moves].to = targetsquare;
				moves[num_moves].prom = QUEEN;
				moves[num_moves].cappiece = cappiece;
				moves[num_moves].piece = piece;
				num_moves++;
				moves[num_moves].from = sourcesquare;
				moves[num_moves].to = targetsquare;
				moves[num_moves].prom = ROOK;
				moves[num_moves].cappiece = cappiece;
				moves[num_moves].piece = piece;
				num_moves++;
				moves[num_moves].from = sourcesquare;
				moves[num_moves].to = targetsquare;
				moves[num_moves].prom = BISHOP;
				moves[num_moves].cappiece = cappiece;
				moves[num_moves].piece = piece;
				num_moves++;
				moves[num_moves].from = sourcesquare;
				moves[num_moves].to = targetsquare;
				moves[num_moves].prom = KNIGHT;
				moves[num_moves].cappiece = cappiece;
				moves[num_moves].piece = piece;
				num_moves++;
			}
			else {
				char cappiece = getPiece(pos,targetsquare);
				moves[num_moves].from = sourcesquare;
				moves[num_moves].to = targetsquare;
				moves[num_moves].prom = NONE;
				moves[num_moves].cappiece = cappiece;
				moves[num_moves].piece = piece;
				num_moves++;
			}
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
				const char piece = getPiece(pos,idx);

				if (piece == PAWN && getColour(pos, idx) == BLACK) {
					// Add move
					if (!forqsearch) {
						moves[num_moves].from = idx;
						moves[num_moves].to = pos->epsquare;
						moves[num_moves].prom = NONE;
						moves[num_moves].cappiece = NONE;
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
				if (piece == PAWN && getColour(pos, idx) == WHITE) {
					// Add move
					if (!forqsearch) {
						moves[num_moves].from = idx;
						moves[num_moves].to = pos->epsquare;
						moves[num_moves].prom = NONE;
						moves[num_moves].cappiece = NONE;
						moves[num_moves].piece = piece;
						num_moves++;
					}
				}
			}
		}
	}
	
	
	BBallpieces &= ~pos->pieces[PAWN];
	
	while (BBallpieces != 0) {
		int square = __builtin_ctzll(BBallpieces);
		char piece = getPiece(pos,square);
		switch (piece) {
			case KING: num_moves += genKingMoves(pos,square,&moves[num_moves], forqsearch); break;
			//case PAWN: num_moves += genPawnMoves(pos,square,&moves[num_moves], forqsearch); break;
			case KNIGHT: num_moves += genKnightMoves(pos,square,&moves[num_moves], forqsearch); break;
			case BISHOP: num_moves += genBishopMoves(pos,square,&moves[num_moves], forqsearch); break;
			case ROOK: num_moves += genRookMoves(pos,square,&moves[num_moves], forqsearch); break;
			case QUEEN: num_moves += genQueenMoves(pos,square,&moves[num_moves], forqsearch); break;
			
		}
		BBallpieces &= BBallpieces - 1;
	}
	
	U64 BBstmPawns = 0;
	if (pos->tomove == WHITE) BBstmPawns = pos->colours[WHITE] & pos->pieces[PAWN];
	else BBstmPawns = pos->colours[BLACK] & pos->pieces[PAWN];
	if (BBstmPawns) num_moves += genAllPawnMoves(pos, 0, &moves[num_moves], forqsearch);
	
	return num_moves;
}