#ifndef MISC_C
#define MISC_C

#include "misc.h"
#include <string.h>
#include <ctype.h>
#include <assert.h>

void strtolwr(char* str) {
	assert(str);
	for(int i = 0; str[i]; i++){
	  str[i] = tolower(str[i]);
	}
}
/*
char *strtolwr(char *str)
{
  unsigned char *p = (unsigned char *)str;

  while (*p) {
     *p = tolower((unsigned char)*p);
      p++;
  }

  return str;
}
 */

int min(int a, int b) {
	if (a < b) return a;
	return b;
}
int max(int a, int b) {
	if (a > b) return a;
	return b;
}

#endif