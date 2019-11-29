#ifndef HASH_H
#define HASH_H

#include "position.h"

typedef unsigned long long U64;

extern void initZobrist();
extern int pieceintval(char inpiece, int colour);
extern U64 generateHash(struct position *pos);

#endif