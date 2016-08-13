#pragma once

#include <map>
#include "global_file.h"
#include "partition_table.h"
#include <regex>
#include "kernelFile.h"

bool compare(const Entry& entry, char* fname);
void parse(char* fname, char& part, char* name, char* ext);

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
	char readRootDir(char part, EntryNum n, Directory &d); // result can be 0 because the partition doesn't exist, or is unmounting
									// or formatting, or there are no entries after the specified entry number in the root directory
									//prvim argumentom se zadaje particija, drugim redni broj
									//validnog ulaza od kog se počinje čitanje

	char findFile(char* fname, Entry& entry, RootEntriesCluster& cluster, ClusterPointer& pointer, int& entryNumber); // entry will be populated with root entry content if found
																// cluster will contain cluster which contains found entry
																// pointer would point to the cluster on partition
	char doesExist(char* fname); //argument je naziv fajla sa
									   //apsolutnom putanjom
	File* open(char* fname, char mode);
	char deleteFile(char* fname);

	// global open files table 
	map<string, GlobalFile*> global_files_table;
	HANDLE mutex_GFTable;

	// mounted partitions table
	MPartitionTable partition_table;

private:
	void deleteFileContents(MPartition* mPartition, Entry& entry, RootEntriesCluster& cluster, ClusterPointer pointer, int entryNumber);
	bool createEntryInRoot(const Entry& entry, MPartition* partition); // returns false if there is no free space
};