#include "pch.h"
#include "GameManager.h"
#include "Projectile.h"

Projectile _projectile[PROJECTILE_MAX]; // 전역 투사체 배열 생성

// ============================================================================
//							투사체 초기화
// ---------------------------------------------------------------------------
// _projectile[] 을 처음부터 끝가지 순회하면서
// 초기값
// _projectile[i].Visable	= false
// _projectile[i].Pos		= (0,0)	
// _projectile[i].Owner		= empty
// _projectile[i].Dir		= none
// 를 차례대로 넣는다.
// ============================================================================
void Projectile_Init() {
	// 처음부터 끝까지 순회 하며
	for (int i = 0; i < PROJECTILE_MAX; i++) {
		// 초기값 대입
		_projectile[i].Visable = false;
		_projectile[i].Pos.X = 0;
		_projectile[i].Pos.Y = 0;
		_projectile[i].Owner = ObjectType::Empty;
		_projectile[i].Dir = Direction::None;
	}
}

// ============================================================================
//							투사체 생성(발싸)
// ---------------------------------------------------------------------------
// _projectile[] 을 처음부터 끝가지 순회하면서
// _projectile[i].Visable == false인지점을 찾아 (없으면 리턴)
//  Pos, Owner, Dir값으로 방향을 계산해서 넣습니다.
// 다 넣었으면 Visable = true 가 됩니다.
// ============================================================================
bool Projectile_Fire(Position pos, ObjectType owner, Direction dir) {
	bool fire = false; // 발사를 했는가?  
	ObjectType targetObject; // 시작점에 무엇이있는가?
	Projectile tPro;// 임시 화살에 넣어 대입
	Position nPos;// 다음 포지션
	// 정보 넣어주기
	tPro.Visable = true;
	tPro.Pos = pos;
	tPro.Owner = owner;
	tPro.Dir = dir;

	// 다음 지점 계산하기
	nPos.X = pos.X + _dx[(int)dir];
	nPos.Y = pos.Y + _dy[(int)dir];

	// 다음지점 에 무언가 있는가?
	targetObject = Game_FindObject(nPos);
	if (targetObject == ObjectType::Empty) {
		for (int i = 0; i < PROJECTILE_MAX; i++) {
			// 비어있는 배열을 찾는다
			// 순회중 첫번째로 Visable == false 만나면 발싸!
			if (_projectile[i].Visable == false) {
				fire = true;	// 발싸 가능!
				// 쏜사람이랑 겹치지 않게 다음지점 계산해서
				// 스테이지 상태에 등록
				// _projectile[i]정보 넣어주기

				tPro.Pos = nPos;
				Game_CreateObject(tPro.Pos, ObjectType::Projectile);
				_projectile[i] = tPro;

				break;
			}

			// Visable == false 를 끝까지 만나지 못하면 발싸못함
			// 최대 숫자의 투사체가 스테이지에 존재
		}
	}
	// 시작지점에 몬스터나 플레이어가 있을
	else {
		// 바로 히트!
		// 가상의 화살을 맞은거로 처리
		Projectile_Hit(&tPro, targetObject);
	}
	return fire;
}

// ============================================================================
//							투사체 찾기
// ---------------------------------------------------------------------------
// _projectile[] 을 처음부터 끝가지 순회하면서
// _projectile[i].Visable == true 이면서
// _projectile[i].Pos == pos 인 지점을 찾아
// *_projectile[i]를 반환합니다.
// ============================================================================
Projectile* Projectile_Find(Position pos) {
	return Projectile_Find(pos.X, pos.Y);
}
// 위치에 따른 투사체 찾기 좌표로 찾을 수 있게 래핑합니다.
Projectile* Projectile_Find(int iX, int iY) {
	for (int i = 0; i < PROJECTILE_MAX; i++) {
		if (_projectile[i].Visable == true &&
			_projectile[i].Pos.X == iX &&
			_projectile[i].Pos.Y == iY) {
			return (_projectile + i);
		}
	}
	return nullptr;
}

