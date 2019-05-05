#ifndef PERFT_H
#define PERFT_H

#include <assert.h>
#include <inttypes.h>
#include "hash.h"
#include "position.h"
#include "move.h"
#include "movegen.h"
#include "makemove.h"
#include "attacks.h"

U64 perft(struct position *pos, int depth);

U64 splitperft(struct position *pos, int depth);

#endif
