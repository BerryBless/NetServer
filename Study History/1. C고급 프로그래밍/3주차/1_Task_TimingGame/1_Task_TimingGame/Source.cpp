// 초에 해당하는 값을 배열에 넣어두고, 이 시간을 타이밍 데이터로 씀.
// 뒤로 갈 수록 큰 값을 넣는걸 원칙으로 함.

// # 화면 지우기 system("cls")
// system 함수는 외부 커멘드 명령어를 실행시키는 함수.
// cls 는 콘솔 화면을 지우는 명령어임.
// system("cls");  // 이라고 코드를 적으면 콘솔 화면이 지워짐.
//

//
// # 시간구하기 - clock() 함수.
// 프로세스가 시작된 후의 시간, 1초는 CLOCKS_PER_SEC 값. (1000ms)
// 리턴값은 clock_t (long) 타입.

// # 키보드 눌릭 확인 - _kbhit() 함수.
// 키보드가 눌렸는지 확인후 눌렸으면 true 리턴.
// 단, 키를 누른 후 콘솔키에서 값을 빼주지 않으면 다음 루프에서도 계속 true 리턴.
//
// _getch() 함수를 사용하여 콘솔키 입력을 뽑을 수 있음.
// 본래 사용법 char KeyChar = _getch();
// 우리는 _getch() 의 리턴값 (눌린키) 는 사용하지 않지만 콘솔키를 빼기 위해 씀
//
// if ( _kbhit() )
// {
// _getch();
// 컨텐츠 로직...
// }


// # abs(X) 절대값 함수.
// 음수의 값을 양수로 바꿈, 양수는 그냥 양수


// - 컨텐츠 부
//
// 1. 화면 상단에 시간이 표시됨.  초:밀리세컨드   (00:000  으로 자리수 맞춤)
// 2. 아래에는 각 키 타이밍의 정보와 해당 타이밍에 성공여부 결과를 표시
// 3. 아무런 키를 누르지 않고 지정 시간을 1초 이상 넘으면 자동으로 fail 처리.
// 4. 사용자가 키를 누르면 해당 시간을 체크하여 오차 범위에 따라서 지정 타이밍의 결과가 화면에 표시됨.
//
//  / 4 한 수치를 1단계로 최대 4단계 까지의 오차로 Great, Good, Nogood, Bad 단계 측정.

// 오차라는 것은, + - 모두 해당됨.

#include <stdio.h>
#include <conio.h>
#include <Windows.h>
#include <time.h>

#define MAX_N 9   
#define CORRECTION 1000	// 보정값
//#define CLOCKS_PER_SEC 1000 // 1 sec = 1000 ms 

#pragma region  판정
enum  Accuracy { // 판정
	Wait = 0,
	Fail,
	Great,
	Good,
	Nogood,
	Bad
};
const char* _AccurayStr[] = { // 판정 출력 문자열
	"",
	"Fail",
	"Great",
	"Good",
	"Nogood",
	"Bad"
};
const int _AccurayTable[4] = { // 판정 테이블
	250'000,	// Great
	500'000,	// Good
	750'000,	// Nogoot
	1'000'000	// Bad
};
#pragma endregion

int _Timing[MAX_N] = { 3, 5, 10, 17, 20, 25, 29, 31, 33 };
int _Player[MAX_N] = { 0 }; // 플레이어 점수

int main() {
	int now = 0;		// 현재 맞춰야할 타이밍
	int dt;				// 프로세스 시작으로 부터 시간 (ms)
	int timing;			// 현재 타이밍값(ms)
	while (now < MAX_N) {
		// 시간 체크
		dt = (int)clock();
		timing = _Timing[now] * CLOCKS_PER_SEC;

		// 키입력 체크
		if (_kbhit()) {
			_Player[now] = dt;
			now++;
			char KeyChar = _getch();
		}
		// 정확한 타이밍으로 부터 키 입력없이 1000ms 지남
		else if (timing + CORRECTION <= dt) {
			_Player[now] = dt;
			now++;
		}

		// print
		system("cls");
		printf_s("%02d:%03d\n\n", dt / CLOCKS_PER_SEC, dt % CLOCKS_PER_SEC);
		for (int i = 0; i < MAX_N; i++) {
			Accuracy res = Accuracy::Wait;
			int ac = _Player[i] - (_Timing[i] * CLOCKS_PER_SEC);
			ac = ac * ac;
			if (_Player[i] == 0) {
				res = Accuracy::Wait;
			}
			else if (ac <= _AccurayTable[0]) {
				res = Accuracy::Great;
			}
			else if (ac <= _AccurayTable[1]) {
				res = Accuracy::Good;
			}
			else if (ac <= _AccurayTable[2]) {
				res = Accuracy::Nogood;
			}
			else if (ac <= _AccurayTable[3]) {
				res = Accuracy::Bad;
			}
			else {
				res = Accuracy::Fail;
			}

			printf_s("%d sec : %s\n", _Timing[i], _AccurayStr[res]);
		}

		// 갱신이 너무 빨라서 화면이 안보임
		//Sleep(50);
	}
	return 0;
}
