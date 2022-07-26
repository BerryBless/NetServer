#include "pch.h"
#include "Player.h"
#include "Projectile.h"
#include "GameManager.h"
#include "SceneManager.h"


Player _player; // 플레이어는 딱하나만 존재

// ============================================================================
//						Player 스폰
// ---------------------------------------------------------------------------
// GameLoadStage()에서만 호출!
// _gameState에 플레이어 좌표 등록하기
// _gameState[pos.Y][pos.X] = ObjectType::Player
// _player.Pos = pos
// ============================================================================
void Player_Spawn(Position pos) {
	// 초기값 줌
	_player.Pos = pos;
	_player.Hp = 3;
	// 파일에서 읽은 위치가 명확하기 때문에 직접 스폰
	Game_CreateObject(pos, ObjectType::Player);
}
// 좌표로도 소환할 수 있게 래핑
void Player_Spawn(int iX, int iY) {
	Position pos;
	pos.X = iX;
	pos.Y = iY;
	Player_Spawn(pos);
}

// ============================================================================
//						Player 움직이기
// ---------------------------------------------------------------------------
// dir 방향대로 맵상에 1칸을 움직임
// 이동했으면 true, 못했으면 false
// 이동을 못했는데 못한이유가 포탈이라면 다음 스테이지로 갑니다
// 갈 수 있다면
// 1. 맵내의 좌표를 바꾸고
// 2. _player의 좌표를 바꾸고
// 3. Dir을 움직인 방향으로 바꿉니다.
// ============================================================================
ObjectType Player_Move(Direction dir) {

	Position nPos;// 목적지
	ObjectType object; // 목적지에 이게 있어요!
	// 방향으로 목적지 찾기
	nPos.X = _player.Pos.X + _dx[(int)dir];
	nPos.Y = _player.Pos.Y + _dy[(int)dir];
	// 다음좌표에 무엇이 있는지 찾기
	// 그 좌표에 뭐가있나면..
	object = Game_FindObject(nPos);

	// 갈 수 있는 곳
	if (object == ObjectType::Empty) {
		// 1. 맵의 좌표를 바꾸고
		// 2. 내좌표를 바꾸고
		Game_MoveObject(_player.Pos, nPos);
		_player.Pos = nPos;
	}
	// 포탈에 들어감!
	else if (object == ObjectType::Portal) {
		// 다음 스테이지로!!!!!!!
		Scene_LoadNextStage();
	}
	// 3. 마지막으로 본방향은 지금 dir
	_player.Dir = dir;
	return object;
}

// ============================================================================
//						Player 공격
// ---------------------------------------------------------------------------
// 투사체만 날릴 수 있음
// 투사체를 날릴곳을 계산한뒤
// 투사체 에게 발사 하라고 떠넘기기
// ============================================================================
void Player_Attack() {
	// TODO 근접공격
	// 투사체 쏘라고 정보주기
	Projectile_Fire(_player.Pos, ObjectType::Player, _player.Dir);
}

// ============================================================================
//						Player 업데이트
// ---------------------------------------------------------------------------
// 입력을 체크합니다
// _moveTickCount - 이동속도체크 카운터
// PLAYER_MOVE_DELAY ms마다 한칸씩 움직입니다.
// 
// _attackTickCount - 공격속도체크 카운터
// PLAYER_ATTACK_DELAY ms마다 공격을 날립니다.
// ============================================================================
// 이동 쿨타임 타이머
time_t _moveTickCount = 0;
// 공 쿨타임 타이머
time_t _attackTickCount = 0;
void Player_Update() {
	Direction dir = Direction::None;	// 움직일 방향
	time_t checkTick = clock();			// 쿨타임 체크할 틱카운터
	_player.Input = PlayerInput::None;

	// PLAYER_MOVE_DELAYms 마다 이동키 입력 가능
	if (checkTick - _moveTickCount > PLAYER_MOVE_DELAY) {
		// UP키가 눌려있는 상태
		if (GetAsyncKeyState(VK_UP) & 0x8000) {
			dir = Direction::Up;
		}
		// RIGHT키가 눌려있는 상태
		else if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
			dir = Direction::Right;
		}
		// DOWN키가 눌려있는 상태
		else if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
			dir = Direction::Down;
		}
		// LEFT키가 눌려있는 상태
		else if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
			dir = Direction::Left;
		}
		if (dir != Direction::None) {
			// 이동키가 눌렸으면!
			// 틱카운터 갱신
			// 움직이는 함수 호출
			_moveTickCount = checkTick;
			Player_Move(dir);
			// 도플갱어도 움직임!
			_player.Input = PlayerInput::Move;
		}
	}

	// PLAYER_ATTACK_DELAYms 마다 공격키 입력 가능
	if (GetAsyncKeyState(0x41) & 0x8001 &&
		checkTick - _attackTickCount >= PLAYER_ATTACK_DELAY) {
		// 틱카운터 갱신
		// 공격 함수 호출
		_attackTickCount = checkTick;
		Player_Attack();

		// 도플갱어도 화살 발싸!
		_player.Input = PlayerInput::RangeAttack;
	}
}

// ============================================================================
//						Player 피격
// ---------------------------------------------------------------------------
// 맞으면 HP가 1 감소
// HP가 1이하가 되면 소멸
// ============================================================================
void Player_Hit() {
	_player.Hp--;
	if (_player.Hp < 1) {
		Player_Diss();
	}
}

// ============================================================================
//						Player 소멸
// ---------------------------------------------------------------------------
// 플레이어를 Game_DissObject로 _gameState에서 소멸시킨후
// 맵을 처음부터 다시 만듭니다!
// ============================================================================
void Player_Diss() {
	// 소멸
	// 맵을 처음부터 다시만듦
	Game_DissObject(_player.Pos);
	Scene_Load(SceneType::Load);
	
}