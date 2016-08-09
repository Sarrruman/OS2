#include "kernelFS.h"

KernelFS::KernelFS()
{
	mutex_GFTable = CreateMutex(NULL, FALSE, NULL);
}

KernelFS::~KernelFS()
{
	CloseHandle(mutex_GFTable);
}

char KernelFS::mount(Partition * partition)
{
	return 0;
}

char KernelFS::unmount(char part)
{
	return 0;
}

char KernelFS::format(char part)
{
	return 0;
}

char KernelFS::readRootDir(char part, EntryNum n, Directory & d)
{
	return 0;
}

char KernelFS::doesExist(char * fname)
{
	return 0;
}

File * KernelFS::open(char * fname, char mode)
{
	return nullptr;
}

char KernelFS::deleteFile(char * fname)
{
	return 0;
}
