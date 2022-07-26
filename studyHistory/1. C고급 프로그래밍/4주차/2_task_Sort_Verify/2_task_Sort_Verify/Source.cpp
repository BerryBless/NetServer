/*
* 포폴 문서
* https://docs.google.com/document/d/186-flEYk2_LOosofg0rd2cGtVriTLucydsuzuauXxrQ/edit?usp=sharing
*/

#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <thread>
#include <time.h>

#define MAX_ARRAY 10'000'000

// rand() 확장
// 1~4Byte 랜덤
unsigned int Erand(int byteSize) {
	unsigned int r = 0;		// 0000 0000
	int mask = 0xFF;		// 1111 1111
	for (int i = 0; i < byteSize; i++) {
		int t = rand();
		t &= mask;
		r = (r << 8); // 1byte 만큼 이동
		r |= t;
		//printf_s("%02d : 0x%08x\n", i, r);
	}
	return r;
}

#pragma region QuickSort


void Swap(int& a, int& b) {
	//printf_s("swap[%d, %d]", a, b);
	int t = a;
	a = b;
	b = t;
}
void QuickSort(int arr[], int start, int end) {
	if (end <= start) return;

	int p = start;
	int l = start + 1;
	int r = end;

	while (l <= r) {
		while (arr[p] >= arr[l]) {
			l++;
			if (l >= r) break;
		}

		while (arr[p] <= arr[r]) {
			r--;
			if (r < l) break;
		}

		if (l < r) {
			Swap(arr[l], arr[r]);
		}
	}
	Swap(arr[p], arr[r]);
	QuickSort(arr, start, r);
	QuickSort(arr, l, end);
}
#pragma endregion


#pragma region 정렬 검증
// 검증 : 신뢰 가능한 정렬
void SortVerify(int arr[], int size) {
	printf_s("START :: SortVerify()\n");
	time_t start = clock();
	std::sort(arr, arr + size);

	printf_s("END   :: SortVerify() - %04lldms\n",clock() - start);
}
// 테스트용 배열 만들기
void Array_Make(int origin[], int size) {
	printf_s("START :: Array_Make()\n");
	time_t start = clock();

	for (int i = 0; i < size; i++) {
		origin[i] = (int)Erand(sizeof(int));
	}
	printf_s("END   :: Array_Make() - %04lldms\n", clock() - start);
}
// src 를 size만큼 dest에 복사
void Array_Copy(int dest[], const int src[], int size) {
	printf_s("START :: Array_Copy()\n");
	time_t start = clock();

	for (int i = 0; i < size; i++) {
		dest[i] = src[i];
	}
	printf_s("END   :: Array_Copy() - %04lldms\n", clock() - start);
}

// 테스트할 정렬
void Array_Sort(int arr[], int size) {
	printf_s("START :: Array_Sort()\n");
	time_t start = clock();

	QuickSort(arr, 0, size - 1);

	printf_s("END   :: Array_Sort() - %04lldms\n", clock() - start);
}

// 정렬 채점
bool Array_Veri(int sorted[], int origin[], int size) {
	printf_s("START :: Array_Veri()\n");
	time_t start = clock();

	// 정답과 풀이 비교
	bool o = true;
	for (int i = 0; i < size; i++) {
		if (origin[i] != sorted[i]) {
			o = false;
		}
	}
	printf_s("END   :: Array_Veri() - %04lldms\n", clock() - start);
	return o;
}
#pragma endregion


int main() {
	int* originArray = (int*)malloc(sizeof(int) * MAX_ARRAY);
	int* sortedArray = (int*)malloc(sizeof(int) * MAX_ARRAY);
	for (int TESTCASE = 9999999; ; TESTCASE++) {
		// 재현 가능한 렌덤
		srand(TESTCASE);

		int arrSize = TESTCASE % MAX_ARRAY + 1; // 1 ~ MAX_ARRAY
		printf_s("\n==================================\n");
		printf_s("TEST %d : size(%d) start\n\n", TESTCASE, arrSize);
		// 데이터 생성
		Array_Make(originArray, arrSize);
		Array_Copy(sortedArray, originArray, arrSize);
		// 답안지 작성
		std::thread originSortThread(SortVerify, originArray, arrSize);
		// 문제집 풀기
		Array_Sort(sortedArray, arrSize);

		// 검증
		originSortThread.join(); // 답안지 작성 끝나기 기다리기
		if (Array_Veri(sortedArray, originArray, arrSize)) {
			printf_s("\nTEST %d : size(%d) clear\n", TESTCASE, arrSize);
			printf_s("\n==================================\n");
		}
		else {
			printf_s("\n==================================\n");
			printf_s("\nERROR | TESTCASE :: %d / arrSize :: %d\n", TESTCASE, arrSize);
			printf_s("\n==================================\n");
			break;
		}
	}
	free(originArray);
	free(sortedArray);
	return 0;
}