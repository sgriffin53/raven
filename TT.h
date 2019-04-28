#ifndef TT_H
#define TT_H

#include <assert.h>
#include <stdlib.h>
#include "hash.h"

struct TTentry {
	U64 hash;
	struct position pos;
	int depth;
	int score;
};

struct PTTentry {
	U64 hash;
	int depth;
	U64 nodes;
};

struct ETTentry {
	U64 hash;
	int eval;
};

struct PTTtable {
	struct PTTentry *entries;
	int totentries;
};

struct TTtable {
	struct TTentry *entries;
	int totentries;
};

struct ETTtable {
	struct ETTentry *entries;
	int totentries;
};

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
	int PTTsizemb = 32;
	const int totentries = (PTTsizemb*1024*1024) / sizeof(struct TTentry);
	table->entries = malloc(totentries * sizeof(struct TTentry));
	table->totentries = totentries;
}

void addTTentry(struct TTtable *table,U64 hash, struct position pos,int depth,int score) {
	int index = hash % table->totentries;
	struct TTentry newentry;
	newentry.hash = hash;
	newentry.pos = pos;
	newentry.depth = depth;
	newentry.score = score;
	table->entries[index] = newentry;
}

struct TTentry getTTentry(struct TTtable *table,U64 hash) {
	int index = hash % table->totentries;
	return table->entries[index];
}

#endif
