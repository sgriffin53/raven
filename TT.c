#include <assert.h>
#include <stdlib.h>
#include "hash.h"
#include "TT.h"
#include "move.h"
#include "globals.h"

void initETT(struct ETTtable *table) {
	assert(table);
	int ETTsizemb = 32;
	const int totentries = (ETTsizemb*1024*1024) / sizeof(struct PTTentry);
	table->entries = malloc(totentries * sizeof(struct ETTentry));
	table->totentries = totentries;
}

struct ETTentry getETTentry(struct ETTtable *table,U64 hash) {
	int index = hash % table->totentries;
	return table->entries[index];
}

void addETTentry(struct ETTtable *table,U64 hash, int eval) {
	int index = hash % table->totentries;
	struct ETTentry newentry;
	newentry.hash = hash;
	newentry.eval = eval;
	table->entries[index] = newentry;
}

void initPTT(struct PTTtable *table) {
	assert(table);
	int PTTsizemb = 32;
	const int totentries = (PTTsizemb*1024*1024) / sizeof(struct PTTentry);
	table->entries = malloc(totentries * sizeof(struct PTTentry));
	table->totentries = totentries;
}

struct PTTentry getPTTentry(struct PTTtable *table,U64 hash) {
	int index = hash % table->totentries;
	return table->entries[index];
}

void addPTTentry(struct PTTtable *table,U64 hash, int depth,U64 nodes) {
	int index = hash % table->totentries;
	struct PTTentry newentry;
	newentry.hash = hash;
	newentry.depth = depth;
	newentry.nodes = nodes;
	table->entries[index] = newentry;
	assert(newentry.hash == getPTTentry(table,hash).hash);
	assert(newentry.depth == getPTTentry(table,hash).depth);
	assert(newentry.nodes == getPTTentry(table,hash).nodes);
}

void initTT(struct TTtable *table) {
	assert(table);
	int PTTsizemb = hashsize;
	const int totentries = (PTTsizemb*1024*1024) / sizeof(struct TTentry);
	table->entries = malloc(totentries * sizeof(struct TTentry));
	table->totentries = totentries;
}
void clearTT(struct TTtable *table) {
	for (int i = 0;i < table->totentries;i++) {
		table->entries[i].hash = 0;
		table->entries[i].depth = 0;
		table->entries[i].flag = 0;
		table->entries[i].score = 0;
	}
}
void addTTentry(struct TTtable *table,U64 hash, int depth,int flag,struct move bestmove, int score) {
	int index = hash % table->totentries;
	struct TTentry newentry;
	newentry.hash = hash;
	newentry.depth = depth;
	newentry.flag = flag;
	newentry.bestmove = bestmove;
	newentry.score = score;
	table->entries[index] = newentry;
}

struct TTentry getTTentry(struct TTtable *table,U64 hash) {
	int index = hash % table->totentries;
	return table->entries[index];
}