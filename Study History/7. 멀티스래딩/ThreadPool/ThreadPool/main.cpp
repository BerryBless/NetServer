#include <iostream>
#include <Windows.h>
#include "ThreadPool.h"

#include <vector>

void job1(LPVOID arg) {
	Sleep(30);
	printf_s("job1 %lld\n", (INT64)arg);
}
void job2(LPVOID arg) {
	Sleep(50);
	printf_s("job2 %lld\n", (INT64) arg);
}

int main() {
	std::vector<CThread *> threads;
	CThread *tempThread;
	for (int i = 0; i < 5; ++i) {
		tempThread = new CThread;
		threads.push_back(tempThread);
	}
	for (int i = 0; i < 5; ++i) {
		threads[i]->BeginThread();
	}
	threads[1]->SetThreadName(L"one");
	threads[3]->SetThreadName(L"Three");
	threads[1]->Launch([](LPVOID arg) {
		printf_s("s Lamda1 %lld\n",  (INT64) arg);
		int a = 0;
		for (int i = 0; i < 1000000000; ++i) ++a;
		printf_s("e Lamda11 %lld\n", (INT64) arg);
		}, (LPVOID) 1);
	threads[3]->Launch([](LPVOID arg) {
		printf_s("Lamda33 %lld\n", (INT64) arg);
		Sleep(500);
		}, (LPVOID) 3333333);
	threads[3]->Launch(job1, (LPVOID) 56);
	//threads[1]->Join();
	for (int i = 0; i < 5; ++i) {
		threads[i]->EndThread();
	}

	return 0;
}