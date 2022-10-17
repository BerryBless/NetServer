#include <stdio.h>
#include <Windows.h>
#include <stdlib.h>
#define N 10

void print(); // 프린트
void fill(int y, int x); // [i][j] == [y][x]
// 북 동 남 서 (시계방향)
int dy[4] = { -1,0,1,0 };
int dx[4] = { 0,1,0,-1 };
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
}; // 0이면 막힌곳, 1이면 이어진곳
int _check[N][N] = { 0 };// 간길 체크

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

	Sleep(500);
}

void fill(int y, int x) {
	_check[y][x] = 1;
	print();

	for (int i = 0; i < 4; i++) {

		int ty = y + dy[i];
		int tx = x + dx[i];
		if (ty < 0 || N <= ty ||
			tx < 0 || N <= tx) {
			continue;
		}
		if (_map[ty][tx] == 1 &&
			_check[ty][tx] == 0) {
			fill(ty, tx);
		}
	}
}