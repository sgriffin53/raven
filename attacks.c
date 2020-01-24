#include <stdio.h>

#include "attacks.h"
#include "bitboards.h"
#include "hash.h"
#include "magicmoves.h"
#include "globals.h"

int isCheck(struct position *pos) {
	if (pos->tomove == WHITE) return isAttacked(pos, pos->Wkingpos, !pos->tomove);
	else return isAttacked(pos, pos->Bkingpos, !pos->tomove);
}
int isAttacked(struct position *pos,int square, int colour) {
	assert(pos);
	assert(square >= 0 && square <= 63);
	assert(colour == WHITE || colour == BLACK);
	// colour is colour of attacking side

	// check if being attacked by colour's king
	U64 BBpiece = (1ULL << square); // get bitboard of piece on square
	U64 BBattacks;
	// pawn attacks
	if (colour == WHITE) BBattacks = BBpawnattacksWFlipped(BBpiece);
	else BBattacks = BBpawnattacksBFlipped(BBpiece);
	BBattacks = BBattacks & (pos->pieces[PAWN] & pos->colours[colour]);
	if (BBattacks) {
		//piece being attacked by pawn
		return 1;
	}
	// get king attack squares
	BBattacks |= BBkingLookup[square];
	BBattacks = BBattacks & (pos->pieces[KING] & pos->colours[colour]);
	if (BBattacks) {
		//piece being attacked by king
		return 1;
	}
	// get knight attack squares
	BBattacks = BBknightLookup[square];
	// get attack squares that have colour knights on them
	BBattacks = BBattacks & (pos->pieces[KNIGHT] & pos->colours[colour]);
	if (BBattacks) {
		//piece being attacked by knight
		return 1;
	}
	// rook attacks
	U64 BBoccupancy = pos->colours[WHITE] | pos->colours[BLACK];
	BBattacks = Rmagic(square,BBoccupancy);
	BBattacks = BBattacks & ~pos->colours[!colour]; // mask out non colour pieces
	BBattacks = BBattacks & (pos->pieces[ROOK] | pos->pieces[QUEEN]) & pos->colours[colour];
	if (BBattacks) {
		//piece being attacked by rook
		return 1;
	}
	// bishop attacks
	BBattacks = Bmagic(square,BBoccupancy);
	BBattacks = BBattacks & ~pos->colours[!colour]; // mask out black pieces
	BBattacks = BBattacks & (pos->pieces[BISHOP] | pos->pieces[QUEEN]) & pos->colours[colour];
	if (BBattacks) {
		//piece being attacked by bishop
		return 1;
	}
	/*
	// queen attacks
	BBattacks = Bmagic(square,BBoccupancy) | Rmagic(square,BBoccupancy);
	BBattacks = BBattacks & ~pos->colours[!colour]; // mask out black pieces
	BBattacks = BBattacks & (pos->pieces[QUEEN] & pos->colours[colour]);
	if (BBattacks) {
		//piece being attacked by queen
		return 1;
	}
	 */
	return 0;
}

U64 BBkingattacks(U64 BBpiece) {
	// get king attack squares
	U64 BBattacks = eastOne(BBpiece) | westOne(BBpiece); // east and west one
	BBpiece |= BBattacks; // set piece BB to attacks
	BBattacks |= northOne(BBpiece) | southOne(BBpiece); // north, south, nw, ne, sw, se one
	return BBattacks;
}
U64 BBknightattacks(U64 BBpiece) {
	assert(__builtin_popcountll(BBpiece) >= 0);
	U64 BBattacks = noNoWe(BBpiece);
	BBattacks |= noNoEa(BBpiece);
	BBattacks |= noEaEa(BBpiece);
	BBattacks |= soEaEa(BBpiece);
	BBattacks |= soSoEa(BBpiece);
	BBattacks |= soSoWe(BBpiece);
	BBattacks |= soWeWe(BBpiece);
	BBattacks |= noWeWe(BBpiece);
	return BBattacks;
}
U64 BBpawnattacksWFlipped(U64 BBpiece) {
	return BBpawnEastAttacksWFlipped(BBpiece) | BBpawnWestAttacksWFlipped(BBpiece);
}
U64 BBpawnEastAttacksWFlipped(U64 wpawns) {return soEaOne(wpawns);}
U64 BBpawnWestAttacksWFlipped(U64 wpawns) {return soWeOne(wpawns);}

U64 BBpawnattacksBFlipped(U64 BBpiece) {
	return BBpawnEastAttacksBFlipped(BBpiece) | BBpawnWestAttacksBFlipped(BBpiece);
}
U64 BBpawnEastAttacksBFlipped(U64 bpawns) {return noEaOne(bpawns);}
U64 BBpawnWestAttacksBFlipped(U64 bpawns) {return noWeOne(bpawns);}


U64 BBpawnattacksW(U64 BBpiece) {
	return BBpawnEastAttacksW(BBpiece) | BBpawnWestAttacksW(BBpiece);
}
U64 BBpawnEastAttacksW(U64 wpawns) {return noEaOne(wpawns);}
U64 BBpawnWestAttacksW(U64 wpawns) {return noWeOne(wpawns);}

U64 BBpawnattacksB(U64 BBpiece) {
	return BBpawnEastAttacksB(BBpiece) | BBpawnWestAttacksB(BBpiece);
}
U64 BBpawnEastAttacksB(U64 bpawns) {return soEaOne(bpawns);}
U64 BBpawnWestAttacksB(U64 bpawns) {return soWeOne(bpawns);}

U64 BBpawnSinglePushW(U64 BBwpawns, U64 BBunoccupied) {
   return northOne(BBwpawns) & BBunoccupied;
}

U64 BBpawnDoublePushW(U64 BBwpawns, U64 BBunoccupied) {
   U64 BBsinglePushes = BBpawnSinglePushW(BBwpawns, BBunoccupied);
   return northOne(BBsinglePushes) & BBunoccupied & BBrank4;
}

U64 BBpawnSinglePushB(U64 BBbpawns, U64 BBunoccupied) {
   return southOne(BBbpawns) & BBunoccupied;
}

U64 BBpawnDoublePushB(U64 BBbpawns, U64 BBunoccupied)  {
   U64 BBsinglePushes = BBpawnSinglePushB(BBbpawns, BBunoccupied);
   return southOne(BBsinglePushes) & BBunoccupied & BBrank5;
}