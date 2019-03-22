/*
 * functions.h
 *
 *  Created on: Aug 27, 2017
 *      Author: Antar
 */


#ifndef FUNCTIONS_H
#define FUNCTIONS_H

// Macro functions
#define FR2SQ64(f, r) (((r) << 3) | (f))
#define FR2SQ120(f,r) (((r)+2)*10 + (f) + 1)
#define RANK(sq) ((sq) >> 3)
#define FILE(sq) ((sq) & 7)
#define RANK120(sq) ((sq) / 10 - 2) 
#define FILE120(sq) ((sq) % 10 - 1)
#define SQ64TO120(sq) ((((sq) >> 3)+2)*10 + ((sq) & 7) + 1)
#define SQ120TO64(sq) (FR2SQ64(FILE120(sq), RANK120(sq)))
#define isPawn(piece) (piece == 1 || piece == 7)
#define isKnight(piece) (piece == 2 || piece == 8)
#define isBishop(piece) (piece == 3 || piece == 9)
#define isRook(piece) (piece == 4 || piece == 10)
#define isQueen(piece) (piece == 5 || piece == 11)
#define isKing(piece) (piece == 6 || piece == 12)
#define pieceColor(piece) ((piece > 0 && piece <= 6) ? 0 : 1)
#define isSlidePiece(piece) (isBishop(piece) | isRook(piece) | isQueen(piece))
#define onBoard(square) (square > 20 && square < 99 && square % 10 > 0 && square % 10 < 9 ? 1 : 0)
#define pieceValid(piece) (piece > 0 && piece < 13 ? 1 : 0)
#define uci_print(...); printf(__VA_ARGS__); fflush(stdout);

// board.c
extern void clearBoard(Board *board);
extern void printBoard(Board *board);
extern void parseFEN(char *fen, Board *board);
extern void updatePieceList(Board *board);
extern void printPV(PV *pv);

// move.c
extern void printMove(Move move);
extern int parseMove(Board *board, char *move, Move *m);

// movegen.c
extern void generateMoves(Board *board, MoveList *list);
extern void generateCaptures(Board *board, MoveList *list);

// makemove.c
//extern int makeMove(Board *board, Move move);
extern void takeMove(Board *board);

// evaluation.c
extern int evaluate(Board *board);

// search.c

//extern void search(Board *board, Search *search, PV *pv);
extern int count_nps(long nodes, long long time);

// order.c

int mvvlva(int victim, int attacker);
int compareMoves(const void *m1, const void *m2);
int moveScore(Board *board, Move move);

// attacks.c

extern int isAttacked(Board *board, int square, int turn);

// hash.c

extern void initZobrist();
extern void setBoardHash(Board *board);
extern void initTTable(TTable *table);
extern void clearTTable(TTable *table);
extern int probeTT(Board *board, Move *move);
extern void storeTT(Board *board, Move move);
extern U64 generateHash(struct position *pos);

// uci.c

extern void uci_loop(Board *board, Search *search);
extern long long current_timestamp();
extern void readInput(Search *search);
extern void perft_divide(Board *board, int depth);

#endif