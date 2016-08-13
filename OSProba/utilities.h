#pragma once
#include "fs.h"
#include <bitset>
#include <windows.h>

using namespace std;

#ifndef wait 
#define wait(x) WaitForSingleObject(x, INFINITE)
#endif
#ifndef signal
#define signal(x) ReleaseMutex(x)
#endif

#define CLUSTER_SIZE 2048
#define BIT_VECTOR_ENTRIES 16384
#define INDEX_ENTRIES 512
#define ROOT_ENTRIES_IN_CLASTER 102 // +8 bytes

typedef unsigned long ClusterPointer;

void parse(char* fname, char& part, char* name, char* ext);
bool compare(const Entry& entry, char* fname);

class OutOfMemory {
};

struct RootEntriesCluster {
	Entry entries[ROOT_ENTRIES_IN_CLASTER];
	char reserved[8];
};

struct BitVector {
	bitset<BIT_VECTOR_ENTRIES> bitVector;
	unsigned long sizeOfPartition; // number of clusters

	ClusterPointer findFree() const;

	BitVector(unsigned long);
};