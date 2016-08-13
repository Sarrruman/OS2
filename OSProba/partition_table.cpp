#include "partition_table.h"

MPartitionTable::MPartitionTable()
{
	mutex_MPTable = CreateMutex(NULL, FALSE, TEXT("mutex_MPTable"));
}

MPartitionTable::~MPartitionTable()
{
	CloseHandle(mutex_MPTable);
}

char MPartitionTable::mount(Partition* partition) { // returns 0 if there is no free space
	wait(mutex_MPTable); // the thread has to have exlusive rights to modify Mounting table
	int free_position = 26;
	char res;
	for (int i = 0; i < 26; i++) {
		if (partitions[i] == nullptr) {
			free_position = i; break;
		}
	}
	if (free_position != 26) {
		res = 'A' + free_position;
		partitions[free_position] = new MPartition(partition, res);
	}
	else {
		res = 0;
	}
	signal(mutex_MPTable);
	return res;
}

char MPartitionTable::unmount(char partition)
{
	wait(mutex_MPTable);
	if (partitions[partition - 'A'] == nullptr || unmounting_or_formating[partition - 'A'])
	{
		signal(mutex_MPTable);
		return 0; // partition doesn't exist or is being unmounted or formatted
	}
	MPartition* p = partitions[partition - 'A'];
	unmounting_or_formating[partition - 'A'] = true;
	signal(mutex_MPTable);

	p->waitForOpenedFiles();

	wait(mutex_MPTable); // delete MPartition entry in partitions table
	partitions[partition - 'A'] = nullptr;
	unmounting_or_formating[partition - 'A'] = false;

	//save bit-vector
	p->partition->writeCluster(0, (char*)&(p->bitVector.bitVector));
	delete p;
	signal(mutex_MPTable);

	return 1; // success
}

char MPartitionTable::format(char partition)
{
	wait(mutex_MPTable);
	if (partitions[partition - 'A'] == nullptr || unmounting_or_formating[partition - 'A'])
	{
		signal(mutex_MPTable);
		return 0; // partition doesn't exist or is being unmounted or formatted
	}
	MPartition* p = partitions[partition - 'A'];
	unmounting_or_formating[partition - 'A'] = true;
	signal(mutex_MPTable);

	p->waitForOpenedFiles();

	wait(mutex_MPTable); // format partition
	unmounting_or_formating[partition - 'A'] = false;
	p->bitVector.bitVector.reset();
	p->bitVector.bitVector.set(0); // bit-vector space allocation
	p->bitVector.bitVector.set(1); // allocating index cluster
	//p->partition->writeCluster(0, (char*)&(p->bitVector.bitVector)); // not needed because bitvector is kept in 
																	// memory until unmounting
	//write index
	ClusterPointer index[INDEX_ENTRIES] = { 0 };
	int res = p->partition->writeCluster(1, (char*)index);

	signal(mutex_MPTable);

	return 1; // success
}