/* 비트 연산자만으로 코딩하기
* 과제 3. unsigned int (32bit) 변수를 바이트 단위로 사용하기

		- unsigned int 변수 = 0 초기값 가짐
		- 키보드로 1 ~ 4 의 바이트 위치를 입력 받고
		- 해당 위치에 넣을 데이터 0 ~ 255 를 입력 받음
		- 사용자가 입력한 바이트 위치에 해당 값을 넣음

		- 데이터가 입력 되면 바이트 단위로 쪼개서 출력 & 4바이트 16진수 출력
		- 기존 데이터는 보존이 되어야 하며
		- 입력된 바이트 위치의 데이터는 기존 값을 지우고 넣음.


*/

#include <stdio.h>

int main() {
	unsigned int def[5] = { 0x000000FF,0xFFFFFF00,0xFFFF00FF ,0xFF00FFFF ,0x00FFFFFF };
	unsigned int step[5] = { 0,0,8,16,24 };
	unsigned int var = 0;
	int n;
	unsigned int m;
	while (true)	{
		printf("위치 (1~4) : ");
		scanf("%d", &n);
		printf("값 [0~255] : ");
		scanf("%d", &m);

		m &= def[0];
		var &= def[n];

		var = var | (m << step[n]);
		
		// 출력
		{
			for (int i = 1; i <= 4; i++) {
				unsigned int t = var >> step[i];
				printf("%d 번째 바이트 값 : %d\n", i, t & def[0]);
			}
			printf("\n전체 4바이트 값 :0x%p\n", var);
		}

	}
}