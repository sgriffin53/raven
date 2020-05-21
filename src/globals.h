#ifndef GLOBALS_H
#define GLOBALS_H

#include "position.h"

int wtime, btime;
int origwtime, origbtime;
int movestogo;
U64 BBkingLookup[64];
U64 BBknightLookup[64];
U64 BBpasserLookup[2][64];
U64 BBkingfillLookup2[64];
U64 BBkingfillLookup3[64];
U64 BBkingfillLookup4[64];
U64 BBpawnshieldLookup[2][64];
U64 BBconnectedLookup[2][64];
U64 BBinbetweenLookup[64][64];

#endif