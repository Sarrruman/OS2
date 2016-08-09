#include"part.h"

#include<iostream>
#include<fstream>
#include<cstdlib>
#include<windows.h>
#include<ctime>

#define signal(x) ReleaseSemaphore(x,1,NULL)
#define wait(x) WaitForSingleObject(x,INFINITE)

using namespace std;

struct ProbaClaster {
	char entries[2048];
};

void proba1();
void proba2();
void proba3();
int main() {
	proba1();
	
	system("PAUSE");
	return 0;
}

void proba1() {
	Partition* partition1 = new Partition("p1.ini");

	//for (int i = 0; i < 1050; i++) {
	//	ProbaClaster* novi = new ProbaClaster();
	//	int res = partition1->writeCluster(i, (const char*) novi);
	//	cout << res;
	//	delete novi;
	//}

	cout << "Broj klastera je : " << partition1->getNumOfClusters();

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
	char * ptr = "test";
	cout << ptr[1] << "\n";
	cout  << "\n" << (typeid(TRUE).name());
}