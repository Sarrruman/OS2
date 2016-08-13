#include "mounted_partition.h"

MPartition::MPartition(Partition* part, char name) : bitVector(part->getNumOfClusters()) {
	this->partition = part;
	this->name = name;
	openedFilesCnt = 0;
	InitializeCriticalSection(&cSection_MP);
	InitializeConditionVariable(&cond_openFiles);
	mutex_bit_vector = CreateMutex(NULL, FALSE, TEXT("mutex_bit_vector"));
	part->readCluster(0, (char *) &(bitVector.bitVector));
}
MPartition::~MPartition()
{
	DeleteCriticalSection(&cSection_MP);
	CloseHandle(mutex_bit_vector);
}

void MPartition::waitForOpenedFiles()
{
	EnterCriticalSection(&cSection_MP);
	while (openedFilesCnt != 0) {
		SleepConditionVariableCS(&cond_openFiles, &cSection_MP, INFINITE);
	}
	LeaveCriticalSection(&cSection_MP);
}