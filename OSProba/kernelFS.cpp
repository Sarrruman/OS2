#include "kernelFS.h"
#include "file.h"
#include <list>

bool compare(const Entry&, char*);

KernelFS::KernelFS()
{
	mutex_GFTable = CreateMutex(NULL, FALSE, TEXT("mutex_GFTable"));
}

KernelFS::~KernelFS()
{
	CloseHandle(mutex_GFTable);
}

char KernelFS::mount(Partition * partition)
{
	return partition_table.mount(partition);
}

char KernelFS::unmount(char part)
{
	return partition_table.unmount(part);
}

char KernelFS::format(char part)
{
	return partition_table.format(part);
}

char KernelFS::readRootDir(char part, EntryNum n, Directory & d)
// reading of unformatted partition will cause errors
{
	// check if part is valid input: 
	if (part < 'A' || part > 'Z') return 0;
	wait(partition_table.mutex_MPTable);

	if (partition_table.partitions[part - 'A'] != nullptr &&
		partition_table.unmounting_or_formating[part - 'A'] != 1) {
		Partition* partition = partition_table.partitions[part - 'A']->partition;
		signal(partition_table.mutex_MPTable);

		// read root index
		ClusterPointer cluster_to_be_read;
		ClusterPointer index[INDEX_ENTRIES];
		partition->readCluster(1, (char*)index);

		int cluster_number = 0; // used for keeping track of read clusters with root entries logically
		int number_of_valid_entries = 0;
		while (true) {
			// read cluster with entries
			if (cluster_number < INDEX_ENTRIES / 2) {
				cluster_to_be_read = index[cluster_number];
				if (cluster_to_be_read == 0) goto no_more_entries;
			}
			else {
				if ((cluster_number - INDEX_ENTRIES / 2) / 512 == INDEX_ENTRIES) goto no_more_entries;
				ClusterPointer index_to_be_read = index[(cluster_number - INDEX_ENTRIES / 2) / 512];
				if (index_to_be_read == 0) goto no_more_entries;

				//read second level index
				ClusterPointer index_level_2[INDEX_ENTRIES];
				partition->readCluster(index_to_be_read, (char*)index_level_2);
				cluster_to_be_read = index_level_2[(cluster_number - INDEX_ENTRIES / 2) % INDEX_ENTRIES];
				if (cluster_to_be_read == 0) goto no_more_entries;
			}
			RootEntriesCluster entry = RootEntriesCluster();
			partition->readCluster(cluster_to_be_read, (char *)entry.entries);
			for (int i = 0; i < ROOT_ENTRIES_IN_CLASTER; i++) {
				if (entry.entries[i].name[0] != 0) { // first byte of the free entry is 0x00				
					++number_of_valid_entries;
					if ((number_of_valid_entries - 1) >= n) {
						if ((number_of_valid_entries - n) == 65) goto no_more_entries;
						d[(number_of_valid_entries - 1) - n] = entry.entries[i];
					}
				}
			}
			cluster_number++;
		}
	no_more_entries:
		return number_of_valid_entries - n;
	}
	else {
		signal(partition_table.mutex_MPTable);
		return 0;
	}
}

