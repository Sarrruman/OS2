#include "kernelFile.h"


KernelFile::KernelFile(GlobalFile * gFile, char mode) 
{
	global = gFile; 
	this->mode = mode;
	position = 0;
}

KernelFile::~KernelFile()
{
}

char KernelFile::write(BytesCnt num, char * buffer)
{
	if (num <= 0) return 0;
	if (mode == 'r') return 0;

	global->readOrCreateIndex();
	int new_bytes_to_be_allocated = num - (global->file_size - position);
	if (new_bytes_to_be_allocated < 0) new_bytes_to_be_allocated = 0;

	int num_of_bytes_in_current = CLUSTER_SIZE - position % CLUSTER_SIZE;
	if (num_of_bytes_in_current < num) num_of_bytes_in_current = num;

	int clusters_to_be_allocated = (num - num_of_bytes_in_current) / CLUSTER_SIZE + (position % CLUSTER_SIZE == 0) ? 1 : 0
		+ ((num - num_of_bytes_in_current) % CLUSTER_SIZE != 0) ? 1 : 0;

	int num_of_level2_indexes_to_allocate = 

	return 0;
}

BytesCnt KernelFile::read(BytesCnt, char * buffer)
{
	return BytesCnt();
}

char KernelFile::seek(BytesCnt)
{
	return 0;
}

BytesCnt KernelFile::filePos()
{
	return BytesCnt();
}

char KernelFile::eof()
{
	return 0;
}

BytesCnt KernelFile::getFileSize()
{
	return BytesCnt();
}

char KernelFile::truncate()
{
	return 0;
}
