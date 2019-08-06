SOURCES = chess.c position.c hash.c makemove.c tests.c movegen.c attacks.c bitboards.c magicmoves.c perft.c search.c eval.c TT.c sort.c misc.c
DFLAGS  = -lm -Wpedantic -Wall -Wextra -Wshadow
RFLAGS  = -lm -flto -Ofast -DNDEBUG
release:
	gcc $(SOURCES) $(RFLAGS) -o raven
debug:
	gcc $(SOURCES) $(DFLAGS) -o raven-debug
fast:
	gcc $(SOURCES) -o raven-fast