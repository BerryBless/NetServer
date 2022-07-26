#pragma once
// ================================
// 스프라이트 관리 및 백버퍼에 출력
// ---
// 맴버변수
// _iMaxSprite : 스프라이트의 전체 갯수
// _stpSprite : 스프라이트 개수만큼 구조체를 동적할당한 배열
// _dwColorKey : 이 스프라이트의 컬러키
// ---
// 맴버함수
// LoadDibSprite : 스프라이트를 로딩함
// ReleaseSprite : 스프라이트를 삭제함
// DrawSprite : 스프라이트를 백버퍼에 프린트
// DrawSpriteRed : 스프라이트를 백버퍼에 프린트 (빨강색 255로)
// GetSpriteHitbox : 스프라이트 크기의 RECT구하기 (히트박스 == 스프라이트 전체, 따로 처리안함)
// ================================

#include "framework.h"
#include "GDefine.h"

#define I_SPRITEDIB CSpriteDib::GetInstance() 

class CSpriteDib {

#pragma region Singleton 
	// 싱글톤
private:
	CSpriteDib(int iMaxSprite, DWORD dwColorKey);	// 전체 스프라이트 개수와 투명색(컬러키) 입력
	~CSpriteDib();
public:
	// 전역 인스턴스를 얻어올 전역 함수
	static CSpriteDib *GetInstance() {
		static CSpriteDib _Instance((int) e_SPRITE::eSPRITE_MAX, 0x00FFFFFF);
		return &_Instance;
	};
#pragma endregion

public:	// 스프라이트 구조체
	struct stSPRITE {
		BYTE *bypImage;	// 이미지 포인터
		int iWidth;		// 너비
		int iHeight;	// 높이
		int iPitch;		// 피치

		// 애니메이션, 판정 중심점
		int iCenterPointX; // 중점 X
		int iCenterPointY; // 중점 Y
	};

protected: // 맴버변수
	int _iMaxSprite;		// 스프라이트 전체갯수
	stSPRITE *_stpSprite;	// stSPRITE 구조체 동적할당 뱌열용 포인터
	DWORD _dwColorKey;		// 스프라이트 처리시 사용되는 컬러키(투명색)

public:


	BOOL LoadDibSprite(int iSpriteIndex, const WCHAR *szFileName, int iCenterPointX, int iCenterPointY); // 특정 BMP파일을 애니메이션 index에 지정후 로드 (하나의 프레임으로
	BOOL LoadDibSprite(e_SPRITE eSpriteIndex, const WCHAR *szFileName, int iCenterPointX, int iCenterPointY); // 특정 BMP파일을 애니메이션 index에 지정후 로드 (하나의 프레임으로
	void ReleaseSprite(int iSpriteIndex);// 지정 index번호의 스프라이트 삭제
	 
	void DrawSprite(int iSpriteIndex, int iDrawX, int iDrawY,
		BYTE *bypDest, int iDestWidth, int iDestHeight, int iDestPitch, int iDrawLen = 100, bool bColorKey = false);	//index의 스프라이트를 특정 메모리버퍼 (x,y) 좌표에 출력 (칼라키 처리 bColorKey = false )	
	void DrawSprite(e_SPRITE eSpriteIndex, int iDrawX, int iDrawY,
		BYTE *bypDest, int iDestWidth, int iDestHeight, int iDestPitch, int iDrawLen = 100, bool bColorKey = false);	//index의 스프라이트를 특정 메모리버퍼 (x,y) 좌표에 출력 (칼라키 처리 bColorKey = false )
	
	//void DrawSprite50(int iSpriteIndex, int iDrawX, int iDrawY,
	//	BYTE *bypDest, int iDestWidth, int iDestHeight, int iDestPitch); // 반투명

	void DrawSpriteRed(int iSpriteIndex, int iDrawX, int iDrawY,
		BYTE *bypDest, int iDestWidth, int iDestHeight, int iDestPitch); // 붉은빛
	

	//void DrawImage(int iSpriteIndex, int iDrawX, int iDrawY,
	//	BYTE *bypDest, int iDestWidth, int iDestHeight, int iDestPitch, int iDrawLen = 100); // 칼라키 처리없이 통째로 출력
public:
	// 좌표의 히트박스(iSpriteIndex의 스프라이트 크기만큼) 얻어오기
	BOOL GetSpriteHitbox(int iSpriteIndex, int iX, int iY, RECT *rpHitbox);
};

