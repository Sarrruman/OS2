#pragma once
#include "fs.h"
#include <bitset>
#include <windows.h>

using namespace std;

#ifndef wait 
#define wait(x) WaitForSingleObject(x, INFINITE)
#endif
#ifndef signal
#define signal(x) ReleaseSemaphore(x,1,NULL)
#endif

#define CLUSTER_SIZE 2048
#define BIT_VECTOR_ENTRIES 16384
#define INDEX_ENTRIES 512
#define ROOT_ENTRIES_IN_CLASTER 102 // +8 bytes

typedef unsigned long ClusterPointer;

struct rootEntriesCluster {
	Entry entries[ROOT_ENTRIES_IN_CLASTER];
	char reserved[8];
};