char KernelFS::findFile(char* fname, Entry& e, RootEntriesCluster& cluster, ClusterPointer& pointer, int& entryNumber)
{
	// extract name and extension
	regex r("^[A-Z]:\\\\[a-zA-Z0-9]\\{1,8\\}\\..\\{1,3\\}$", regex::basic);
	if (!regex_match(fname, r)) return 0;
	char part = fname[0];
	wait(partition_table.mutex_MPTable);

	if (partition_table.partitions[part - 'A'] != nullptr &&
		partition_table.unmounting_or_formating[part - 'A'] != 1) {
		Partition* partition = partition_table.partitions[part - 'A']->partition;
		signal(partition_table.mutex_MPTable);

		// read root index
		ClusterPointer cluster_to_be_read;
		ClusterPointer index[INDEX_ENTRIES];
		partition->readCluster(1, (char*)index);

		int cluster_number = 0; // used for keeping track of read clusters with root entries logically
		while (true) {
			// read cluster with entries
			if (cluster_number < INDEX_ENTRIES / 2) {
				cluster_to_be_read = index[cluster_number];
				if (cluster_to_be_read == 0) goto no_more_entries;
			}
			else {
				if ((cluster_number - INDEX_ENTRIES / 2) / 512 == INDEX_ENTRIES) goto no_more_entries;
				ClusterPointer index_to_be_read = index[(cluster_number - INDEX_ENTRIES / 2) / 512];
				if (index_to_be_read == 0) goto no_more_entries;

				//read second level index
				ClusterPointer index_level_2[INDEX_ENTRIES];
				partition->readCluster(index_to_be_read, (char*)index_level_2);
				cluster_to_be_read = index_level_2[(cluster_number - INDEX_ENTRIES / 2) % INDEX_ENTRIES];
				if (cluster_to_be_read == 0) goto no_more_entries;
			}
			partition->readCluster(cluster_to_be_read, (char *)cluster.entries);
			for (int i = 0; i < ROOT_ENTRIES_IN_CLASTER; i++) {
				if (compare(cluster.entries[i], fname)) {
					e = cluster.entries[i];

					pointer = cluster_to_be_read;
					entryNumber = i;
					return 1;
				}
			}
			cluster_number++;
		}
	no_more_entries:
		return 0;
	}
	else {
		signal(partition_table.mutex_MPTable);
		return 0;
	}
	return 0;
}

char KernelFS::doesExist(char * fname)
{
	Entry e = Entry();
	RootEntriesCluster cluster = RootEntriesCluster();
	ClusterPointer pointer;
	int entryNumber;
	return findFile(fname, e, cluster, pointer, entryNumber);
}

File * KernelFS::open(char * fname, char mode)
{
	// extract name and extension
	regex r("^[A-Z]:\\\\[a-zA-Z0-9]\\{1,8\\}\\..\\{1,3\\}$", regex::basic);
	if (!regex_match(fname, r)) return nullptr;
	char part = fname[0];

	// check if partition exists and is not formatting or unmounting
	wait(partition_table.mutex_MPTable);
	if (partition_table.partitions[part - 'A'] == nullptr ||
		partition_table.unmounting_or_formating[part - 'A'] == true) {
		signal(partition_table.mutex_MPTable);
		return 0;
	}
	else {
		// increment the number of opened files atomically
		MPartition* mPartition = partition_table.partitions[part - 'A'];
		EnterCriticalSection(&mPartition->cSection_MP);
		mPartition->openedFilesCnt++;
		LeaveCriticalSection(&mPartition->cSection_MP);
		signal(partition_table.mutex_MPTable);

		wait(mutex_GFTable);
		// find file in global files table
		map<string, GlobalFile*>::iterator it;
		it = global_files_table.find(string(fname));
		if (it != global_files_table.end()) { // if found in global files table
			GlobalFile* gFile = it->second;
			wait(gFile->mutex_GF);
			gFile->waitingRWCnt++;
			signal(gFile->mutex_GF);
			signal(mutex_GFTable);

			gFile->block(mode);

			wait(mutex_GFTable);
			wait(gFile->mutex_GF);
			gFile->waitingRWCnt--;
			gFile->numOfReferences--;
			signal(mutex_GFTable);
			signal(gFile->mutex_GF);

			File* f = new File();
			f->myImpl = new KernelFile(gFile, mode);
			return f;
		} else { // if not found in global files table
			Entry entry = Entry();
			RootEntriesCluster cluster = RootEntriesCluster();
			ClusterPointer pointer;
			int entryNumber;
			if (findFile(fname, entry, cluster, pointer, entryNumber)) { // if found in root directory
				if (mode == 'w') {
					deleteFileContents(mPartition, entry, cluster, pointer, entryNumber);
					GlobalFile* gFile = new GlobalFile(mPartition, entry);
					global_files_table.insert(pair<string, GlobalFile*>(fname, gFile));
					signal(mutex_GFTable);
					gFile->block(mode);
					File* f = new File();
					f->myImpl = new KernelFile(gFile, mode);
					return f;
				}
				else if (mode == 'r') {
					GlobalFile* gFile = new GlobalFile(mPartition, entry);
					signal(mutex_GFTable);
					gFile->block(mode);
					File* f = new File();
					f->myImpl = new KernelFile(gFile, mode);
					return f;
				}
				else if (mode == 'a') {
					GlobalFile* gFile = new GlobalFile(mPartition, entry);
					signal(mutex_GFTable);
					gFile->block(mode);
					File* f = new File();
					f->myImpl = new KernelFile(gFile, mode);
					f->myImpl->pointer = entry.size; // points 1 position after last full entry
					return f;
				}
				else {
					return nullptr;
				}
			}
			else { // if not found in root directory
				if (mode == 'w') {
					char part_pom;
					parse(fname, part_pom, entry.name, entry.ext);
					if (!createEntryInRoot(entry, mPartition)) {
						signal(mutex_GFTable);
						return nullptr; // no free space
					}
					GlobalFile* gFile = new GlobalFile(mPartition, entry);
					signal(mutex_GFTable);
					gFile->block(mode);
					File* f = new File();
					f->myImpl = new KernelFile(gFile, mode);
					return f;
				}
				else { // error, openedFilesCnt has to be refreshed (rollback)
					signal(mutex_GFTable);
					wait(partition_table.mutex_MPTable);
					EnterCriticalSection(&mPartition->cSection_MP);
					mPartition->openedFilesCnt--;
					bool flag = false;
					if (mPartition->openedFilesCnt == 0) flag = true;
					LeaveCriticalSection(&mPartition->cSection_MP);
					if (flag) WakeConditionVariable(&mPartition->cond_openFiles);
					signal(partition_table.mutex_MPTable);

					return nullptr;
				}
			}
		}
	}
}