// ============================================================================
//							투사체 충돌
// ---------------------------------------------------------------------------
// 투사체 진행 도중 무언가 만남
// 몇번째 투사체, 맞은 객체를 받음
// target 과 _projectile.Owner 비교
// 서로 대상이 다르면 맞음 (Player / Monster) (Monster끼리 팀킬 불가능)
// ============================================================================
void Projectile_Hit(Projectile* projectile, ObjectType target) {
	// 널포인터가 들어온다면 바로 리턴합니다.
	IF_PTR_NULL(projectile) return;
	// projectile가 target에 날아가 박힌상태
	if (target == ObjectType::Player) {
		// 타겟이 플레이어 일때
		if (IS_MONSTER(projectile->Owner)) {
			// 투사체의 주인이 몬스터라면 플레이어에게 히트!
			Player_Hit();
		}
	}
	// 투사체끼리 부딛힐
	else if (target == ObjectType::Projectile) {
		// 서로 소멸
		// 충돌 된것
		Projectile_Diss(Projectile_Find(projectile->Pos.X + _dx[(int)projectile->Dir],
			projectile->Pos.Y + _dy[(int)projectile->Dir]));
	}
	else if (IS_MONSTER(target)) {
		// 타겟이 몬스터 범위일
		if (projectile->Owner == ObjectType::Player) {
			// 투사체 주인이 몬스터라면
			// TODO 몬스터가맞음( 히트)
			// TODO 몬스터 마다 다르게 (Monster.cpp 에서 관리)
			Monster_Hit(Monster_Find(projectile->Pos.X + _dx[(int)projectile->Dir],
				projectile->Pos.Y + _dy[(int)projectile->Dir]));
		}
	}
}


// ============================================================================
//							투사체 소멸
// ---------------------------------------------------------------------------
// 게임 상테에서 삭제
// index 번째 투사체를 초기화
// _projectile[index] 의 값을 초기화 시켜줌
// ============================================================================
void Projectile_Diss(Projectile* projectile) {
	// 널포인터가 들어온다면 바로 리턴합니다.
	IF_PTR_NULL(projectile) return;

	// 게임에서 이 투사체를 삭제합니다
	Game_DissObject(projectile->Pos);

	// 투사체 초기화
	projectile->Visable = false;
	projectile->Pos.X = 0;
	projectile->Pos.Y = 0;
	projectile->Owner = ObjectType::Empty;
	projectile->Dir = Direction::None;
}

// ============================================================================
//							투사체 그리기
// ---------------------------------------------------------------------------
// 게임을 그릴때 좌표에있는 투사체를 찾아가지고
// dir 방향에 따라
// up(^) down(v) left(<) right(>)
// 그려주기
// ============================================================================
void Projectile_Draw(int iX, int iY) {
	Projectile* pPro; // 해당 좌표에 있는 투사체 포인터
	pPro = Projectile_Find(iX, iY);	// 그 좌표에 투사체를 찾구
	// 방향에 따라 버퍼에 그려줍니다.
	switch (pPro->Dir) {
	case Direction::Up:
		Sprite_Draw(iX, iY, '^');
		break;
	case Direction::Right:
		Sprite_Draw(iX, iY, '>');
		break;
	case Direction::Down:
		Sprite_Draw(iX, iY, 'v');
		break;
	case Direction::Left:
		Sprite_Draw(iX, iY, '<');
		break;
	}
}

// ============================================================================
//							투사체 업데이트
// ---------------------------------------------------------------------------
// _projectile[] 을 처음부터 끝가지 순회하면서
// _projectile[i].Visable이 true인 투사체를 찾아
// _projectile[i].Dir 방향대로 움직입니다.
// 움직일 수 없으면 _projectile[i].Dir 목적지의 오브젝트와 상호작용을 하고 
// 소멸(Visable = false) 합니다
// ============================================================================
void Projectile_Update() {
	ObjectType targetObject; // 목적지에 있는 오브젝트
	Position dest;			//목적지 좌표
	for (int i = 0; i < PROJECTILE_MAX; i++) {
		if (_projectile[i].Visable == true) {
			// 다음 좌표를 계산한후 그 곳에 있는 오브젝트를 확인한다,
			dest.X = _projectile[i].Pos.X + _dx[(int)_projectile[i].Dir];
			dest.Y = _projectile[i].Pos.Y + _dy[(int)_projectile[i].Dir];
			targetObject = Game_FindObject(dest);
			if (targetObject == ObjectType::Empty) {
				// 아무것도 없으면 진행
				Game_MoveObject(_projectile[i].Pos, dest);
				_projectile[i].Pos = dest;
			}
			else {
				// 무엇인가 부딧침
				//  충돌판정
				Projectile_Hit(_projectile + i, targetObject);
				// 부딧친 물체가 뭔지 몰라도 소멸은 해야함
				Projectile_Diss( _projectile + i);
			}
		}
	}
}
