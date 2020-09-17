SOURCES = chess.c position.c hash.c makemove.c tests.c movegen.c attacks.c bitboards.c magicmoves.c perft.c search.c eval.c TT.c sort.c misc.c tuner/tune.c tuner/tune_eval.c
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