#include <stdio.h>
#include <Windows.h> // timeBeginPeriod()
#include <timeapi.h> // timeBeginPeriod()
#include <time.h> // clock()
#include <sysinfoapi.h> // winmm.lib 추가해야함,GetTickCount()

int main() {
	timeBeginPeriod(1234);
	while (true)
	{
		printf_s("\nclock : %d\n", clock());
		printf_s("GetTickCount : %d\n", GetTickCount());
		printf_s("timeGetTime : %d\n", timeGetTime());

	}
	
	return 0;
}