SOURCES = chess.c position.c search.c attacks.c movegen_white.c movegen_black.c hash.c eval.c makemove.c globals.c TT.c misc.c tests.c perft.c
DFLAGS  = -Wpedantic -Wall -Wextra -Wshadow
RFLAGS  = -flto -Ofast -DNDEBUG
release:
	gcc $(RFLAGS) $(SOURCES) -o raven
debug:
	gcc $(DFLAGS) $(SOURCES) -o raven-debug