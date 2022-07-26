#include <stdio.h>

int main() {
	int a;
	int b;
	const int* pa1 = &a;
	int const* pa2 = &a;
	int* const pa3 = &a;

	pa2 = &b;

	return 0;
	 }