#pragma once
#include "utilities.h"
#include "mounted_partition.h"

class GlobalFile {
public:
	Entry rootEntry;
	ClusterPointer firstIndex[INDEX_ENTRIES] = { 0 }; // --------------------->
														// when saving to partition check if it is modified (first entry != 0)
	bool indexRead;
	MPartition* partition;
	int numOfReferences; // number of references from the local files
	int waitingRWCnt; // threads waiting to open this file
	HANDLE mutex_GF; // Used for securing accesss to numOfReferences and waitingRWCnt variables
	int readersCnt; // for readers-writters algorithm
	HANDLE mutex_R, mutex_RW; // for readers-writters algorithm

	//caching
	unsigned long file_size;
	ClusterNo pointer_to_cluster; //0 if there is no cached cluster
	char cached_cluster[CLUSTER_SIZE];

	bool readOrCreateIndex();
	void block(char mode); // wait for others to close the file if necessary
	GlobalFile(MPartition * partition, const Entry & entry);
	~GlobalFile();
};