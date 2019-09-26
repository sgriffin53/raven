#ifndef BITBOARDS_H
#define BITBOARDS_H

#include "hash.h"

const U64 BBfileA;
const U64 BBfileB;
const U64 BBfileAB;
const U64 BBfileC;
const U64 BBfileD;
const U64 BBfileD;
const U64 BBfileE;
const U64 BBfileF;
const U64 BBfileG;
const U64 BBfileH;
const U64 BBfileGH;
const U64 BBrank1;
const U64 BBrank2;
const U64 BBrank3;
const U64 BBrank4;
const U64 BBrank5;
const U64 BBrank6;
const U64 BBrank7;
const U64 BBrank8;

const U64 BBcentre;
const U64 BBbigcentre;

const U64 BBdiagA8H1;
const U64 BBdiagA1H8;

const U64 BBdarksquares;
const U64 BBlightsquares;

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
void initInBetween();
#endif