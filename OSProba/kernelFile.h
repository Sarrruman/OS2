#pragma once
#include "fs.h"
#include "global_file.h"

class KernelFile {
public:
	KernelFile(GlobalFile* gFile, char mode);
	~KernelFile(); //zatvaranje fajla
	char write(BytesCnt, char* buffer);
	BytesCnt read(BytesCnt, char* buffer);
	char seek(BytesCnt);
	BytesCnt filePos();
	char eof();
	BytesCnt getFileSize();
	char truncate();

	//pointer to the entry in the global files table
	GlobalFile* global;
	char mode; // r, w or a
	BytesCnt position; // points one location after last accessed, 0 if wasn't used
};