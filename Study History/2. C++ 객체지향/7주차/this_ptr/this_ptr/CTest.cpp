#include "CTest.h"
#include <iostream>

#define NAMX  255

CTest::CTest(int a) {
	_a = a;
	_b = 0xAAAAAAAAAAAAAAAA;
}

void CTest::Show() {
	printf_s("%d", this->_a);
}

void CTest::Set(int a) {
	//int asd = a*45;
	//int ase = asd*45;
	//int asa = ase*45;

	_a = a;
}

int CTest::Get() {
	char a = 0xEE;
	__int64 b =  a;
	float c = b;
	int d = c;
	char* p = &a;
	*(p) = 0xAAAAAAAAAAAAAAAA;
	return _a;
}