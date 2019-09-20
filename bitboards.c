#include "bitboards.h"
#include "hash.h"
#include "position.h"
#include "attacks.h"
#include "globals.h"
#include "magicmoves.h"
#include <stdlib.h>
#include "movegen.h"

typedef unsigned long long U64;

const U64 BBfileA = 0b00000001\
00000001\
00000001\
00000001\
00000001\
00000001\
00000001\
00000001;

const U64 BBfileB = BBfileA << 1;
const U64 BBfileC = BBfileA << 2;
const U64 BBfileD = BBfileA << 3;
const U64 BBfileE = BBfileA << 4;
const U64 BBfileF = BBfileA << 5;
const U64 BBfileG = BBfileA << 6;
const U64 BBfileH = BBfileA << 7;

				
const U64 BBfileAB = BBfileA | BBfileB;

const U64 BBfileGH = BBfileG | BBfileH;

const U64 BBrank1 = 0b00000000\
00000000\
00000000\
00000000\
00000000\
00000000\
00000000\
11111111;

const U64 BBrank2 = BBrank1 << (8*1);
const U64 BBrank3 = BBrank1 << (8*2);
const U64 BBrank4 = BBrank1 << (8*3);
const U64 BBrank5 = BBrank1 << (8*4);
const U64 BBrank6 = BBrank1 << (8*5);
const U64 BBrank7 = BBrank1 << (8*6);
const U64 BBrank8 = BBrank1 << (8*7);

const U64 BBcentre = 0b00000000\
00000000\
00000000\
00011000\
00011000\
00000000\
00000000\
00000000;

const U64 BBbigcentre = 0b00000000\
00000000\
00111100\
00111100\
00111100\
00111100\
00000000\
00000000;

const U64 BBdiagA8H1 = 0b10000000\
01000000\
00100000\
00010000\
00001000\
00000100\
00000010\
00000001;

const U64 BBdiagA1H8 = 0b00000001\
00000010\
00000100\
00001000\
00010000\
00100000\
01000000\
10000000;

const U64 BBlightsquares = 0b10101010\
01010101\
10101010\
01010101\
10101010\
01010101\
10101010\
01010101;

const U64 BBdarksquares = ~BBlightsquares;

U64 northOne(U64 BB) {
	return BB << 8;
}
U64 southOne(U64 BB) {
	return BB >> 8;
}
U64 westOne(U64 BB) {
	return (BB >> 1) & ~BBfileH;
	//else return 0ULL;
}
U64 eastOne(U64 BB) {
	return (BB << 1) & ~BBfileA;
}

U64 noNoEa(U64 BB) {return (BB << 17) & ~BBfileA;}
U64 noEaEa(U64 BB) {return (BB << 10) & ~BBfileAB;}
U64 soEaEa(U64 BB) {return (BB >>  6) & ~BBfileAB;}
U64 soSoEa(U64 BB) {return (BB >> 15) & ~BBfileA;}
U64 noNoWe(U64 BB) {return (BB << 15) & ~BBfileH;}
U64 noWeWe(U64 BB) {return (BB <<  6) & ~BBfileGH;}
U64 soWeWe(U64 BB) {return (BB >> 10) & ~BBfileGH;}
U64 soSoWe(U64 BB) {return (BB >> 17) & ~BBfileH;}

U64 soEaOne(U64 BB) {
	return (BB >> 7) & ~BBfileA;
}
U64 soWeOne(U64 BB) {
	return (BB >> 9) & ~BBfileH;
}
U64 noEaOne(U64 BB) {
	return (BB << 9) & ~BBfileA;
}
U64 noWeOne(U64 BB) {
	return (BB << 7) & ~BBfileH;
}
void genLookups() {
	for (int i = 0; i < 64;i++) {
		BBkingLookup[i] = BBkingattacks(1ULL << i);
		BBknightLookup[i] = BBknightattacks(1ULL << i);
		
		// generate passed pawn lookups
		
		// white
		
		int rank = getrank(i);
		U64 BBpiece = (1ULL << i);
		U64 BBmidsquare = (1ULL << i);
		U64 BBchecksquares = 0ULL;
		
		while (rank < 6) {
			BBchecksquares |= noWeOne(BBmidsquare);
			BBchecksquares |= northOne(BBmidsquare);
			BBchecksquares |= noEaOne(BBmidsquare);
			BBmidsquare = northOne(BBmidsquare);
			rank++;
		}
		BBpasserLookup[WHITE][i] = BBchecksquares;
		
		// black
		
		BBmidsquare = (1ULL << i);
		BBchecksquares = 0ULL;
		rank = getrank(i);
		while (rank > 1) {
			BBchecksquares |= soWeOne(BBmidsquare);
			BBchecksquares |= southOne(BBmidsquare);
			BBchecksquares |= soEaOne(BBmidsquare);
			BBmidsquare = southOne(BBmidsquare);
			rank--;
		}
		BBpasserLookup[BLACK][i] = BBchecksquares;
		
		// king fills
		
		// kinglookup is 1 away
		
		BBkingfillLookup2[i] = BBkingattacks(BBkingLookup[i]);
		BBkingfillLookup3[i] = BBkingattacks(BBkingfillLookup2[i]);
		BBkingfillLookup4[i] = BBkingattacks(BBkingfillLookup3[i]);
		
		// pawn shields
		
		U64 BBpawnshield = noWeOne(1ULL << i) | northOne(1ULL << i) | noEaOne(1ULL << i);
		BBpawnshield |= northOne(BBpawnshield);
		BBpawnshieldLookup[WHITE][i] = BBpawnshield;
		
		BBpawnshield = soWeOne(1ULL << i) | southOne(1ULL << i) | soEaOne(1ULL << i);
		BBpawnshield |= southOne(BBpawnshield);
		BBpawnshieldLookup[BLACK][i] = BBpawnshield;
	}
	initInBetween();

}
void initInBetween() {
	for (int i = 0; i < 64; ++i) {
		for (int j = 0; j < 64; ++j) {
			if (i == j)
				continue;

			BBinbetweenLookup[i][j] = 0;

			if (i % 8 == j % 8 || i / 8 == j / 8) {
				BBinbetweenLookup[i][j] = Rmagic(i, bitmask[j]) & Rmagic(j, bitmask[i]);
				continue;
			}

			if (abs(j % 8 - i % 8) == abs(j / 8 - i / 8)) {
				BBinbetweenLookup[i][j] = Bmagic(i, bitmask[j]) & Bmagic(j, bitmask[i]);;
				continue;
			}


		}
	}
}