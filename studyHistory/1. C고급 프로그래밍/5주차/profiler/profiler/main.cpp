#include "Profiler.h"
#include <conio.h>

//*
void f1() {
	PRO_START(L"F1");
	Sleep(1);
}
void f2() {
	PRO_START(L"F2");
	Sleep(80);
}
void f3() {
	PRO_BEGIN(L"F3");
	Sleep(1000);
	PRO_END(L"F3");
}
//*/
int main() {
	while (true) {
		f1();
		f2();
		f3();
		if (_kbhit()) {
			char key = _getch();
			PRO_PRINT((WCHAR*)L"log.log");
		}
	}


	return 0;
}