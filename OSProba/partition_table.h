#pragma once
#include <windows.h>
#include "mounted_partition.h"
#include "utilities.h"

class MPartitionTable { // M for Mounted
public:
	HANDLE mutex_MPTable;

	MPartition* partitions[26] = { nullptr };
	bool unmounting_or_formating[26] = { false };
	MPartitionTable();
	~MPartitionTable();

	char mount(Partition* partition);
	char unmount(char partition);
	char format(char part);
};



