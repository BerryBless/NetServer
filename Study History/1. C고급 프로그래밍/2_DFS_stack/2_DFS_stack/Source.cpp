#include <stdio.h>
#include <Windows.h>
#include <stdlib.h>
#define N 10

struct Pos { // [i][j] == [y][x]
	int y;
	int x;
};

// 북 동 남 서 (시계방향)
int _dy[4] = { -1,0,1,0 };
int _dx[4] = { 0,1,0,-1 };
int _check[N][N] = { 0 };// 간길 체크

Pos _stack[1000];
int _top = -1;
void Push(Pos p);
bool Pop(Pos& p);

void print(); // 프린트
void fill(int y, int x); // [i][j] == [y][x]
int _map[N][N] = {
	{0,0,0,0,0,1,1,1,1,0},
	{0,0,1,1,1,1,0,0,1,0},
	{0,1,1,1,0,0,1,0,1,0},
	{0,1,1,1,0,1,0,1,1,0},
	{0,1,1,0,0,1,0,1,1,1},
	{1,1,1,0,1,1,0,1,0,1},
	{1,0,1,0,0,0,0,0,1,1},
	{1,0,0,1,1,0,0,1,1,0},
	{0,0,1,1,1,1,1,0,1,0},
	{0,0,0,0,0,0,1,1,1,0}
}; // 0이면 막힌곳, 1이면 길

int main() {
	while (true) {
		memset(_check, 0, sizeof(_check));
		fill(1, 2);
	}
	return 0;
}

void print() {
	system("cls");

	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			if (_check[i][j] == 1) {
				printf("*");
			}
			else if (_map[i][j] == 1) {
				printf("O");
			}
			else {
				printf(".");
			}
		}
		printf("\n");
	}

	Sleep(20);
}

void fill(int y, int x) {
	Pos t;
	t.y = y;
	t.x = x;
	Push(t);
	while (true)
	{
		Pos p;
		bool check = Pop(p);
		if (check == false) {
			break;
		}
		if (_check[p.y][p.x] == 1)		{
			continue;
		}
		_check[p.y][p.x] = 1;
		print();



		for (int i = 0; i < 4; i++) {

			int ty = p.y + _dy[i];
			int tx = p.x + _dx[i];
			if (ty < 0 || N <= ty ||
				tx < 0 || N <= tx) {
				continue;
			}
			if (_map[ty][tx] == 1 &&
				_check[ty][tx] == 0) {
				t.y = ty;
				t.x = tx;
				Push(t);
			}
		}
	}
}
void Push(Pos p) {
	_top++;
	_stack[_top] = p;
}
bool Pop(Pos& p) {
	if (_top < 0) return false;
	p = _stack[_top];
	_top--;
	return true;
}