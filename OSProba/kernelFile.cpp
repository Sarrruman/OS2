#include "kernelFile.h"


KernelFile::KernelFile(GlobalFile * gFile, char mode) 
{
	global_file = gFile; 
	this->mode = mode;
}

KernelFile::~KernelFile()
{
}

char KernelFile::write(BytesCnt, char * buffer)
{
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
