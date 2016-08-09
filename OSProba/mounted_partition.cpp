#include "mounted_partition.h"

MPartition::MPartition(Partition* part, char name) {
	this->partition = part;
	this->name = name;
	openedFilesCnt = 0;
	InitializeCriticalSection(cSection_MP);
	InitializeConditionVariable(cond_openFiles);
}
inline MPartition::~MPartition()
{
	DeleteCriticalSection(cSection_MP);
	delete cond_openFiles;
}

inline void MPartition::waitForOpenedFiles()
{
	EnterCriticalSection(cSection_MP);
	while (openedFilesCnt != 0) {
		SleepConditionVariableCS(cond_openFiles, cSection_MP, INFINITE);
	}
	LeaveCriticalSection(cSection_MP);
}