#include<stdio.h>
#include<stdlib.h>

int erand() {
	int random = 0;
	int mask = 0x7fffffff;
	for (int i = 0; i < 4; i++) {
		random = random << 8;
		random = random | rand();
		random = random & mask;
		printf_s("%x\n", random);
	}
	return random;
}

int main() {
	int random=erand();
	printf_s("%x\n", random);
}