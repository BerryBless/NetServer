#include "pch.h"
#include "GameManager.h"
#include "Projectile.h"
#include "Monster.h"
#include "Player.h"

Monster _monster[MONSTER_MAX];	//	스테이지의 몬스터 리스트

// ============================================================================
//							몬스터 초기화
// ---------------------------------------------------------------------------
// _monster[] 을 처음부터 끝가지 순회하면서 
// 초기값
// _monster[i].Visable				= false
// _monster[i].Pos					= (0,0)	
// _monster[i].Dir					= None
// _monster[i].Type					= Monster
// _monster[i].Hp					= 0
// _monster[i].PatternTickCount		= 0
// 를 차례대로 넣는다.
// ============================================================================
void Monster_Init() {
	for (int i = 0; i < MONSTER_MAX; i++) {
		_monster[i].Visable = false;
		_monster[i].Pos.X = 0;
		_monster[i].Pos.Y = 0;
		_monster[i].Dir = Direction::None;
		_monster[i].Type = ObjectType::Monster;
		_monster[i].Hp = 0;
		_monster[i].PatternTickCount = 0;
	}
}

// ============================================================================
//						Monster 스폰
// ---------------------------------------------------------------------------
// GameLoadStage()에서만 호출!
// _gameState에 몬스터 좌표 등록하기
//		스폰을 못하면 바로 리턴합니다.
// 처음부터 순회하며 Visable == false 인곳을 찾아
// _gameState[pos.Y][pos.X] = type
// _monster[i].Pos = pos
// Visable = true
// 넣기
// ============================================================================
void Monster_Spawn(Position pos, ObjectType type) {
	// 빈곳(Visable == false)을 찾아
	for (int i = 0; i < MONSTER_MAX; i++) {
		if (_monster[i].Visable == false) {
			// 스테이지에 스폰되하겠다고 허락받습니다.
			if (Game_CreateObject(pos, type) == false) {
				// 스폰을 못하면 그냥 리턴합니다..
				return;
			}
			// 스폰할곳 정보를 넣어주고
			_monster[i].Pos = pos;
			_monster[i].Visable = true;
			_monster[i].Type = type;
			break;
		}
	}
}
// 좌표로도 스폰할 수 있게 래핑합니다.
void Monster_Spawn(int iX, int iY, ObjectType type) {
	Position pos;
	pos.X = iX;
	pos.Y = iY;
	Monster_Spawn(pos, type);
}

// ============================================================================
//							투사체 찾기
// ---------------------------------------------------------------------------
// _monster[] 을 처음부터 끝가지 순회하면서
// _monster[i].Visable == true 이면서
// _monster[i].Pos == pos 인 지점을 찾아
// *_monster[i]를 반환합니다.
// ============================================================================
Monster* Monster_Find(Position pos) {
	return Monster_Find(pos.X, pos.Y);
}
// 위치에 따른 몬스터 찾기
Monster* Monster_Find(int iX, int iY) {
	for (int i = 0; i < MONSTER_MAX; i++) {
		// 활성화 되어있고 해당위치에 있는 몬스터 포인터를 리턴합니다.
		if (_monster[i].Visable == true &&
			_monster[i].Pos.X == iX &&
			_monster[i].Pos.Y == iY) {
			return (_monster + i);
		}
	}
	return nullptr;
}

// ============================================================================
//						Monster 움직이기
// ---------------------------------------------------------------------------
// dir 방향대로 맵상에 1칸을 움직임
// 이동했으면 ObjectType::Empty
// 이동에 실패했으면 막힌 방해물 타입을 반환합니다!
// ============================================================================
ObjectType Monster_Move(Monster* monster, Direction dir) {
	// 널포인터가 들어왔다면 디버깅용 값을 리턴해줍니다
	IF_PTR_NULL(monster) return ObjectType::MonsterEnd;
	Position nPos;// 목적지
	ObjectType object = ObjectType::Empty; // 목적지에 이게 있어요!

	// 목적지 계산
	nPos.X = monster->Pos.X + _dx[(int)dir];
	nPos.Y = monster->Pos.Y + _dy[(int)dir];
	// 목적지에 뭐가 있나면..
	object = Game_FindObject(nPos);

	// 이동 가능하다면?
	if (object == ObjectType::Empty) {
		// 1. 맵의 좌표를 바꾸고
		// 2. 내좌표를 바꾸고
		Game_MoveObject(monster->Pos, nPos);
		monster->Pos = nPos;
	}
	// 3. 마지막으로 본방향은 지금 dir
	monster->Dir = dir;
	return object;
}

