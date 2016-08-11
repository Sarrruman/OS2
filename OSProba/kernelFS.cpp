#include "kernelFS.h"

bool compare(const Entry&, char*);

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
		partition->readCluster(2, (char*)index);

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
			partition->readCluster(cluster_to_be_read, (char *) entry.entries);
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

char KernelFS::findFile(char * fname, Entry & e)
{
	// extract name and extension
	if (regex_match(fname, regex("^[A-Z]:\\(.|\d){1,8}\..{1,3}$"))) return 0;
	char part = fname[0];
	wait(partition_table.mutex_MPTable);

	if (partition_table.partitions[part - 'A'] != nullptr &&
		partition_table.unmounting_or_formating[part - 'A'] != 1) {
		Partition* partition = partition_table.partitions[part - 'A']->partition;
		signal(partition_table.mutex_MPTable);

		// read root index
		ClusterPointer cluster_to_be_read;
		ClusterPointer index[INDEX_ENTRIES];
		partition->readCluster(2, (char*)index);

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
			RootEntriesCluster entry = RootEntriesCluster();
			partition->readCluster(cluster_to_be_read, (char *)entry.entries);
			for (int i = 0; i < ROOT_ENTRIES_IN_CLASTER; i++) {
				if (compare(entry.entries[i], fname)) {			
					e = entry.entries[i];
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
	return findFile(fname, e);
}

File * KernelFS::open(char * fname, char mode)
{
	// extract name and extension
	if (regex_match(fname, regex("^[A-Z]:\\(.|\d){1,8}\..{1,3}$"))) return nullptr;
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
		EnterCriticalSection(mPartition->cSection_MP);
		mPartition->openedFilesCnt++;
		LeaveCriticalSection(mPartition->cSection_MP);
		signal(partition_table.mutex_MPTable);

		wait(mutex_GFTable);
		// find file in global files table
		map<string, GlobalFile*>::iterator it;
		it = global_files_table.find(string(fname));
		if (it != global_files_table.end()) { // if found
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
			
			// --------------------------------------------------------->
			File* f = new File();
			f->my
			return new File(f);
		}
		else {

		}

	}
	
}

char KernelFS::deleteFile(char * fname)
{
	return 0;
}
