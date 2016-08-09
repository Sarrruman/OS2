#pragma once
#include "part.h"
#include "fs.h"
#include <map>
#include "global_file.h"

class KernelFS {

public:
	KernelFS();
	~KernelFS();
	char mount(Partition* partition); //montira particiju
											 // vraca dodeljeno slovo
	char unmount(char part); //demontira particiju oznacenu datim
								   // slovom vraca 0 u slucaju neuspeha ili 1 u slucaju uspeha
	char format(char part); //particija zadatu slovom se formatira;
								  // vraca 0 u slucaju neuspeha ili 1 u slucaju uspeha
	char readRootDir(char part, EntryNum n, Directory &d);

	//prvim argumentom se zadaje particija, drugim redni broj
	//validnog ulaza od kog se počinje čitanje
	char doesExist(char* fname); //argument je naziv fajla sa
									   //apsolutnom putanjom
	File* open(char* fname, char mode);
	char deleteFile(char* fname);

	// global open files table 
	map<string, GlobalFile*> global_files_table;
	HANDLE mutex_GFTable;
};