// ============================================================================
//						Monster 공격
// ---------------------------------------------------------------------------
// 투사체만 날릴 수 있음
// 투사체를 날릴곳을 계산한뒤
// 투사체 에게 발사 하라고 떠넘기기
// ============================================================================
void Monster_Attack(Monster* monster) {
	// 널포인터가 들어왔다면 바로 리턴해줍니다
	IF_PTR_NULL(monster) return;
	// 쏘라고 정보주기
	Projectile_Fire(monster->Pos, monster->Type, monster->Dir);
}


// ============================================================================
//							업데이트
// ---------------------------------------------------------------------------
// 몬스터 마다 정해진 패턴으로 업데이트 합니다
// ============================================================================
void Monster_Update() {
	// 전체를 순회하며
	// 살아있는 몬스터를 패턴에 따른 업데이트
	for (int i = 0; i < MONSTER_MAX; i++) {
		if (_monster[i].Visable == true) {
			switch (_monster[i].Type) {
			case ObjectType::RockNRoll:
				Monster_RockNRoll(_monster + i);
				break;
			case ObjectType::Turret:
				Monster_Turret(_monster + i);
				break;
			case ObjectType::Doppelganger:
				Monster_Doppelganger(_monster + i);
				break;
			default:
				break;
			}
		}
	}
}

// ============================================================================
//						Monster 피격
// ---------------------------------------------------------------------------
// 몬스터의 포인터를 받고 포인터가 가르키는 몬스터에 피격 판정을 합니다.
//  Hp가 1씩 감소합니다
// Hp가 1보다 작아지면 소멸합니다
// ============================================================================
void Monster_Hit(Monster* monster) {
	// 널포인터가 들어왔다면 바로 리턴해줍니다
	IF_PTR_NULL(monster) return;
	// 피격 체력이 1감소한다
	monster->Hp--;

	// 소멸시킨다
	if (monster->Hp < 1) {
		Monster_Diss(monster);
	}
}

// ============================================================================
//						Monster 소멸
// ---------------------------------------------------------------------------
// 몬스터의 포인터를 받고 포인터가 가르키는 몬스터를 소멸 합니다.
// Game_DissObject(monster->Pos) 호출하여 스테이지에서 소멸 시킵니다
// *monster을 초기값으로 되돌립니다
// ============================================================================
void Monster_Diss(Monster* monster) {
	// 널포인터가 들어왔다면 바로 리턴해줍니다
	IF_PTR_NULL(monster) return;
	// TODO 리스폰
	// 스테이지에서 소멸
	Game_DissObject(monster->Pos);

	// 초기화
	// 포인터가 가르키는 몬스터를 초기값으로 되돌립니다.
	monster->Visable = false;
	monster->Pos.X = 0;
	monster->Pos.Y = 0;
	monster->Dir = Direction::None;
	monster->Type = ObjectType::Monster;
	monster->Hp = 0;
	monster->PatternTickCount = 0;
}


#pragma region Monster Type Pattern

// ============================================================================
//						몬스터 타입별 패턴 AI
// ---------------------------------------------------------------------------
// TYPE 별 패턴
// Monster = 20,	// 20 : (M) 그냥 서있음 (테스트용) , 몬스터 대역폭의 시작지점
// RockNRoll,		// 21 : (R) 상하좌우 랜덤으로 이동, 화살은 안날림
// Turret,			// 22 : (T) 플레이어가 직선상 (x 나 y 축 차이가 0) 이면 투사체 발싸
// Doppelganger,	// 23 : (q) 플레이어 행동을 반대로 따라함
// MonsterEnd		// 24 : 대여폭의 이 범위까지 몬스터입니다를 알려줌
// ============================================================================

