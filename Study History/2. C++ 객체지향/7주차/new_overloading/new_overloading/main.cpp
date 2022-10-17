#include <iostream>
#define PROFILER
#ifdef PROFILER
#include "MemProfiler.h"
#endif // DEBUG

class CTest {
private: 
	int arr[50];
public:
	CTest() {
		printf_s("持 失 切\n");
		for (int i = 0; i < 50; i++) {
			arr[i] = i;
		}
	}
	~CTest() {
		printf_s("社 瑚 切\n");
		for (int i = 0; i < 50; i++) {
			arr[i] = 0;
		}
	}
};

int main() {
	CTest* test = new CTest[5];
	CTest* test2 = new CTest[5];
	CTest* test3 = new CTest[5];
	CTest* test4 = new CTest[5];
	CTest* test5 = new CTest[5];
	CTest* test1 = new CTest;
	delete[] test;
	delete test1;
	return 0;
}