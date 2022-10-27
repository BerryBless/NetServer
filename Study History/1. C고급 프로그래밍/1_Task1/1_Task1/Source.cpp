/*
* 비트연산자만으로 코딩
* 과제 1. unsigned char 변수의 값을 비트단위로 찍어주기

		- 지역변수에 특정 값을 하나 넣음
		- 비트 단위로 분해해서 0 이면 0 출력, 1 이면 1 출력
*/
#include <stdio.h>

int main() {
	// 1. var >> 1 반복할시 뒤집(flip)는 이쁜방법?
	// 2. 반복문 안쓰는 방법?
	unsigned char var = 40;
	int i;
	printf("%d 의 바이너리 : ", var);
	for (i = 7; i >= 0; i--) {
		printf("%d", var >> i & 1);
	}
	return 0;
}