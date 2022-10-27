#include "pch.h"
#include "CPlayer.h"
#include "CProjectile.h"
#include "Managers.h"
#include "CPortal.h"
// 생성자 : 기본정보를 넣고, _type을 Player로
CPlayer::CPlayer(int iX, int iY, int hp, int attack) : CCreature(iX, iY, hp, attack) {
	_type = ObjectType::Player;
}

// Render()	: _pos좌표에 'P'출력
void CPlayer::Render() {
	SPRITE_DRAW(_pos.X, _pos.Y, 'P');
}

// 충돌한게 포탈이면 다음스테이지로
void CPlayer::OnCollison(IObject *pTarget) {
	// 포탈인지 확인
	if (pTarget->_type == ObjectType::Portal) {
		// 포탈이면 그포탈에 지정된 씬 로딩
		((CPortal *) pTarget)->LoadNextScene();
	}
}

// 게임을 리셋
void CPlayer::Destroy() {
	_activate = false;	// 중복파괴 방지
	I_SCENEMANAGER->Reset();
}

// Inpu  메시지 큐에 있는걸 모두 꺼내서 처리
void CPlayer::Input(CQueue<InputMessage> &input) {
	while (!input.empty()) {

		InputMessage mes = input.front();
		Direction dir = Direction::None;
		Direction atDir = Direction::None;
		input.pop();

		switch (mes) {
		// 이동
		case InputMessage::Up:
			dir = Direction::Up;
			break;
		case InputMessage::Right:
			dir = Direction::Right;
			break;
		case InputMessage::Down:
			dir = Direction::Down;
			break;
		case InputMessage::Left:
			dir = Direction::Left;
			break;


		// 공격
		case InputMessage::AttackUp:
			atDir = Direction::Up;
			break;
		case InputMessage::AttackRight:
			atDir = Direction::Right;
			break;
		case InputMessage::AttackDown:
			atDir = Direction::Down;
			break;
		case InputMessage::AttackLeft:
			atDir = Direction::Left;
			break;
		default:
			break;
		}
		// 방향대로 움직이기
		if (dir != Direction::None) {
			_dir = dir;	
			OBJECT_MOVE(_pos.X + _dx[(int) _dir], _pos.Y + _dy[(int) _dir]);
		}
		// 방향대로 공격하기
		if (atDir != Direction::None) {
			INSTANTIATE(new CProjectile(_pos.X + _dx[(int) atDir], _pos.Y + _dy[(int) atDir], _attack, 1, atDir));

		}
	}
}
