#ifndef TUNE_H
#define TUNE_H


#include "../position.h"

struct tunerEntry {
	struct position pos;
	float score;
};
float sigmoid(const float s, float k);
float E(struct tunerEntry *entries, int numentries, float k);
void tune();
void findK();

#endif