#pragma once
#include "clusters.h"
#include "mounted_partition.h"

class GlobalFile {
public:
	Entry rootEntry;
	MPartition* partition;
	int numOfReferences; // number of references from the local files
	int waitingRWCnt; // threads waiting to open this file
	HANDLE mutex_GF; // Used for securing accesss to numOfReferences and waitingRWCnt variables
	int readersCnt; // for readers-writters algorithm
	HANDLE mutex_R, mutex_RW; // for readers-writters algorithm

	GlobalFile(MPartition * partition, const Entry & entry);
	~GlobalFile();
};