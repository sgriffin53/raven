#ifndef GLOBALS_C
#define GLOBALS_C

#include "globals.h"
#include "position.h"
#include "TT.h"

struct position posstack[1024];
int posstackend = 0;
nodesSearched = 0;
currenthash = 0;
struct TTtable TT;
struct ETTtable ETT;
hashsize = 32;
int numinstantbetacutoffs;
int numbetacutoffs;

#endif