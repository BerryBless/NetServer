#pragma once
#define CONSOLE // 콘솔 틀기


#pragma region ActionMapping

// 정지
#define dfACTION_STAND -1

// 이동
#define dfACTION_MOVE_LL	0
#define dfACTION_MOVE_LU	1
#define dfACTION_MOVE_UU	2
#define dfACTION_MOVE_RU	3
#define dfACTION_MOVE_RR	4
#define dfACTION_MOVE_RD	5
#define dfACTION_MOVE_DD	6
#define dfACTION_MOVE_LD	7

// 공격
#define dfACTION_ATTACK1	20
#define dfACTION_ATTACK2	22
#define dfACTION_ATTACK3	24
#pragma endregion

// 이동속도
#define dfMOVE_PIXEL_HORIZONTAL 3
#define dfMOVE_PIXEL_VERTICAL 2



#pragma region Sprite

// 이동 가능 영역
#define dfRANGE_MOVE_TOP	0
#define dfRANGE_MOVE_LEFT	0
#define dfRANGE_MOVE_RIGHT	6400
#define dfRANGE_MOVE_BOTTOM	6400
// 화면크기
#define dfSCREEN_WIDTH		640
#define dfSCREEN_HEIGHT		480
// 카메라 영역 이동영역
#define dfCAMERA_MIN_HEIGHT	dfRANGE_MOVE_TOP
#define dfCAMERA_MIN_WIDTH	dfRANGE_MOVE_LEFT
#define dfCAMERA_MAX_HEIGHT	dfRANGE_MOVE_BOTTOM - dfSCREEN_HEIGHT
#define dfCAMERA_MAX_WIDTH	dfRANGE_MOVE_RIGHT - dfSCREEN_WIDTH
// 에니메이션 프레임 딜레이
#define dfDELAY_STAND	5
#define dfDELAY_MOVE	4
#define dfDELAY_ATTACK1	3
#define dfDELAY_ATTACK2	4
#define dfDELAY_ATTACK3	4
#define dfDELAY_EFFECT	3

enum class e_SPRITE {
	eTileMap,
	eMap,
	/////
	// 	   STAND(IDLE)
	/////
	ePLAYER_STAND_L01,
	ePLAYER_STAND_L02,
	ePLAYER_STAND_L03,
	ePLAYER_STAND_L04,
	ePLAYER_STAND_L05,
	ePLAYER_STAND_L_MAX,

	ePLAYER_STAND_R01,
	ePLAYER_STAND_R02,
	ePLAYER_STAND_R03,
	ePLAYER_STAND_R04,
	ePLAYER_STAND_R05,
	ePLAYER_STAND_R_MAX,
	/////
	// 	   MOVE
	/////
	ePLATER_MOVE_L01,
	ePLATER_MOVE_L02,
	ePLATER_MOVE_L03,
	ePLATER_MOVE_L04,
	ePLATER_MOVE_L05,
	ePLATER_MOVE_L06,
	ePLATER_MOVE_L07,
	ePLATER_MOVE_L08,
	ePLATER_MOVE_L09,
	ePLATER_MOVE_L10,
	ePLATER_MOVE_L11,
	ePLATER_MOVE_L12,
	ePLATER_MOVE_L_MAX,

	ePLATER_MOVE_R01,
	ePLATER_MOVE_R02,
	ePLATER_MOVE_R03,
	ePLATER_MOVE_R04,
	ePLATER_MOVE_R05,
	ePLATER_MOVE_R06,
	ePLATER_MOVE_R07,
	ePLATER_MOVE_R08,
	ePLATER_MOVE_R09,
	ePLATER_MOVE_R10,
	ePLATER_MOVE_R11,
	ePLATER_MOVE_R12,
	ePLATER_MOVE_R_MAX,
	/////
	// 	   ATTACK
	/////
	ePLAYER_ATTACK1_L01,
	ePLAYER_ATTACK1_L02,
	ePLAYER_ATTACK1_L03,
	ePLAYER_ATTACK1_L04,
	ePLAYER_ATTACK1_L_MAX,

	ePLAYER_ATTACK1_R01,
	ePLAYER_ATTACK1_R02,
	ePLAYER_ATTACK1_R03,
	ePLAYER_ATTACK1_R04,
	ePLAYER_ATTACK1_R_MAX,
	//
	ePLAYER_ATTACK2_L01,
	ePLAYER_ATTACK2_L02,
	ePLAYER_ATTACK2_L03,
	ePLAYER_ATTACK2_L04,
	ePLAYER_ATTACK2_L_MAX,

	ePLAYER_ATTACK2_R01,
	ePLAYER_ATTACK2_R02,
	ePLAYER_ATTACK2_R03,
	ePLAYER_ATTACK2_R04,
	ePLAYER_ATTACK2_R_MAX,
	//
	ePLAYER_ATTACK3_L01,
	ePLAYER_ATTACK3_L02,
	ePLAYER_ATTACK3_L03,
	ePLAYER_ATTACK3_L04,
	ePLAYER_ATTACK3_L05,
	ePLAYER_ATTACK3_L06,
	ePLAYER_ATTACK3_L_MAX,

	ePLAYER_ATTACK3_R01,
	ePLAYER_ATTACK3_R02,
	ePLAYER_ATTACK3_R03,
	ePLAYER_ATTACK3_R04,
	ePLAYER_ATTACK3_R05,
	ePLAYER_ATTACK3_R06,
	ePLAYER_ATTACK3_R_MAX,
	/////
	// 	   SPARK(EFFECT)
	/////
	eEFFECT_SPARK_01,
	eEFFECT_SPARK_02,
	eEFFECT_SPARK_03,
	eEFFECT_SPARK_04,
	eEFFECT_SPARK_MAX,
	/////
	// 	   OTHER
	/////
	eGUAGE_HP,
	eSHADOW,
	eSPRITE_MAX
};
#pragma endregion

// 타일맵
#define dfMAP_WIDTH	100
#define dfMAP_HEIGHT	100