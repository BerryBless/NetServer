#pragma once
// ============================================================================
//						IObject
// ---------------------------------------------------------------------------
// 게임오브젝트의 인터페이스
// 게임 내에서 존재할 오브젝트는 모두 IObject 상속받아 사용
// 오브젝트는 좌표 _pos와 오브젝트 타입 _type 가짐
// ----
// 가상함수
// Update()					: 이 오브젝트가 매 프레임 해야할 일
// Render()					: 이 오브젝트를 매 프레임 화면에 출력
// OnCollison(IObject *obj)	: this 오브젝트와 obj 가 충돌했을때 [(가해자)->OnCollison(피해자)]
// Destroy()				: 이 오브젝트가 delete 되기 직전에 해야할 일, 이 함수 호출 이후 이 오브젝트는 완전 소멸
// ============================================================================
class IObject {
public:
	Position	_pos;							// 좌표계 [_pos.Y][_pos.X]
	ObjectType	_type;							// pch.h 참조
	bool		_collison;						// 이 오브젝트를 충돌처리 하냐(true) 안하냐(false)
	bool		_activate;						// 이 프레임 마지막에 이 오브젝트가 죽을예정이면 false
public:
	// 프레임워크에서 실행할 인터페이스
	virtual void Update() {};					// 이 오브젝트가 매 프레임 해야할 일
	virtual void Render() {};					// 이 오브젝트를 매 프레임 화면에 출력
	virtual void OnCollison(IObject *obj) {};	// this 오브젝트와 obj 가 충돌했을때 [(가해자)->OnCollison(피해자)]
	virtual void Destroy() {};					// 이 오브젝트가 delete 되기 직전에 해야할 일, 이 함수 호출 이후 이 오브젝트는 완전 소멸
};
