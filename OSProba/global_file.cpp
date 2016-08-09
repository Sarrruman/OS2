#include "global_file.h"

GlobalFile::GlobalFile(MPartition* partition, const Entry& entry) : partition(partition), rootEntry(entry)
{
	numOfReferences = waitingRWCnt = readersCnt = 0;
	mutex_GF = CreateMutex(NULL, FALSE, NULL);
	mutex_R = CreateMutex(NULL, FALSE, NULL);
	mutex_RW = CreateMutex(NULL, FALSE, NULL);
}

GlobalFile::~GlobalFile()
{
	CloseHandle(mutex_GF);
	CloseHandle(mutex_R);
	CloseHandle(mutex_RW);
}