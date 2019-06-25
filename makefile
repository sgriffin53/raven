SOURCES = chess.c position.c hash.c makemove.c tests.c movegen.c attacks.c bitboards.c magicmoves.c perft.c search.c eval.c TT.c sort.c
DFLAGS  = -Wpedantic -Wall -Wextra -Wshadow
RFLAGS  = -flto -Ofast -DNDEBUG
release:
	gcc $(RFLAGS) $(SOURCES) -o raven
debug:
	gcc $(DFLAGS) $(SOURCES) -o raven-debug
fast:
	gcc $(SOURCES) -o raven-fast