char KernelFS::deleteFile(char * fname)
{
	return 0;
}

void KernelFS::deleteFileContents(MPartition* mPartition, Entry& entry, RootEntriesCluster& cluster, ClusterPointer pointer, int entryNumber)
{
	// gather pointers to file content
	list<ClusterPointer> pointers;
	ClusterPointer index[INDEX_ENTRIES];	// read first-level index
	mPartition->partition->readCluster(entry.indexCluster, (char*)index);

	pointers.push_back(entry.indexCluster); // index cluster needs to be released also
	bool end_reached = false;
	for (int i = 0; i < INDEX_ENTRIES / 2; i++) {
		if (index[i] != 0) pointers.push_back(index[i]);
		else {
			end_reached = true; break;
		}
	}
	if (!end_reached) {
		for (int i = INDEX_ENTRIES/2 + 1; i < INDEX_ENTRIES; i++) {
			if (index[i] != 0 && !end_reached) {
				pointers.push_back(index[i]);
				// read second-level index
				ClusterPointer index_level2[INDEX_ENTRIES];
				mPartition->partition->readCluster(index[i], (char*)index_level2);

				for (int j = 0; j < INDEX_ENTRIES; j++) {
					if (index_level2[j] != 0) {
						pointers.push_back(index_level2[j]);
					}
					else {
						end_reached = true; break;
					}
				}
			}
			else {
				end_reached = true; break;
			}
		}
	}
	// deleting
	entry.size = 0;
	cluster.entries[entryNumber].size = 0;
	cluster.entries[entryNumber].indexCluster = 0;
	mPartition->partition->writeCluster(pointer, (char*)&cluster);
	wait(mPartition->mutex_bit_vector);
	for (list<ClusterPointer>::iterator it = pointers.begin(); it != pointers.end(); it++) {
		mPartition->bitVector.bitVector.reset(*it);
	}
	signal(mPartition->mutex_bit_vector);
}

