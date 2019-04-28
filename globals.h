#ifndef GLOBALS_H
#define GLOBALS_H

//#include "TT.h"
#include "position.h"
#include "TT.h"
#include "hash.h"

extern struct TTtable TT;
//struct PTTtable PTT;
//struct ETTtable ETT;
extern struct position posstack[1024];
extern int posstackend;
extern int nodesSearched;
extern U64 currenthash;

#endif
