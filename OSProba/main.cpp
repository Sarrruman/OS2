#include"part.h"

#include "kernelFS.h"
#include<iostream>
#include<fstream>
#include<cstdlib>
#include<windows.h>
#include<ctime>
#include <regex>

bool compare(const Entry&, char*);

using namespace std;

struct ProbaClaster {
	char entries[2048];
};

void proba1();
void proba2();
void proba3();
int main() {
	proba3();

	system("PAUSE");
	return 0;
}

void proba1() {
	Partition* partition1 = new Partition("p1.ini");

	char p1 = FS::mount(partition1);
	int res = FS::doesExist("A:\\fajl1.dat");
	// cout << "Broj klastera je : " << partition1->getNumOfClusters();
	//char filepath[] = "1:\\fajl1.dat";
	//filepath[0] = p1;
	//File* f = FS::open(filepath, 'w');

	delete partition1;
}

//kreiranje .dat fajla
void proba2() {
	char niz[2050];
	ofstream f("disk2.dat");

	for (int i = 0; i < 1000; i++) {
		f << niz;
	}

	f.close();
}

void proba3() {
	char* file = new char[11]{ 'X', ':', '\\', 't', 'i', '3', '.', 't', 'x', 'x', '\0' };
	char* file2 = new char[11]{ 'X', ':', '\\', 't', 'i', '3', '.', 't', 'x', 'x', '\0' };
	
	ClusterPointer niz[3];

	cout << sizeof(BitVector);
	cout << "\n" << (typeid(TRUE).name());
}