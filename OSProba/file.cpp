#include "file.h"
#include "kernelFile.h"

File::File() {
}

File::~File()
{
}

char File::write(BytesCnt c, char * buffer)
{
	return myImpl->write(c, buffer);
}

BytesCnt File::read(BytesCnt c, char * buffer)
{
	return myImpl->read(c, buffer);
}

char File::seek(BytesCnt c)
{
	return myImpl->seek(c);
}

BytesCnt File::filePos()
{
	return myImpl->filePos();
}

char File::eof()
{
	return myImpl->eof();
}

BytesCnt File::getFileSize()
{
	return myImpl->getFileSize();
}

char File::truncate()
{
	return myImpl->truncate();
}
