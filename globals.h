#ifndef GLOBALS_H
#define GLOBALS_H

#include "TT.h"
#include <time.h>

int hashsize;
struct position posstack[1024];
int posstackend;
U64 nodesSearched;
U64 currenthash;
struct TTtable TT;
struct ETTtable ETT;
struct PVTTtable PVTT;
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
int movestogo;
int lastsearchdepth;
struct move movestack[1024];
int movestackend;
struct move countermoves[64][64];
U64 hashstack[1024];
int hashstackend;
int seldepth;
int lastreduce;
U64 BBkingLookup[64];
U64 BBknightLookup[64];
U64 BBpasserLookup[2][64];
U64 BBkingfillLookup2[64];
U64 BBkingfillLookup3[64];
U64 BBkingfillLookup4[64];
U64 BBpawnshieldLookup[2][64];
U64 BBconnectedLookup[2][64];
U64 BBinbetweenLookup[64][64];
int movecount[1024];
int numgoodhist;
U64 totNodesSearched;
int silentsearch;
clock_t totalendtime;
U64 rootNodesSearched[64][64];
U64 lastRootNodesSearched[64][64];
struct move rootmove;

#endif