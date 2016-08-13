#include "global_file.h"

bool GlobalFile::readOrCreateIndex()
{
	if (indexRead) return true;
	if (rootEntry.indexCluster == 0) {
		wait(partition->mutex_bit_vector);
		ClusterPointer free = partition->bitVector.findFree();
		if (free == 0) {
			signal(partition->mutex_bit_vector);
			return 0;
		}
		partition->bitVector.bitVector.set(free);
		signal(partition->mutex_bit_vector);

		rootEntry.indexCluster = free;
		indexRead = true;
		return true;
	}
	else {
		partition->partition->readCluster(rootEntry.indexCluster, (char*)firstIndex);
		indexRead = true;
		return true;
	}
}

void GlobalFile::block(char mode)
{
	if (mode == 'r') {
		wait(mutex_R);
		readersCnt++;
		if (readersCnt == 1) wait(mutex_RW);
		signal(mutex_R);
	}
	else {
		wait(mutex_RW);
	}
}

GlobalFile::GlobalFile(MPartition* partition, const Entry& entry) : partition(partition), rootEntry(entry)
{
	indexRead = false;
	pointerToIndex = entry.indexCluster;
	numOfReferences = waitingRWCnt = readersCnt = 0;
	mutex_GF = CreateMutex(NULL, FALSE, TEXT("mutex_GF"));
	mutex_R = CreateMutex(NULL, FALSE, TEXT("mutex_R"));
	mutex_RW = CreateMutex(NULL, FALSE, TEXT("mutex_RW"));

	pointer_to_cluster = NULL;
	file_size = entry.size;
}

GlobalFile::~GlobalFile()
{
	CloseHandle(mutex_GF);
	CloseHandle(mutex_R);
	CloseHandle(mutex_RW);
}