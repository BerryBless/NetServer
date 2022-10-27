#include "pch.h"
#include "GameManager.h"

//#define TESTCODE
#ifdef TESTCODE
#include "Projectile.h"
#endif
int main(void)
{
#ifdef TESTCODE
#else
	bool isRunGame = true; // 게임이 진행중인지 체크할 변수
	// 게임의 초기화
	Game_Init();

	//--------------------------------------------------------------------
	// 게임의 메인 루프
	// 이 루프가  1번 돌면 1프레임 이다.
	//--------------------------------------------------------------------
	while (isRunGame)
	{
		// 스크린 버퍼를 지움
		Buffer_Clear();

		// 게임 진행
		// 게임 종료도 체크
		isRunGame = Game_Logic();

		// 스크린 버퍼를 화면으로 출력
		Buffer_Flip();

		// 몇FPS으로 할껀지
		Sleep(FPS_DELAY);

	}

#endif // TESTCODE
	return 0;
}





