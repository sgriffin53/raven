#include "bitboards.h"
#include "hash.h"
#include "position.h"

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