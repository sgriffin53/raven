#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "../chess/hash.h"
#include "TT.h"
#include "../chess/move.h"
#include "../globals.h"
#include "../chess/position.h"

void initETT(struct ETTtable *table) {
	assert(table);
	int ETTsizemb = 32;
	const int totentries = (ETTsizemb*1024*1024) / sizeof(struct ETTentry);
	table->entries = malloc(totentries * sizeof(struct ETTentry));
	table->totentries = totentries;
	assert(totentries > 0);
}
void initPVTT(struct PVTTtable *table) {
	assert(table);
	int PVTTsizemb = 32;
	const int totentries = (PVTTsizemb*1024*1024) / sizeof(struct PVTTentry);
	table->entries = malloc(totentries * sizeof(struct PVTTentry));
	table->totentries = totentries;
	assert(totentries > 0);
}
struct PVTTentry getPVTTentry(struct PVTTtable *table, U64 hash) {
	assert(table);
	assert(table->entries);
	assert(table->totentries > 0);
	int index = hash % table->totentries;
	return table->entries[index];
}
void addPVTTentry(struct PVTTtable *table, U64 hash, struct move bestmove, int score) {
	assert(table);
	assert(table->entries);
	assert(table->totentries > 0);
	int index = hash % table->totentries;
	struct PVTTentry newentry;
	newentry.hash = hash;
	newentry.bestmove = bestmove;
	newentry.score = score;
	table->entries[index] = newentry;
}
struct ETTentry getETTentry(struct ETTtable *table, U64 hash) {
	assert(table);
	assert(table->entries);
	assert(table->totentries > 0);
	int index = hash % table->totentries;
	return table->entries[index];
}

void addETTentry(struct ETTtable *table, U64 hash, int eval) {
	assert(table);
	assert(table->entries);
	assert(table->totentries > 0);
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

struct PTTentry getPTTentry(struct PTTtable *table, U64 hash) {
	assert(table);
	assert(table->entries);
	assert(table->totentries > 0);
	int index = hash % table->totentries;
	return table->entries[index];
}

void addPTTentry(struct PTTtable *table,U64 hash, int depth,U64 nodes) {
	assert(table);
	assert(table->entries);
	assert(table->totentries > 0);
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
	int TTsizemb = hashsize;
	const int totentries = (TTsizemb*1024*1024) / sizeof(struct TTentry);
	table->entries = malloc(totentries * sizeof(struct TTentry));
	table->totentries = totentries;
	assert(table->totentries > 0);
}
void clearTT(struct TTtable *table) {
	for (int i = 0;i < table->totentries;i++) {
		table->entries[i].hash = 0;
		table->entries[i].depth = 0;
		table->entries[i].flag = 0;
		table->entries[i].score = 0;
	}
}
void clearETT(struct ETTtable *table) {
	for (int i = 0;i < table->totentries;i++) {
		table->entries[i].hash = 0;
		table->entries[i].eval = 0;
	}
}
void clearPVTT(struct PVTTtable *table) {
	for (int i = 0;i < table->totentries;i++) {
		struct move nomove = {.to=-1, .from=-1, .prom=-1 };
		table->entries[i].bestmove = nomove;
		table->entries[i].score = -999999;
	}
}
void addTTentry(struct TTtable *table,U64 hash, int depth, int flag, struct move bestmove, int score) {
	assert(table);
	assert(table->totentries > 0);
	int index = hash % table->totentries;
	struct TTentry newentry;
	newentry.hash = hash;
	newentry.depth = depth;
	newentry.flag = flag;
	newentry.bestmove = bestmove;
	newentry.score = score;
	table->entries[index] = newentry;
}

struct TTentry getTTentry(struct TTtable *table, U64 hash) {
	assert(table);
	assert(table->totentries > 0);
	assert(table->entries);
	int index = hash % table->totentries;
	return table->entries[index];
}