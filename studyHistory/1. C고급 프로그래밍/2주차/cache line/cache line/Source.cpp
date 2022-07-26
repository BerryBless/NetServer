#include <stdio.h>

struct T
{
	short a;
	int b;
	__int64 c;
	short d;
	char e;
	short f;
	__int64 g;
	char h;
	int i;
	int j;
	char k;
	short l;
	char m;
	__int64 n;
};

int main() {
	volatile T t;
	t.a = 0;
	t.b = 0;
	t.c = 0;
	t.d = 0;
	t.e = 0;
	t.f = 0;
	t.g = 0;
	t.h = 0;
	t.i = 0;
	t.j = 0;
	t.k = 0;
	t.l = 0;
	t.m = 0;
	t.n = 0;
	return 0;
}