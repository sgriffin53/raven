#ifndef BITBOARDS_H
#define BITBOARDS_H

#include "hash.h"

extern const U64 BBfileA;
extern const U64 BBfileB;
extern const U64 BBfileAB;
extern const U64 BBfileC;
extern const U64 BBfileD;
extern const U64 BBfileD;
extern const U64 BBfileE;
extern const U64 BBfileF;
extern const U64 BBfileG;
extern const U64 BBfileH;
extern const U64 BBfileGH;
extern const U64 BBrank1;
extern const U64 BBrank2;
extern const U64 BBrank3;
extern const U64 BBrank4;
extern const U64 BBrank5;
extern const U64 BBrank6;
extern const U64 BBrank7;
extern const U64 BBrank8;

extern const U64 BBcentre;
extern const U64 BBbigcentre;

extern const U64 BBdiagA8H1;
extern const U64 BBdiagA1H8;

extern const U64 BBdarksquares;
extern const U64 BBlightsquares;

U64 arrFiles[8];


U64 northOne(U64 BB);
U64 southOne(U64 BB);
U64 westOne(U64 BB);
U64 eastOne(U64 BB);

U64 noEaOne(U64 BB);
U64 noWeOne(U64 BB);
U64 noEaOne(U64 BB);
U64 noWeOne(U64 BB);
U64 noNoEa(U64 BB);
U64 noEaEa(U64 BB);
U64 soEaEa(U64 BB);
U64 soSoEa(U64 BB);
U64 noNoWe(U64 BB);
U64 noWeWe(U64 BB);
U64 soWeWe(U64 BB);
U64 soSoWe(U64 BB);

U64 soEaOne(U64 BB);
U64 soWeOne(U64 BB);
U64 noEaOne(U64 BB);
U64 noWeOne(U64 BB);

void genLookups();

#endif