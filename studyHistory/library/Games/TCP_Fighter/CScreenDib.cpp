#include "CScreenDib.h"
#include "CFramework.h"
#include <string.h>
CScreenDib::CScreenDib(int iWidth, int iHeight, int iColorBit) {
	// 맴버변수 초기화
	_bypBuffer = NULL;
	memset(&_stDibInfo, 0, sizeof(BITMAPINFO));
	_iBufferSize = 0;
	_iColorBit = 0;
	_iHeight = 0;
	_iWidth = 0;
	_iPitch = 0;

	// 스크린버퍼 생성 호출
	CreateDibBuffer(iWidth, iHeight, iColorBit);
}

CScreenDib::~CScreenDib() {
	// 스크린버퍼 파괴 호출
	ReleaseDibBuffer();
}

void CScreenDib::CreateDibBuffer(int iWidth, int iHeight, int iColorBit) {
	_iWidth = iWidth;
	_iHeight = iHeight;
	_iColorBit = iColorBit;
	_iPitch = ((_iWidth * (_iColorBit / 8)) + 3) & ~3;
	_iBufferSize = _iHeight * _iPitch;

	// DIB헤더 넣기
	_stDibInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	_stDibInfo.bmiHeader.biWidth = iWidth;
	_stDibInfo.bmiHeader.biHeight = -iHeight;
	_stDibInfo.bmiHeader.biPlanes = 1;
	_stDibInfo.bmiHeader.biBitCount = _iColorBit;
	_stDibInfo.bmiHeader.biCompression = 0;
	_stDibInfo.bmiHeader.biSizeImage = _iBufferSize;
	_stDibInfo.bmiHeader.biXPelsPerMeter = 0;
	_stDibInfo.bmiHeader.biYPelsPerMeter = 0;
	_stDibInfo.bmiHeader.biClrUsed = 0;
	_stDibInfo.bmiHeader.biClrImportant = 0;

	// 버퍼 생성
	_bypBuffer = new BYTE[_iBufferSize];
	memset(_bypBuffer, 0xFF, _iBufferSize);	// 흰색 배경
}

void CScreenDib::ReleaseDibBuffer() {
	// 맴버 초기화
	memset(&_stDibInfo, 0, sizeof(BITMAPINFO));
	_iBufferSize = 0;
	_iColorBit = 0;
	_iHeight = 0;
	_iWidth = 0;
	_iPitch = 0;

	// 할당 해지
	delete[] _bypBuffer;
	_bypBuffer = NULL;
}

void CScreenDib::DrawBuffer(HWND hWnd, int iX, int iY) {
	if (_bypBuffer == NULL) return;

	RECT rect;
	HDC hDC;
	// DC얻기
	GetWindowRect(hWnd, &rect);
	hDC = GetDC(hWnd);

	// DC출력
	int i = SetDIBitsToDevice(hDC, 0, 0, _iWidth, _iHeight,
									0, 0, 0, _iHeight,
									_bypBuffer, &_stDibInfo,
									DIB_RGB_COLORS);
	// 프레임출력
	WCHAR wcpRFrame[5];
	wsprintf(wcpRFrame, L"%d", I_FRAMEWORK->GetRenderFPS());
	TextOut(hDC, 0, 0, wcpRFrame, (int)wcslen(wcpRFrame));

	// DC풀기
	ReleaseDC(hWnd, hDC);
}

BYTE *CScreenDib::GetDibBuffer() {
	return _bypBuffer;
}

int CScreenDib::GetHeight() {
	return _iHeight;
}

int CScreenDib::GetPitch() {
	return _iPitch;
}

int CScreenDib::GetWidth() {
	return _iWidth;
}