bool KernelFS::createEntryInRoot(const Entry & entry, MPartition * partition)
{
	// find empty space for entry
	ClusterPointer cluster_to_be_read;
	ClusterPointer index[INDEX_ENTRIES];
	partition->partition->readCluster(1, (char*)index);

	int cluster_number = 0; // used for keeping track of read clusters with root entries logically
	while (true) {
		// read cluster with entries
		if (cluster_number < INDEX_ENTRIES / 2) {
			cluster_to_be_read = index[cluster_number];
			if (cluster_to_be_read == 0) goto no_more_entries;
		}
		else {
			if ((cluster_number - INDEX_ENTRIES / 2) / 512 == INDEX_ENTRIES) goto no_more_entries;
			ClusterPointer index_to_be_read = index[(cluster_number - INDEX_ENTRIES / 2) / 512];
			if (index_to_be_read == 0) goto no_more_entries;

			//read second level index
			ClusterPointer index_level_2[INDEX_ENTRIES];
			partition->partition->readCluster(index_to_be_read, (char*)index_level_2);
			cluster_to_be_read = index_level_2[(cluster_number - INDEX_ENTRIES / 2) % INDEX_ENTRIES];
			if (cluster_to_be_read == 0) goto no_more_entries;
		}
		RootEntriesCluster tek_entry = RootEntriesCluster();
		partition->partition->readCluster(cluster_to_be_read, (char *)tek_entry.entries);
		for (int i = 0; i < ROOT_ENTRIES_IN_CLASTER; i++) {
			if (tek_entry.entries[i].name[0] == 0) { // first byte of the free entry is 0x00				
				tek_entry.entries[i] = entry;
				partition->partition->writeCluster(cluster_to_be_read, (char *)tek_entry.entries);
				return true;
			}
		}
		cluster_number++;
	}
no_more_entries:
	if (cluster_number < INDEX_ENTRIES / 2) {
		// find one free cluster for entries and allocate it
		wait(partition->mutex_bit_vector);
		ClusterPointer free = partition->bitVector.findFree();
		if (free == 0) {
			signal(partition->mutex_bit_vector);
			return false;
		}
		partition->bitVector.bitVector[free] = 1;
		signal(partition->mutex_bit_vector);

		RootEntriesCluster root_entry_pom = RootEntriesCluster(); // every entry will be initialized with zeros, as free
		root_entry_pom.entries[0] = entry;
		int res1 = partition->partition->writeCluster(free, (char*)& root_entry_pom);
		index[cluster_number] = free;

		char * pok = (char*)& index;
		int res = partition->partition->writeCluster(1, (char*) index);

		return true;
	}
	else {
		// find two free clusters for second level index and entry cluster
		wait(partition->mutex_bit_vector);
		ClusterPointer free1 = partition->bitVector.findFree();
		if (free1 == 0) {
			signal(partition->mutex_bit_vector);
			return false;		
		}
		ClusterPointer free2 = partition->bitVector.findFree();
		if (free2 == 0) { // free2 for entries
			signal(partition->mutex_bit_vector);
			return false;
		}
		partition->bitVector.bitVector[free1] = partition->bitVector.bitVector[free2]= 1;
		signal(partition->mutex_bit_vector);

		RootEntriesCluster root_entry_pom = RootEntriesCluster(); // every entry will be initialized with zeros, as free
		root_entry_pom.entries[0] = entry;
		partition->partition->writeCluster(free2, (char*)& root_entry_pom);

		// create second level index
		ClusterPointer second_level_ind[INDEX_ENTRIES] = { 0 };
		second_level_ind[0] = free2;
		partition->partition->writeCluster(free1, (char*)& second_level_ind);

		index[(cluster_number - INDEX_ENTRIES / 2) / 512 == INDEX_ENTRIES] = free1;
		partition->partition->writeCluster(1, (char*)& index);

		return true;
	}
}
