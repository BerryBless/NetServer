#pragma once

#pragma region ActionMapping

//-----------------------------------------------------------------
// 정지
//-----------------------------------------------------------------
#define dfACTION_STAND -1


//-----------------------------------------------------------------
// 이동
//-----------------------------------------------------------------
#define dfACTION_MOVE_LL	0
#define dfACTION_MOVE_LU	1
#define dfACTION_MOVE_UU	2
#define dfACTION_MOVE_RU	3
#define dfACTION_MOVE_RR	4
#define dfACTION_MOVE_RD	5
#define dfACTION_MOVE_DD	6
#define dfACTION_MOVE_LD	7

//-----------------------------------------------------------------
// 공격
//-----------------------------------------------------------------
#define dfACTION_ATTACK1	20
#define dfACTION_ATTACK2	22
#define dfACTION_ATTACK3	24

//---------------------------------------------------------------
// 공격 데미지.
//---------------------------------------------------------------
#define dfATTACK1_DAMAGE		1
#define dfATTACK2_DAMAGE		2
#define dfATTACK3_DAMAGE		3

//---------------------------------------------------------------
// 공격범위.
//---------------------------------------------------------------
#define dfATTACK1_RANGE_X		80
#define dfATTACK2_RANGE_X		90
#define dfATTACK3_RANGE_X		100
#define dfATTACK1_RANGE_Y		10
#define dfATTACK2_RANGE_Y		10
#define dfATTACK3_RANGE_Y		20
#pragma endregion

//-----------------------------------------------------------------
// 이동속도
// 25FPS 기준
//-----------------------------------------------------------------
#define dfMOVE_PIXEL_HORIZONTAL 6 // 50FPS : 3
#define dfMOVE_PIXEL_VERTICAL 4	// 50FPS : 2

//-----------------------------------------------------------------
// 이동 가능 영역
//-----------------------------------------------------------------
#define dfRANGE_MOVE_TOP	0
#define dfRANGE_MOVE_LEFT	0
#define dfRANGE_MOVE_BOTTOM	6400
#define dfRANGE_MOVE_RIGHT	6400

//-----------------------------------------------------------------
// 응답없이 연결 끊는 시간
//-----------------------------------------------------------------
#define dfMAX_IDLE_TIME 	60000


//-----------------------------------------------------------------
// 타일맵(섹터)
//-----------------------------------------------------------------
#define dfSECTOR_WIDTH		400
#define dfSECTOR_HEIGHT		400
#define dfSECTOR_MAX_Y	65
#define dfSECTOR_MAX_X	65

//-----------------------------------------------------------------
// 이동 오류체크 범위
//-----------------------------------------------------------------
#define dfERROR_RANGE		50

//-----------------------------------------------------------------
// 델타타임 기본값
//-----------------------------------------------------------------
#define dfDTD 40 // 25FPS
//#define dfDTD 20 // 50FPS

#define dfLOGFILENAME "_v0.9_25FPS_400400.log"