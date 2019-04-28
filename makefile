SOURCES = chess.c position.c search.c attacks.c movegen_white.c movegen_black.c hash.c eval.c makemove.c globals.c
DFLAGS  = -Wpedantic -Wall -Wextra -Wshadow
RFLAGS  = -march=native -flto -Ofast -DNDEBUG
release:
	gcc $(RFLAGS) $(SOURCES) -o raven
debug:
	gcc $(DFLAGS) $(SOURCES) -o raven-debug