#ifndef GLOBALS_H
#define GLOBALS_H

#include "TT.h"

int hashsize;
struct position posstack[1024];
int posstackend;
U64 nodesSearched;
U64 currenthash;
struct TTtable TT;
struct ETTtable ETT;
int numinstantbetacutoffs;
int numbetacutoffs;
int hashsize;
struct move killers[128][2];
int history[2][64][64];
int butterfly[2][64][64];
struct move pvArray[1024];
int rootdepth;
int wtime, btime;
int origwtime, origbtime;
int lastsearchdepth;
struct move movestack[1024];
int movestackend;
struct move countermoves[64][64];
U64 hashstack[1024];
int hashstackend;

#endif