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

#endif