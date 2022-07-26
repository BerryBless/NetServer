#pragma once

// ==================================
// 스크린 백버퍼
// 실제로 그림을 그릴(스프라이트를 출력할) 백버퍼
// -- 
// 맴버변수
// BYTE *_bypBuffer;	// 메모리 버퍼의 포인터
// int _iBufferSize;	// 메모리 버퍼의 전체 사이즈
// int _iColorBit;		// 이 버퍼의 컬러비트
// int _iHeight;		// 이 버퍼의 높이
// int _iWidth;		// 이 버퍼의 너비
// int _iPitch;		// 이 버퍼의 피치
// BITMAPINFO _stDibInfo;		// Dib활용을 위한 헤더
// --
// void CreateDibBuffer(int iWidth, int iHeight, int iColorBit); //메모리 버퍼와 DIB를 생성합니다. 생성자에서 호출됩니다.
// void ReleaseDibBuffer();	// 생성된 메모리 버퍼를 삭제합니다. 파괴자에서 호출됩니다.
// 
// void DrawBuffer(HWND hWnd, int iX = 0, int iY = 0);	// FLIP 게임쪽에서 버퍼에 그림을 모두 그린 후 화면으로 출력합니다
// BYTE *GetDibBuffer();	// 메모리 버퍼포인터를 리턴합니다.
// int GetHeight();		// 메모리 버퍼 Pixel 높이를 리턴합니다.
// int GetWidth();			// 메모리 버퍼 Pixel 너비를 리턴합니다.
// int GetPitch();			// 메모리 버퍼 한줄의 피치 길이를 리턴합니다.
// ==================================


#include "framework.h"

#define I_SCREENDIB CScreenDib::GetInstance() 

class CScreenDib {
#pragma region Singleton 
	// 싱글톤
private:
	CScreenDib(int iWidth, int iHeight, int iColorBit);	// 전체 사이즈와 컬러 비트를 받습니다.
	virtual ~CScreenDib();
public:
	// 전역 인스턴스를 얻어올 전역 함수
	static CScreenDib *GetInstance() {
		static CScreenDib _Instance(640, 480, 32);
		return &_Instance;
	};
#pragma endregion

private:
	BYTE *_bypBuffer;	// 메모리 버퍼의 포인터
	int _iBufferSize;	// 메모리 버퍼의 전체 사이즈
	int _iColorBit;		// 이 버퍼의 컬러비트
	int _iHeight;		// 이 버퍼의 높이
	int _iWidth;		// 이 버퍼의 너비
	int _iPitch;		// 이 버퍼의 피치
	BITMAPINFO _stDibInfo;		// Dib활용을 위한 헤더


protected:
	void CreateDibBuffer(int iWidth, int iHeight, int iColorBit); //메모리 버퍼와 DIB를 생성합니다. 생성자에서 호출됩니다.
	void ReleaseDibBuffer();	// 생성된 메모리 버퍼를 삭제합니다. 파괴자에서 호출됩니다.

public:
	void DrawBuffer(HWND hWnd, int iX = 0, int iY = 0);	// FLIP 게임쪽에서 버퍼에 그림을 모두 그린 후 화면으로 출력합니다
	BYTE *GetDibBuffer();	// 메모리 버퍼포인터를 리턴합니다.
	int GetHeight();		// 메모리 버퍼 Pixel 높이를 리턴합니다.
	int GetWidth();			// 메모리 버퍼 Pixel 너비를 리턴합니다.
	int GetPitch();			// 메모리 버퍼 한줄의 피치 길이를 리턴합니다.
	
};

