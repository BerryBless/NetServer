#include <stdio.h>
#include <conio.h>
#include <Windows.h>

int main() {
	int count = 120;
	int a = 120;
	int b = 120;
	int c = 120;
	int d = 120;
	int e = 120;
	int f = 120;

	while (true)	{
		if (_kbhit()) {
			count++;
			a=121;
			c=121;
			char Key = _getch();
		}
		printf("%p :: %d\n", &count,count);
		Sleep(1000);
	}
}