#include <stdio.h>

#include "attacks.h"
#include "bitboards.h"
#include "hash.h"
#include "magicmoves.h"

int isCheck(struct position *pos) {
	if (pos->tomove == WHITE) return isAttacked(pos, pos->Wkingpos, !pos->tomove);
	else return isAttacked(pos, pos->Bkingpos, !pos->tomove);
}
int isAttacked(struct position *pos,int square, int colour) {
	assert(pos);
	assert(square >= 0 && square <= 63);
	assert(colour == WHITE || colour == BLACK);
	// colour is colour of attacking side
	if (colour == WHITE) {
		// white is attacking black
		// white king
		// check if black is being attacked by white king
		U64 BBpiece = (1ULL << square); // get bitboard of piece on square
		// get king attack squares
		U64 BBattacks = BBkingattacks(BBpiece);
		BBattacks = BBattacks & (pos->BBkings & pos->BBwhitepieces);
		if (BBattacks) {
			// black piece being attacked by white king
			return 1;
		}
		// get knight attack squares
		BBattacks = BBknightattacks(BBpiece);
		// get attack squares that have white knights on them
		BBattacks = BBattacks & (pos->BBknights & pos->BBwhitepieces);
		if (BBattacks) {
			// black piece being attacked by white knight
			return 1;
		}
		// pawn attacks
		BBattacks = BBpawnattacksWFlipped(BBpiece);
		BBattacks = BBattacks & (pos->BBpawns & pos->BBwhitepieces);
		//dspBB(BBattacks);
		if (BBattacks) {
			//black piece being attacked by white pawn
			return 1;
		}
		// rook attacks
		U64 BBoccupancy = pos->BBwhitepieces | pos->BBblackpieces;
		BBattacks = Rmagic(square,BBoccupancy);
		BBattacks = BBattacks & ~pos->BBblackpieces; // mask out black pieces
		BBattacks = BBattacks & (pos->BBrooks & pos->BBwhitepieces);
		if (BBattacks) {
			//black piece being attacked by white rook
			return 1;
		}
		// bishop attacks
		BBattacks = Bmagic(square,BBoccupancy);
		BBattacks = BBattacks & ~pos->BBblackpieces; // mask out black pieces
		BBattacks = BBattacks & (pos->BBbishops & pos->BBwhitepieces);
		if (BBattacks) {
			//black piece being attacked by white bishop
			return 1;
		}
		// queen attacks
		BBattacks = Bmagic(square,BBoccupancy) | Rmagic(square,BBoccupancy);
		BBattacks = BBattacks & ~pos->BBblackpieces; // mask out black pieces
		BBattacks = BBattacks & (pos->BBqueens & pos->BBwhitepieces);
		if (BBattacks) {
			//black piece being attacked by white queen
			return 1;
		}
	}
	else if (colour == BLACK) {
		// black is attacking white
		// black king
		// check if white is being attacked by black king
		U64 BBpiece = (1ULL << square); // get bitboard of piece on square
		// get king attack squares
		U64 BBattacks = BBkingattacks(BBpiece);
		BBattacks = BBattacks & (pos->BBkings & pos->BBblackpieces);
		if (BBattacks) {
			// white piece being attacked by black king
			return 1;
		}
		// get knight attack squares
		BBattacks = BBknightattacks(BBpiece);
		// get attack squares that have black knights on them
		BBattacks = BBattacks & (pos->BBknights & pos->BBblackpieces);
		if (BBattacks) {
			// white piece being attacked by black knight
			return 1;
		}
		BBattacks = BBpawnattacksBFlipped(BBpiece);
		BBattacks = BBattacks & (pos->BBpawns & pos->BBblackpieces);
		if (BBattacks) {
			//white piece being attacked by black pawn
			return 1;
		}
		U64 BBoccupancy = pos->BBwhitepieces | pos->BBblackpieces;
		BBattacks = Rmagic(square,BBoccupancy);
		BBattacks = BBattacks & ~pos->BBwhitepieces; // mask out white pieces
		BBattacks = BBattacks & (pos->BBrooks & pos->BBblackpieces);
		if (BBattacks) {
			//black piece being attacked by white rook
			return 1;
		}
		BBattacks = Bmagic(square,BBoccupancy);
		BBattacks = BBattacks & ~pos->BBwhitepieces; // mask out white pieces
		BBattacks = BBattacks & (pos->BBbishops & pos->BBblackpieces);
		if (BBattacks) {
			//black piece being attacked by white bishop
			return 1;
		}
		// queen attacks
		BBattacks = Bmagic(square,BBoccupancy) | Rmagic(square,BBoccupancy);
		BBattacks = BBattacks & ~pos->BBwhitepieces; // mask out white pieces
		BBattacks = BBattacks & (pos->BBqueens & pos->BBblackpieces);
		if (BBattacks) {
			//black piece being attacked by white queen
			return 1;
		}
	}
	return 0;
}

U64 BBkingattacks(U64 BBpiece) {
	//assert(__builtin_popcountll(BBpiece) == 1);
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