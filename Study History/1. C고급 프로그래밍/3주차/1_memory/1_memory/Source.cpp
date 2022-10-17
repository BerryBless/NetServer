#include<stdio.h>
#include <malloc.h>

struct AA
{
	int a;
	alignas(32) int b;
};

int main() {
	AA* p = (AA*)malloc(63);
	printf_s("\t\tp\t\t:: 0x%p : %d\n\n", p, ((int)p) % 32);


	int mask = ~0x1f;
	AA* pa = (AA*)(((int)p +31) & mask);
	printf_s("(((int)p +31) & mask)\t\t:: 0x%p : %d\n\n", pa, ((int)pa) % 32);
	AA* pb = (AA*)(((int)p + 0x1f) & (~0x1f));
	printf_s("((int)p + 0x1f) & (~0x1f)\t:: 0x%p : %d\n", pb, ((int)pb) % 32);

	free(p);
}