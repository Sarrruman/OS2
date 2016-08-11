// test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include<cstdlib>
using namespace std;

class Proba {
public:
	Proba() {}
	int i = 0;
};

void proba3() {
	char * ptr = "test";
	unsigned long * niz = new unsigned long[3];
	unsigned long niz1[3];
	niz1[0] = 0xbcdeff;
	ptr = (char *)niz1;

	Proba p = Proba();


	niz[0] = 0xABCDEF;
	ptr = (char *)niz;
	cout << "\n" << "" ;
}

int main()
{
	proba3();

	system("PAUSE");
	return 0;
}

