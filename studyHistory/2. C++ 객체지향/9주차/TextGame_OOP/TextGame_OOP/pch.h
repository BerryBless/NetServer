#pragma once
#include <stdio.h>
#include <memory.h>
#include <Windows.h>
#include <stdlib.h>
#include <winuser.h>
#include <time.h>

// 프로파일러
#include "MemProfiler.h"

#define CRASH() int* nPtr = nullptr; *nPtr = 0 // 크래쉬!

#define INIT_FILE_PATH "Data/Init.dat" // 파일경로
#define STRING_LEN_MAX		64		// 파일 경로 문자열 최대값
#define dfSCREEN_WIDTH		81		// 콘솔 가로 80칸 + NULL
#define dfSCREEN_HEIGHT		24		// 콘솔 세로 24칸

#define FPS_DELAY			40		// 40ms마다

// 오브젝트가 몬스터인가? 체크 매개변수 : ObjectType 타입
// ObjectType 가 몬스터 범위 내라면 true
#define IS_MONSTER(obj) (int)ObjectType::Monster <= (int)obj && \
						(int)obj < (int)ObjectType::MonsterEnd
#define IF_PTR_NULL(ptr) if(ptr == nullptr) 

typedef unsigned __int64 frame_t;

// ============================================================================
//									좌표계
// X, Y =  map[Y][X]
// ============================================================================
struct Position {
	int X;
	int Y;
};


// ============================================================================
//								Input Message
// ---------------------------------------------------------------------------
// 플레이어의 인풋이 발생할때 메시지큐에 쌓아둘 메시지
// 방향키 : Up, Right, Down, Left
// wasd	 : AttackUp, AttackRight, AttackDown, AttackLeft
// ============================================================================
enum class InputMessage {
	// 이동
	Up = 0,
	Right,
	Down,
	Left,
	// 공격
	AttackUp,
	AttackRight,
	AttackDown,
	AttackLeft,
};

// ============================================================================
//								방향 이동
// ---------------------------------------------------------------------------
//			 [0](i-1,j)
// [3](i,j-1)	[4](i,j)	[1](i,j+1)
//			 [2](i+1,j)
// ============================================================================
const int _dx[5] = {0,1,0,-1,0};
const int _dy[5] = {-1,0,1,0,0};
enum class Direction {
	Up = 0,
	Right,
	Down,
	Left,
	None,
};
// ============================================================================
//							Object Type DATA SHEET
// ---------------------------------------------------------------------------
// 이 오브젝트가 어떤 타입인지?
// ---------------------------------------------------------------------------
// 0 ~ 9 Stage
// 0 : wall
// ---------------------------------------------------------------------------
// 10 ~ 19 Creatur
// 10 : player
// 11 : Monster
// ---------------------------------------------------------------------------
// 20 ~ 29 Projectile
// 20 : Projectile
// ---------------------------------------------------------------------------
// 30 ~ 39 SFX
// 30 : SFX
// ============================================================================
enum class ObjectType {
	//--------------------
	// stage
	Wall = 0,
	Portal,
	//--------------------
	// creatuer
	Player = 10,
	Monster,
	//--------------------
	// projectile
	Projectile=20,
	//--------------------
	// sfx
	SFX = 30,
};
