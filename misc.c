#ifndef MISC_C
#define MISC_C

#include "misc.h"

int min(int a, int b) {
	if (a < b) return a;
	return b;
}
int max(int a, int b) {
	if (a > b) return a;
	return b;
}

#endif