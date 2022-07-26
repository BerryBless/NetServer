#include "pch.h"
#include "CScreenBuffer.h"


//-------------------------------------------------------------
// 이렇게 씁니다.
//
// #incude <stdio.h>
// #include <windows.h>
// #incude "Console.h"
//
// void main(void)
// {
//		CScreenBuffer *inst = CScreenBuffer::GetInstance();
//
//		inst->cs_MoveCursor(0, 0);	// 커서를 0, 0 위치로
//		printf("abcde");		// 0, 0 위치에 글씨를 찍음
//		inst->cs_MoveCursor(20, 10);	// 커서를 20, 10 위치로
//		printf("abcde");		// 20, 10 위치에 글씨를 찍음
//
// }
//-------------------------------------------------------------

#pragma region Singleton 
CScreenBuffer::CScreenBuffer() {
	// 생성시점 
	cs_Initial();
}

CScreenBuffer::~CScreenBuffer() {
	// 소멸시점이 프로그렘 종료시점이라 따로 안만듦
}
#pragma endregion


#pragma region Console Screen
//-------------------------------------------------------------
// 콘솔 제어를 위한 준비 작업.
//
//-------------------------------------------------------------
void CScreenBuffer::cs_Initial(void)
{
	CONSOLE_CURSOR_INFO stConsoleCursor;

	//-------------------------------------------------------------
	// 화면의 커서를 안보이게끔 설정한다.
	//-------------------------------------------------------------
	stConsoleCursor.bVisible = FALSE;
	stConsoleCursor.dwSize = 1;			// 커서 크기.
											// 이상하게도 0 이면 나온다. 1로하면 안나온다.

	//-------------------------------------------------------------
	// 콘솔화면 (스텐다드 아웃풋) 핸들을 구한다.
	//-------------------------------------------------------------
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorInfo(hConsole, &stConsoleCursor);
}

//-------------------------------------------------------------
// 콘솔 화면의 커서를 X, Y 좌표로 이동시킨다.
//
//-------------------------------------------------------------
void CScreenBuffer::cs_MoveCursor(int iPosX, int iPosY)
{
	COORD stCoord;
	stCoord.X = iPosX;
	stCoord.Y = iPosY;
	//-------------------------------------------------------------
	// 원하는 위치로 커서를 이동시킨다.
	//-------------------------------------------------------------
	SetConsoleCursorPosition(hConsole, stCoord);
}

//-------------------------------------------------------------
// 콘솔 화면을 조기화 한다.
//
//-------------------------------------------------------------
void CScreenBuffer::cs_ClearScreen(void)
{
	//int iCountX, iCountY;
	DWORD dw;

	FillConsoleOutputCharacter(GetStdHandle(STD_OUTPUT_HANDLE), ' ', 100 * 100, { 0, 0 }, &dw);

	/*
		//-------------------------------------------------------------
		// 화면 크기만큼 세로, 가로 이중 for 문을 사용하여
		// 각각의 좌표마다 printf(" ");  공백을 전부 출력 해준다.
		//-------------------------------------------------------------
		for ( iCountY = 0 ; iCountY < dfSCREEN_HEIGHT; iCountY++ )
		{
			for ( iCountX = 0; iCountX < dfSCREEN_WIDTH; iCountX++ )
			{
				cs_MoveCursor(iCountX, iCountY);
				printf(" ");
			}
		}
	*/
}
#pragma endregion


#pragma region PrintConsol

//--------------------------------------------------------------------
// 버퍼의 내용을 화면으로 찍어주는 함수.
//
// 적군,아군,총알 등을 szScreenBuffer 에 넣어주고, 
// 1 프레임이 끝나는 마지막에 본 함수를 호출하여 버퍼 -> 화면 으로 그린다.
//--------------------------------------------------------------------
void CScreenBuffer::Buffer_Flip(void)
{
	int iCnt;
	for (iCnt = 0; iCnt < dfSCREEN_HEIGHT; iCnt++)
	{
		cs_MoveCursor(0, iCnt);
		printf(szScreenBuffer[iCnt]);
	}
}


//--------------------------------------------------------------------
// 화면 버퍼를 지워주는 함수
//
// 매 프레임 그림을 그리기 직전에 버퍼를 지워 준다. 
// 안그러면 이전 프레임의 잔상이 남으니까
//--------------------------------------------------------------------
void CScreenBuffer::Buffer_Clear(void)
{
	int iCnt;
	memset(szScreenBuffer, ' ', dfSCREEN_WIDTH * dfSCREEN_HEIGHT);

	for (iCnt = 0; iCnt < dfSCREEN_HEIGHT; iCnt++)
	{
		szScreenBuffer[iCnt][dfSCREEN_WIDTH - 1] = '\0';
	}

}

//--------------------------------------------------------------------
// 버퍼의 특정 위치에 원하는 문자를 출력.
//
// 입력 받은 X,Y 좌표에 아스키코드 하나를 출력한다. (버퍼에 그림)
//--------------------------------------------------------------------
void CScreenBuffer::Sprite_Draw(int iX, int iY, char chSprite)
{
	if (iX < 0 || iY < 0 || iX >= dfSCREEN_WIDTH - 1 || iY >= dfSCREEN_HEIGHT)
		return;

	szScreenBuffer[iY][iX] = chSprite;
}

// 버퍼의 특정 위치에 원하는 문자열를 출력.
void CScreenBuffer::Sprite_Draw(int iX, int iY, const char* chSprite, int len) {
	IF_PTR_NULL(chSprite) return;
	memcpy(&szScreenBuffer[iY][iX], chSprite, len);
}

#pragma endregion
