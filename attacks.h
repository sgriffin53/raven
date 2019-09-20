#ifndef ATTACKS_H
#define ATTACKS_H

#include "position.h"
#include "hash.h"

int isCheck(struct position *pos);
int isAttacked(struct position *pos,int square, int colour);
U64 BBkingattacks(U64 BB);
U64 BBknightattacks(U64 BBpiece);
U64 BBpawnattacksWFlipped(U64 BBpiece);
U64 BBpawnEastAttacksWFlipped(U64 wpawns);
U64 BBpawnWestAttacksWFlipped(U64 wpawns);
U64 BBpawnattacksBFlipped(U64 BBpiece);
U64 BBpawnEastAttacksBFlipped(U64 bpawns);
U64 BBpawnWestAttacksBFlipped(U64 bpawns);
U64 BBpawnattacksB(U64 BBpiece);
U64 BBpawnEastAttacksB(U64 bpawns);
U64 BBpawnWestAttacksB(U64 bpawns);
U64 BBpawnattacksW(U64 BBpiece);
U64 BBpawnEastAttacksW(U64 bpawns);
U64 BBpawnWestAttacksW(U64 bpawns);
U64 BBpawnSinglePushW(U64 BBwpawns, U64 BBunoccupied);
U64 BBpawnDoublePushW(U64 BBwpawns, U64 BBunoccupied);
U64 BBpawnSinglePushB(U64 BBbpawns, U64 BBunoccupied);
U64 BBpawnDoublePushB(U64 BBwpawns, U64 BBunoccupied);

U64 BBallkingattacks(struct position *pos, int side);
U64 BBpawnattacks(struct position *pos, int side);
U64 BBallknightattacks(struct position *pos, int side);
U64 BBqueenAttacks(int square, U64 BBoccupied);
U64 BBrookAttacks(int square, U64 BBoccupied);
U64 BBbishopAttacks(int square, U64 BBoccupied);
U64 BBslidingAttacks(struct position *pos, U64 (*movesFunc)(int, U64), U64 BB);

U64 wCaptRightPawn (const U64 bb, const U64 opPieces);
U64 bCaptRightPawn (const U64 bb, const U64 opPieces);
U64 wCaptLeftPawn  (const U64 bb, const U64 opPieces);
U64 bCaptLeftPawn  (const U64 bb, const U64 opPieces);

#endif