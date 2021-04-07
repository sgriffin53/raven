SOURCES = src/main.c src/chess/position.c src/chess/hash.c src/chess/makemove.c src/tests.c src/chess/movegen.c src/chess/attacks.c src/chess/bitboards.c src/chess/magicmoves.c src/search/perft.c src/search/search.c src/search/eval.c src/search/TT.c src/search/sort.c src/misc.c src/uci.c
DFLAGS  = -lm -static -fcommon -Wpedantic -Wall -Wextra -Wshadow
RFLAGS  = -lm -static -fcommon -flto -Ofast -DNDEBUG
TFLAGS = -lm -static -fcommon -flto -Ofast -DNDEBUG -Wl,--stack,2000000000
release:
	gcc $(SOURCES) $(RFLAGS) -o raven
debug:
	gcc $(SOURCES) $(DFLAGS) -o raven-debug
fast:
	gcc $(SOURCES) -o raven-fast
tune:
	gcc $(SOURCES) $(TFLAGS) -o raven-tuner