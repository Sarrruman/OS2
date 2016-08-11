#include "global_file.h"

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
	numOfReferences = waitingRWCnt = readersCnt = 0;
	mutex_GF = CreateMutex(NULL, FALSE, NULL);
	mutex_R = CreateMutex(NULL, FALSE, NULL);
	mutex_RW = CreateMutex(NULL, FALSE, NULL);

	pointer_to_cluster = NULL;
	file_size = entry.size;
}

GlobalFile::~GlobalFile()
{
	CloseHandle(mutex_GF);
	CloseHandle(mutex_R);
	CloseHandle(mutex_RW);
}