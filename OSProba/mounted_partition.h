#pragma once
#include <Windows.h>
#include "part.h"
#include <bitset>
#include "utilities.h"

class MPartition {
private:
public:
	Partition* partition;
	char name;
	int openedFilesCnt;
	CRITICAL_SECTION cSection_MP;  // thread that owns critical section can enter it again without blocking
	CONDITION_VARIABLE cond_openFiles;
	HANDLE mutex_bit_vector;
	BitVector bitVector;

	// -------------------------------------------------
	MPartition(Partition*, char);
	~MPartition();

	void waitForOpenedFiles();
};