// ============================================================================
//						방향계산
// ---------------------------------------------------------------------------
// src -> dest 방향
// 상하좌우 직선상만 계산
// 나머지는 None방향으로 버려짐
// Up	: src.X == dest.X && src.Y < dest.Y
// Down	: src.X == dest.X && src.Y > dest.Y
// Right: src.X > dest.X && src.Y == dest.Y
// Left : src.X < dest.X && src.Y == dest.Y
// ============================================================================
Direction SrcToDestDir(Position src, Position dest) {
	Direction rDir = Direction::None; // src -> dest 바라보는 방향
	// 상 하 좌 우 src의 직선 방향에 dest가 있는지 확인!
	if (src.X == dest.X) {
		// Up
		if (src.Y > dest.Y) { rDir = Direction::Up; }
		// Down
		if (src.Y < dest.Y) { rDir = Direction::Down; }
	}
	else if (src.Y == dest.Y) {
		// Left
		if (src.X > dest.X) { rDir = Direction::Left; }
		// Right
		if (src.X < dest.X) { rDir = Direction::Right; }
	}

	return rDir;
}


// ============================================================================
//						RockNRoll
// ---------------------------------------------------------------------------
// 1초마다 랜덤으로 굴러감
// 구르기 실패 이유가 플레이어면
// 플레이어 즉 사 (돌에 깔림)
// ============================================================================
void Monster_RockNRoll(Monster* monster) {
	// 널포인터가 들어왔다면 바로 리턴해줍니다
	IF_PTR_NULL(monster) return;
	time_t checkTick = clock(); // 쿨타임 체크할 타이머
	int r = rand();				// 랜덤! (srand 안함 -> 정해진 랜덤 = 정해진 패턴)
	Direction dir;				// 랜덤으로 지정할 방향
	// 1초마다 패턴 실행
	if (checkTick - monster->PatternTickCount > MONSTER_PATTERN_DELAY) {
		monster->PatternTickCount = checkTick; // 쿨타임 갱신
		// 4방향중 랜덤으로!
		dir = (Direction)(r % 4);
		if (Monster_Move(monster, dir) == ObjectType::Player) {
			// monster->dir 방향으로 발싸!
			Monster_Attack(monster);
		}
	}
}

// ============================================================================
//						Turret
// ---------------------------------------------------------------------------
// 행,열 직선상에 플레이어가 있으면 발싸 (벽판정 X)
// SrcToDestDir(Turret, player) 리턴값 있으면
// 리턴값으로 발싸
// ============================================================================
void Monster_Turret(Monster* monster) {
	// 널포인터가 들어왔다면 바로 리턴해줍니다
	IF_PTR_NULL(monster) return;
	time_t checkTick = clock(); // 쿨타임 체크할 타이머
	// 1초마다 패턴 실행
	if (checkTick - monster->PatternTickCount > MONSTER_PATTERN_DELAY) {
		// 플레이어 바라보는 방향을 구하고
		monster->Dir = SrcToDestDir(monster->Pos, _player.Pos);
		if (monster->Dir != Direction::None) {
			monster->PatternTickCount = checkTick; // 쿨타임 갱신
			// 발각되면 발싸!
			// monster->dir 방향으로 발싸!
			Monster_Attack(monster);
		}
	}
}

// ============================================================================
//						Doppelganger
// ---------------------------------------------------------------------------
// 플레이어의 입력시 움직임
// 움직임을 점대칭으로 따라함
// 플레이어에 플레그 (_player.Input)을 달아놈
// 공략법 : 바로 옆으로 붙어서 공격 (플레이어 판정이 우선)
// ============================================================================
void Monster_Doppelganger(Monster* monster) {
	// 널포인터가 들어왔다면 바로 리턴해줍니다
	IF_PTR_NULL(monster) return;
	Direction dir; // 도플갱어가 움직일 방향
	// 대칭 +2한다음 나머지를 구하고 1을 더함
	dir = (Direction)(((int)_player.Dir+2) % 4);
	if (_player.Input == PlayerInput::Move) {
		// 도플갱어 움직임
		Monster_Move(monster, dir);
	}
	if (_player.Input == PlayerInput::RangeAttack) {
		//도플갱어 화살 발싸!
		Monster_Attack(monster);
	}

}



#pragma endregion