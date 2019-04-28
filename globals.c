#include "globals.h"
#include "TT.h"
#include "hash.h"

struct position posstack[1024];
int posstackend = 0;
int nodesSearched = 0;
struct TTtable TT;
U64 currenthash;