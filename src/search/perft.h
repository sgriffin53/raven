#ifndef PERFT_H
#define PERFT_H

#include <assert.h>
#include <inttypes.h>
#include "../chess/hash.h"
#include "../chess/position.h"
#include "../chess/move.h"
#include "../chess/movegen.h"
#include "../chess/makemove.h"
#include "../chess/attacks.h"

U64 perft(struct position *pos, int depth);

U64 splitperft(struct position *pos, int depth);

#endif