#ifndef HASH_H
#define HASH_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "position.h"

#define EMPTY 0

typedef unsigned long long U64;

void initZobrist();
U64 generateHash(const struct position *pos);

#endif
