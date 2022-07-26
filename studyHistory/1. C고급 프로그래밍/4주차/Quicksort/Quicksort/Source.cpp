#include<stdio.h>

void Swap(int& a, int& b) {
	printf_s("swap[%d, %d]", a, b);
	int t = a;
	a = b;
	b = t;
}

void QuickSort(int arr[], int start, int end) {
	if (end <= start) return;

	int p = start;
	int l = start+1;
	int r = end;

	while (l <= r) {
		while (arr[p] >= arr[l]) {
			l++;
			if (l == r) break;
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

int main() {
	int arr[20] = { 5,96,31,4,8,22,78,16,45,54,23,1,3,85,4,6,1,7,99,46 };
	QuickSort(arr, 0, 19);
	printf_s("\n\n=== SORT FINISH ====\n\n");
	for (int i = 0; i < 20; i++) {
		printf_s("%2d ", arr[i]);
	}
	return 0;
}